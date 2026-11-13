"""Smoke-test the built application, including C++ brush input and rendering.

Build bin/web/Release first, then run: python tests/test_application.py
Requires playwright, its Chromium browser, Pillow, and numpy.
Use --site-root <directory> --page index.html to test a deployed static folder.
"""

import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread

import numpy as np
from PIL import Image
from playwright.sync_api import sync_playwright


class Handler(SimpleHTTPRequestHandler):
    def log_message(self, *args):
        pass


def main():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--site-root", type=Path, default=root / "bin" / "web" / "Release")
    parser.add_argument("--page", default="cascades.html")
    args = parser.parse_args()
    output = root / "bin" / "validation"
    output.mkdir(parents=True, exist_ok=True)
    server = ThreadingHTTPServer(("127.0.0.1", 0), partial(Handler, directory=str(args.site_root)))
    Thread(target=server.serve_forever, daemon=True).start()
    errors = []
    logs = []
    failed_responses = []
    try:
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={"width": 1280, "height": 720})
            page.add_init_script("""
                window.compiledShaderSources = [];
                const original = WebGL2RenderingContext.prototype.shaderSource;
                WebGL2RenderingContext.prototype.shaderSource = function(shader, source) {
                    window.compiledShaderSources.push(source);
                    return original.call(this, shader, source);
                };
            """)
            page.on("pageerror", lambda error: errors.append(str(error)))
            page.on("console", lambda message: logs.append(message.text))
            page.on("response", lambda response: failed_responses.append(f"{response.status} {response.url}") if response.status >= 400 else None)
            page.goto(f"http://127.0.0.1:{server.server_port}/{args.page}")
            page.wait_for_function("typeof Module !== 'undefined' && Module.calledRun", timeout=30000)
            page.wait_for_timeout(1000)
            assert not page.evaluate("crossOriginIsolated"), "Test must use ordinary static hosting without isolation headers"
            compiled = page.evaluate("compiledShaderSources")
            for name in ("distance", "cascades", "gather", "frontbuffer"):
                body = (root / "files" / "gl" / f"{name}.glsl").read_text().split("\n", 1)[1]
                assert any(body in source.replace("\r\n", "\n") for source in compiled), f"Web build contains stale {name}.glsl; relink the application"

            def frame():
                page.evaluate("() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve)))")

            def stroke(x0, y0, x1, y1, button):
                page.mouse.move(x0, y0)
                page.mouse.down(button=button)
                frame()
                page.mouse.move(x1, y1)
                frame()
                page.mouse.up(button=button)
                frame()

            stroke(500, 220, 500, 500, "left")
            page.screenshot(path=str(output / "app-emitter.png"))
            unblocked = np.asarray(Image.open(output / "app-emitter.png").convert("RGB"))
            # A single mouse jump must paint a continuous vertical line.
            assert np.all(unblocked[240:480, 500, 0] > 200), "Emitter stroke has gaps or did not render"
            before = float(unblocked[300:420, 850:950, 0].mean())
            assert before > 10, f"Distant scene received no light: {before}"

            stroke(640, 4, 640, 716, "right")
            page.screenshot(path=str(output / "app-shadow.png"))
            blocked = np.asarray(Image.open(output / "app-shadow.png").convert("RGB"))
            after = float(blocked[300:420, 850:950, 0].mean())
            assert after < before * 0.05, f"Wall failed to cast a shadow: {before} -> {after}"

            page.keyboard.press("c")
            frame()
            page.screenshot(path=str(output / "app-cleared.png"))
            cleared = np.asarray(Image.open(output / "app-cleared.png").convert("RGB"))
            assert np.max(cleared[200:520, 460:980]) == 0, "Clear left stale light or geometry"
            assert not errors, errors
            assert not failed_responses, failed_responses
            assert not any("gl error:" in line or "compile error" in line or "link error" in line for line in logs), logs
            print(f"Static deployment smoke test passed: continuous strokes, shadows ({before:.1f} -> {after:.1f}), clear; no missing assets or shader/GL errors; no isolation headers.")
            browser.close()
    finally:
        server.shutdown()
        (output / "application-console.log").write_text("\n".join(logs + errors + failed_responses))


if __name__ == "__main__":
    main()
