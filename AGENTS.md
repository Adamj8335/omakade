# Release safety

- Never push commits, create or push tags, publish GitHub releases, or upload release assets until all relevant automated checks pass and the maintainer has tested the exact candidate locally and explicitly approved publication.
- Treat every change as a local release candidate until that approval is given.
- A critical hotfix may bypass local maintainer testing only when the maintainer explicitly authorizes that specific release.
- Before requesting release approval, report the exact candidate commit, checks run, known limitations, and a short manual test checklist.
