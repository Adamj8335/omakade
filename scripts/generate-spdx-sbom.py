#!/usr/bin/env python3

import argparse
import datetime
import hashlib
import json
import re
import subprocess
import tarfile
import urllib.parse
from pathlib import Path


def parse_package_info(package_path: Path) -> dict[str, list[str]]:
    with tarfile.open(package_path, mode="r:*") as archive:
        member = archive.getmember(".PKGINFO")
        package_info = archive.extractfile(member)
        if package_info is None:
            raise ValueError("package does not contain .PKGINFO")
        text = package_info.read().decode("utf-8")

    values: dict[str, list[str]] = {}
    for line in text.splitlines():
        if not line or line.startswith("#") or " = " not in line:
            continue
        key, value = line.split(" = ", maxsplit=1)
        values.setdefault(key, []).append(value)
    return values


def required_value(values: dict[str, list[str]], key: str) -> str:
    entries = values.get(key, [])
    if not entries:
        raise ValueError(f".PKGINFO is missing {key}")
    return entries[0]


def dependency_name(specification: str) -> str:
    match = re.match(r"^[A-Za-z0-9@._+:-]+", specification)
    if match is None:
        raise ValueError(f"invalid dependency: {specification}")
    return match.group(0)


def installed_version(name: str) -> str | None:
    result = subprocess.run(
        ["pacman", "-Q", name],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return None
    fields = result.stdout.strip().split(maxsplit=1)
    return fields[1] if len(fields) == 2 else None


def dependency_closure(name: str) -> list[tuple[str, str]]:
    result = subprocess.run(
        ["pactree", "--unique", name],
        check=True,
        capture_output=True,
        text=True,
    )
    packages = []
    seen = {name}
    for specification in result.stdout.splitlines():
        candidate = dependency_name(specification)
        if candidate in seen:
            continue
        version = installed_version(candidate)
        if version is None:
            continue
        seen.add(candidate)
        packages.append((candidate, version))
    return packages


def spdx_id(name: str) -> str:
    return "SPDXRef-Package-" + re.sub(r"[^A-Za-z0-9.-]", "-", name)


def purl(namespace: str, name: str, version: str | None, architecture: str) -> str:
    locator = f"pkg:alpm/{namespace}/{urllib.parse.quote(name, safe='')}"
    if version:
        locator += f"@{urllib.parse.quote(version, safe='')}"
    return locator + f"?arch={urllib.parse.quote(architecture, safe='')}"


def package_entry(
    *,
    identifier: str,
    name: str,
    version: str | None,
    architecture: str,
    namespace: str,
    supplier: str,
) -> dict:
    entry = {
        "SPDXID": identifier,
        "name": name,
        "downloadLocation": "NOASSERTION",
        "filesAnalyzed": False,
        "copyrightText": "NOASSERTION",
        "licenseConcluded": "NOASSERTION",
        "licenseDeclared": "NOASSERTION",
        "supplier": supplier,
        "externalRefs": [
            {
                "referenceCategory": "PACKAGE-MANAGER",
                "referenceType": "purl",
                "referenceLocator": purl(namespace, name, version, architecture),
            }
        ],
    }
    if version:
        entry["versionInfo"] = version
    return entry


def generate_sbom(package_path: Path) -> dict:
    package_info = parse_package_info(package_path)
    name = required_value(package_info, "pkgname")
    version = required_value(package_info, "pkgver")
    architecture = required_value(package_info, "arch")
    digest = hashlib.sha256(package_path.read_bytes()).hexdigest()
    dependency_namespace = "archarm" if architecture == "aarch64" else "arch"
    main_identifier = spdx_id(name)
    main_package = package_entry(
        identifier=main_identifier,
        name=name,
        version=version,
        architecture=architecture,
        namespace="btsouth",
        supplier="Organization: Omakade",
    )
    main_package["packageFileName"] = package_path.name
    main_package["checksums"] = [{"algorithm": "SHA256", "checksumValue": digest}]
    licenses = package_info.get("license", [])
    if licenses:
        main_package["licenseDeclared"] = " AND ".join(licenses)

    dependencies = []
    relationships = [
        {
            "spdxElementId": "SPDXRef-DOCUMENT",
            "relationshipType": "DESCRIBES",
            "relatedSpdxElement": main_identifier,
        }
    ]
    direct_dependencies = {
        dependency_name(specification)
        for specification in package_info.get("depend", [])
    }
    for dep_name, dep_version in dependency_closure(name):
        identifier = spdx_id(dep_name)
        dependencies.append(
            package_entry(
                identifier=identifier,
                name=dep_name,
                version=dep_version,
                architecture=architecture,
                namespace=dependency_namespace,
                supplier=(
                    "Organization: Arch Linux ARM"
                    if dependency_namespace == "archarm"
                    else "Organization: Arch Linux"
                ),
            )
        )
        relationships.append(
            {
                "spdxElementId": main_identifier,
                "relationshipType": "DEPENDS_ON",
                "relatedSpdxElement": identifier,
                "comment": (
                    "Direct runtime dependency"
                    if dep_name in direct_dependencies
                    else "Transitive runtime dependency"
                ),
            }
        )

    created = datetime.datetime.now(datetime.UTC).replace(microsecond=0).isoformat()
    return {
        "spdxVersion": "SPDX-2.3",
        "dataLicense": "CC0-1.0",
        "SPDXID": "SPDXRef-DOCUMENT",
        "name": package_path.name,
        "documentNamespace": (
            f"https://spdx.org/spdxdocs/{name}-{version}-{architecture}-{digest}"
        ),
        "creationInfo": {
            "created": created.replace("+00:00", "Z"),
            "creators": ["Tool: Omakade SPDX generator"],
        },
        "packages": [main_package, *dependencies],
        "relationships": relationships,
    }


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate an SPDX 2.3 SBOM from an Arch package."
    )
    parser.add_argument("package", type=Path)
    parser.add_argument("output", type=Path)
    arguments = parser.parse_args()

    sbom = generate_sbom(arguments.package)
    arguments.output.write_text(
        json.dumps(sbom, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


if __name__ == "__main__":
    main()
