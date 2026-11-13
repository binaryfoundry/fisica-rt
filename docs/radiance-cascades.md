# Radiance cascades implementation

The renderer follows the distance-field tracing and direction-first,
pre-averaged atlas approach used by the interactive demo at
[radiance-cascades.com](https://radiance-cascades.com/js/rc.js).
The algorithm is also explained in [Jason McGhee's walkthrough](https://jason.today/rc).

The scene is a 1024 by 1024 floating-point texture. Alpha means occupancy:
empty pixels have alpha zero; emitters and black occluders have alpha one.
Emitter RGB stores the painted color multiplied by the intensity control.
Brush strokes fill a capsule between successive mouse positions, including
the first point, so fast mouse movement does not leave holes in walls.
The application opens with the reference demo's warm curved emitter and
horizontal black shadow bar, scaled from its 512px canvas to the scene.
The ImGui emitter intensity defaults to 1.5. Press C to clear the scene;
press R or click **Reset example** to restore the example using the current
intensity setting.

`Cascades::UpdateDistanceField` seeds occupied pixels and runs jump flooding
with an additional unit step (JFA+1). Its two textures store nearest occupied
pixel centers. Jump flooding approximates the nearest-seed field, as in the
reference demo. Tracing uses distance jumps through empty space and pixel
boundary steps near geometry, rather than a fixed number of equally spaced
samples that can skip thin surfaces. The field rebuilds only when painting
or clearing changes the scene.

Six 512 by 512 atlases store the cascades. Level zero has 512 by 512 probes
spaced two scene pixels apart. Each level doubles the probe spacing and
quadruples the angular resolution: 4, 16, 64, 256, 1024, then 4096 rays per
probe. Each texel averages four adjacent rays. Direction tiles contain
contiguous grids of spatial probes, so the base cascade is already a
fluence image.

All ray lengths use scene pixels. For level `c` and base interval `b`, the
near distance is `b * (4^c - 1) / 3` and the nominal interval length is
`b * 4^c`. The end distance is expanded by `1 / cos(pi / ray_count)` to
cover the ray sector's angular footprint. This overlap prevents a child
ray from starting beyond a thin wall missed by its parent's center ray,
including at large base intervals. The terminal interval extends at least
to the scene diagonal. A ray that
has left the scene contributes no environment illumination.

An atlas texel stores `(radiance, transmittance)`. Empty intervals have
transmittance one; opaque hits have transmittance zero. Composition is
`(Lnear + Tnear * Lfar, Tnear * Tfar)` throughout. For each upper probe,
the near interval is traced to that probe's interval start and composed
with its far interval **before** bilinear interpolation. This visibility
check prevents illumination from interpolating through a wall. Coordinates
are clamped within each direction tile, with the probe-center offset
included in the interpolation weights.

The gather pass interpolates base probes and composites occupied scene
pixels at the full scene resolution. Walls remain black, and emitters
retain their painted color. With sRGB falloff enabled, emission is decoded
before accumulation and encoded once at display; disabling it disables both
conversions. Display uses exposure and output clamping, matching the demo's
color conversion rather than applying an additional ACES filmic curve.
Manual interpolation avoids requiring float-linear texture filtering.

Lighting is recomputed when the scene, base interval, or sRGB setting changes.
Changing exposure only updates display. The existing experimental
`cascades_probe.glsl` is unused; `cascades.glsl` is the active renderer.

## Validation

Build the Windows application with `cmake --build bin/win --config Release`.
Build the Emscripten application with `cmake --build bin/web/Release` from an
environment with the configured build tool available (NMake for the existing
Windows configuration).
Web link dependencies include runtime assets and the HTML shell so editing
either regenerates the web package. The web application does not use CPU
threads; it builds without pthreads so static hosting needs no cross-origin
isolation headers or service worker.
Compiler prefix maps keep assertion paths relative in the distributed WebAssembly.

Run `python tests/test_radiance.py` for production-shader WebGL 2 checks:
analytic disk illumination, symmetry, sRGB, empty-scene transmittance,
one-pixel walls, a sealed room, atlas edges, interval bounds, and full-size
1024 by 1024 rendering. Requires Python Playwright and its Chromium browser.

After building the web application, run `python tests/test_application.py`
to exercise its C++ painting and rendering through the browser. It verifies
stroke continuity, a wall shadow, clearing, and absence of shader/GL errors.
It also checks the visible starting example, the 1.5 default intensity,
and restoration of the example after clearing.
This test additionally requires Pillow and NumPy. It serves the application
locally without special headers and checks that all requested assets load.

Test reports, console logs, and rendered PNGs are saved in the ignored
`bin/validation` directory.

## Static deployment

Copy `bin/web/Release/cascades.html` to the destination as `index.html`, and
copy `cascades.js`, `cascades.wasm`, and `cascades.data` alongside it without
renaming them. Include the project `LICENCE` as `LICENCE.txt`. Runtime shaders
are already inside the data package; build files and loose shader copies
are not needed on the server.

Verify the deployed folder with
`python tests/test_application.py --site-root <deployment-directory> --page index.html`.
After publishing, the same checks can run against the public site with
`python tests/test_application.py --url <public-url>`.
