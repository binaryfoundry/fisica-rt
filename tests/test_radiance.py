"""Render the production GLSL in WebGL 2 and check lighting against geometry.

Run: python tests/test_radiance.py
Requires the Python playwright package and its Chromium browser.
Artifacts are written to the ignored bin/validation directory.
"""

import base64
import json
from pathlib import Path

from playwright.sync_api import sync_playwright


def main():
    root = Path(__file__).resolve().parents[1]
    sources = {
        name: (root / "files" / "gl" / f"{name}.glsl").read_text()
        for name in ("distance", "cascades", "gather", "frontbuffer")
    }
    output = root / "bin" / "validation"
    output.mkdir(parents=True, exist_ok=True)
    with sync_playwright() as playwright:
        browser = playwright.chromium.launch(headless=True)
        page = browser.new_page()
        page.set_content("<canvas></canvas>")
        page.add_script_tag(path=str(root / "tests" / "radiance_checks.js"))
        report = page.evaluate("runRadianceChecks", sources)
        browser.close()

    for name, url in report.pop("images").items():
        (output / f"{name}.png").write_bytes(base64.b64decode(url.split(",", 1)[1]))
    (output / "radiance-report.json").write_text(json.dumps(report, indent=2))
    print(json.dumps(report, indent=2))
    assert all(check["passed"] for check in report["checks"]), "Radiance regression failed"


if __name__ == "__main__":
    main()
