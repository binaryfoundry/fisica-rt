"""Smoke-test the built application, including C++ brush input and rendering.

Build bin/web/Release first, then run: python tests/test_application.py
Requires playwright, its Chromium browser, Pillow, and numpy.
"""

from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread

import numpy as np
from PIL import Image
from playwright.sync_api import sync_playwright


class Handler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        super().end_headers()

    def log_message(self, *args):
        pass


def main():
    root = Path(__file__).resolve().parents[1]
    output = root / "bin" / "validation"
    output.mkdir(parents=True, exist_ok=True)
    server = ThreadingHTTPServer(("127.0.0.1", 0), partial(Handler, directory=str(root / "bin" / "web" / "Release")))
    Thread(target=server.serve_forever, daemon=True).start()
    errors = []
    logs = []
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
            page.goto(f"http://127.0.0.1:{server.server_port}/cascades.html")
            page.wait_for_function("typeof Module !== 'undefined' && Module.calledRun", timeout=30000)
            page.wait_for_timeout(1000)
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
            assert not any("gl error:" in line or "compile error" in line or "link error" in line for line in logs), logs
            print(f"Application smoke test passed: continuous strokes, shadows ({before:.1f} -> {after:.1f}), clear; no shader/GL errors.")
            browser.close()
    finally:
        server.shutdown()
        (output / "application-console.log").write_text("\n".join(logs + errors))


if __name__ == "__main__":
    main()
