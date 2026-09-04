#!/usr/bin/env python3

import importlib.util
import io
import tarfile
import tempfile
import unittest
from pathlib import Path
from unittest import mock


SCRIPT = Path(__file__).parents[1] / "scripts" / "generate-spdx-sbom.py"
SPEC = importlib.util.spec_from_file_location("omakade_sbom", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
SBOM = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(SBOM)


class SbomGeneratorTests(unittest.TestCase):
    def test_generates_valid_runtime_package_relationships(self) -> None:
        package_info = b"""\
pkgname = omakade
pkgver = 1.6.0-1
arch = aarch64
license = GPL-3.0-or-later
depend = qt6-base
depend = sdl3>=3.2
"""
        with tempfile.TemporaryDirectory() as directory:
            package = Path(directory) / "omakade-1.6.0-1-aarch64.pkg.tar"
            with tarfile.open(package, mode="w") as archive:
                member = tarfile.TarInfo(".PKGINFO")
                member.size = len(package_info)
                archive.addfile(member, io.BytesIO(package_info))

            with mock.patch.object(
                SBOM,
                "dependency_closure",
                return_value=[
                    ("qt6-base", "6.11.2-3"),
                    ("sdl3", "3.4.14-1"),
                    ("glibc", "2.42+r33+gde32b1c089a4-1"),
                ],
            ):
                document = SBOM.generate_sbom(package)

        self.assertEqual(document["spdxVersion"], "SPDX-2.3")
        self.assertEqual(len(document["packages"]), 4)
        self.assertEqual(len(document["relationships"]), 4)
        self.assertEqual(
            document["packages"][1]["externalRefs"][0]["referenceLocator"],
            "pkg:alpm/archarm/qt6-base@6.11.2-3?arch=aarch64",
        )
        self.assertEqual(
            document["relationships"][1]["comment"], "Direct runtime dependency"
        )
        self.assertEqual(
            document["relationships"][3]["comment"],
            "Transitive runtime dependency",
        )
        for package in document["packages"]:
            self.assertEqual(package["copyrightText"], "NOASSERTION")

    def test_rejects_invalid_dependency(self) -> None:
        with self.assertRaisesRegex(ValueError, "invalid dependency"):
            SBOM.dependency_name("<invalid")


if __name__ == "__main__":
    unittest.main()
