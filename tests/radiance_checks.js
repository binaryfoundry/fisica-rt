// Exercise production shaders with independent geometric lighting expectations.
window.runRadianceChecks = function (sources) {
    const canvas = document.querySelector("canvas");
    const gl = canvas.getContext("webgl2", { antialias: false });
    if (!gl || !gl.getExtension("EXT_color_buffer_float")) {
        throw new Error("WebGL 2 with EXT_color_buffer_float is required");
    }
    const checks = [];
    const images = {};
    function check(name, passed, measured) { checks.push({ name, passed, measured }); }
    function program(source) {
        const stages = [[gl.VERTEX_SHADER, "COMPILING_VS"], [gl.FRAGMENT_SHADER, "COMPILING_FS"]];
        const p = gl.createProgram();
        for (const [type, define] of stages) {
            const s = gl.createShader(type);
            gl.shaderSource(s, source.replace("#version 300 es", `#version 300 es\n#define ${define}`));
            gl.compileShader(s);
            if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(s));
            gl.attachShader(p, s);
        }
        gl.linkProgram(p);
        if (!gl.getProgramParameter(p, gl.LINK_STATUS)) throw new Error(gl.getProgramInfoLog(p));
        return p;
    }
    const programs = Object.fromEntries(Object.entries(sources).map(([k, v]) => [k, program(v)]));
    const vertices = new Float32Array([0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1]);
    gl.bindBuffer(gl.ARRAY_BUFFER, gl.createBuffer());
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
    gl.enableVertexAttribArray(0);
    gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 20, 0);
    gl.enableVertexAttribArray(1);
    gl.vertexAttribPointer(1, 2, gl.FLOAT, false, 20, 12);

    function target(size, data = null) {
        const texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA32F, size, size, 0, gl.RGBA, gl.FLOAT, data);
        const fb = gl.createFramebuffer();
        gl.bindFramebuffer(gl.FRAMEBUFFER, fb);
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
        if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) !== gl.FRAMEBUFFER_COMPLETE) throw new Error("Incomplete float framebuffer");
        return { texture, fb, size };
    }
    function draw(p, output, textures, uniforms = {}) {
        gl.useProgram(p);
        gl.bindFramebuffer(gl.FRAMEBUFFER, output.fb);
        gl.viewport(0, 0, output.size, output.size);
        Object.entries(textures).forEach(([name, t], i) => {
            gl.activeTexture(gl.TEXTURE0 + i);
            gl.bindTexture(gl.TEXTURE_2D, t.texture);
            gl.uniform1i(gl.getUniformLocation(p, name), i);
        });
        for (const [name, value] of Object.entries(uniforms)) {
            const location = gl.getUniformLocation(p, name);
            if (Array.isArray(value)) {
                if (value.length === 16) gl.uniformMatrix4fv(location, false, value);
                else gl.uniform2fv(location, value);
            } else gl.uniform1f(location, value);
        }
        gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        const error = gl.getError();
        if (error !== gl.NO_ERROR) throw new Error(`WebGL error: ${error}`);
    }
    function read(t) {
        gl.bindFramebuffer(gl.FRAMEBUFFER, t.fb);
        const pixels = new Float32Array(t.size * t.size * 4);
        gl.readPixels(0, 0, t.size, t.size, gl.RGBA, gl.FLOAT, pixels);
        return pixels;
    }
    let size = 256, atlasSize = 128;
    const levels = 6;
    let scene = target(size);
    let distances = [target(size), target(size)];
    let cascades = Array.from({ length: levels }, () => target(atlasSize));
    let gathered = target(size), displayed = target(size);
    let pixels;
    function clear() { pixels = new Float32Array(size * size * 4); }
    function pixel(x, y, color) {
        if (x >= 0 && y >= 0 && x < size && y < size) pixels.set(color, (y * size + x) * 4);
    }
    function disk(cx, cy, radius, color) {
        for (let y = 0; y < size; ++y) for (let x = 0; x < size; ++x) {
            if ((x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2 <= radius ** 2) pixel(x, y, color);
        }
    }
    function rect(x0, y0, x1, y1, color = [0, 0, 0, 1]) {
        for (let y = y0; y < y1; ++y) for (let x = x0; x < x1; ++x) pixel(x, y, color);
    }
    function field() {
        gl.bindTexture(gl.TEXTURE_2D, scene.texture);
        gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, size, size, gl.RGBA, gl.FLOAT, pixels);
        draw(programs.distance, distances[0], { scene_tex: scene, seeds_tex: scene }, { jump_distance: 0 });
        let current = 0;
        for (let jump = size / 2; jump >= 1; jump /= 2) {
            draw(programs.distance, distances[1 - current], { scene_tex: scene, seeds_tex: distances[current] }, { jump_distance: jump });
            current = 1 - current;
        }
        draw(programs.distance, distances[1 - current], { scene_tex: scene, seeds_tex: distances[current] }, { jump_distance: 1 });
        return distances[1 - current];
    }
    const identity = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
    const projection = [2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, -1, -1, 0, 1];
    function render(name, srgb = 1, interval = 4) {
        const distance = field();
        for (let i = levels - 1; i >= 0; --i) {
            draw(programs.cascades, cascades[i], {
                scene_tex: scene, distance_tex: distance, prev_atlas: cascades[i === levels - 1 ? 0 : i + 1]
            }, { cascade_level: i, cascade_count: levels, base_interval_f: interval, srgb_falloff: srgb });
        }
        draw(programs.gather, gathered, { atlas: cascades[0], scene_tex: scene }, { srgb_falloff: srgb });
        draw(programs.frontbuffer, displayed, { tex: gathered }, { exposure: 1, srgb_falloff: srgb, view: identity, projection });
        const output = read(gathered), display = read(displayed);
        check(`${name}: finite nonnegative radiance`, output.every(v => Number.isFinite(v) && v >= 0), true);
        const imageCanvas = document.createElement("canvas");
        imageCanvas.width = imageCanvas.height = size;
        const context = imageCanvas.getContext("2d");
        const image = context.createImageData(size, size);
        for (let i = 0; i < display.length; ++i) image.data[i] = Math.round(display[i] * 255);
        context.putImageData(image, 0, 0);
        images[name] = imageCanvas.toDataURL();
        return { output, display, atlas: read(cascades[0]), distance };
    }
    function value(data, x, y, channel = 0) { return data[(y * size + x) * 4 + channel]; }
    function mean(data, x0, y0, x1, y1, channel = 0) {
        let sum = 0;
        for (let y = y0; y < y1; ++y) for (let x = x0; x < x1; ++x) sum += value(data, x, y, channel);
        return sum / ((x1 - x0) * (y1 - y0));
    }

    clear();
    let result = render("empty");
    check("Empty scene is black and fully transmissive", result.atlas.every((v, i) => v === (i % 4 === 3 ? 1 : 0)), true);

    clear();
    disk(128, 128, 12, [0.8, 0.4, 0.1, 1]);
    result = render("disk");
    const sample = value(result.output, 192, 128);
    const expected = Math.asin(12 / 64.5) / Math.PI * 0.8 ** 2.2;
    check("Disk illumination agrees with its analytic solid angle", Math.abs(sample / expected - 1) < 0.2, { sample, expected });
    const axisValues = [[192, 128], [63, 128], [128, 192], [128, 63]].map(([x, y]) => value(result.output, x, y));
    check("Lighting is symmetric about a centered emitter", Math.max(...axisValues) / Math.min(...axisValues) < 1.08, axisValues);
    check("Emitter retains its painted color", Math.abs(value(result.display, 128, 128) - 0.8) < 0.0001, value(result.display, 128, 128));
    const red = value(result.output, 192, 128), green = value(result.output, 192, 128, 1);
    check("Light accumulates in linear color space", Math.abs(red / green / (2 ** 2.2) - 1) < 0.001, red / green);
    result = render("disk-no-srgb", 0);
    check("Disabling sRGB disables both conversions", Math.abs(value(result.display, 128, 128) - 0.8) < 0.0001, value(result.display, 128, 128));

    clear();
    disk(64, 128, 12, [1, 0.4, 0.1, 1]);
    const unblocked = render("unblocked");
    rect(128, 0, 129, size);
    result = render("one-pixel-wall");
    const blockedMean = mean(result.output, 152, 64, 240, 192);
    const unblockedMean = mean(unblocked.output, 152, 64, 240, 192);
    check("A one-pixel wall blocks far-cascade illumination", blockedMean < unblockedMean * 0.01, { blockedMean, unblockedMean });
    check("The wall surface stays black", mean(result.output, 128, 0, 129, size) === 0, mean(result.output, 128, 0, 129, size));
    for (const interval of [1, 32]) {
        const boundaryCase = render(`wall-interval-${interval}`, 1, interval);
        const shadow = mean(boundaryCase.output, 152, 64, 240, 192);
        check(`One-pixel wall blocks light at base interval ${interval}`, shadow < 0.0001, shadow);
    }

    clear();
    disk(64, 128, 12, [1, 0.4, 0.1, 1]);
    rect(100, 100, 156, 101);
    rect(100, 155, 156, 156);
    rect(100, 100, 101, 156);
    rect(155, 100, 156, 156);
    result = render("closed-room");
    const inside = mean(result.output, 116, 116, 140, 140);
    check("A closed one-pixel enclosure excludes exterior light", inside < 0.0001, inside);

    clear();
    disk(0, 64, 12, [1, 0, 0, 1]);
    disk(256, 192, 12, [0, 0, 1, 1]);
    result = render("edge-lights");
    check("Atlas edges preserve direction and color symmetry", Math.abs(value(result.output, 64, 64) - value(result.output, 191, 191, 2)) < 0.002, [value(result.output, 64, 64), value(result.output, 191, 191, 2)]);
    check("Colored lights introduce no unrelated channel", result.output.every((v, i) => i % 4 !== 1 || v === 0), true);

    // Exercise the actual ray marcher separately with a ray that would skip
    // a one-pixel wall when divided into a fixed number of steps.
    const traceSource = sources.cascades.slice(0, sources.cascades.lastIndexOf("    void main()")) + `
        uniform vec2 trace_start;
        uniform vec2 trace_end;
        void main() { out_color = trace_interval(trace_start, trace_end); }
        #endif
    `;
    const traceProgram = program(traceSource), traceOutput = target(1);
    clear();
    disk(240, 128, 8, [1, 0, 0, 1]);
    rect(101, 0, 102, size);
    let distance = field();
    draw(traceProgram, traceOutput, { scene_tex: scene, distance_tex: distance }, {
        trace_start: [0.5, 128.5], trace_end: [255.5, 128.5], srgb_falloff: 1
    });
    let traced = Array.from(read(traceOutput));
    check("Long ray hits a one-pixel occluder", traced[0] === 0 && traced[3] === 0, traced);
    rect(101, 0, 102, size, [0, 0, 0, 0]);
    distance = field();
    draw(traceProgram, traceOutput, { scene_tex: scene, distance_tex: distance }, {
        trace_start: [0.5, 128.5], trace_end: [255.5, 128.5], srgb_falloff: 1
    });
    traced = Array.from(read(traceOutput));
    check("Long ray finds the emitter after the wall is removed", traced[0] === 1 && traced[3] === 0, traced);

    // Repeat the demanding cases at the application's production resolution.
    for (const t of [scene, ...distances, ...cascades, gathered, displayed]) {
        gl.deleteTexture(t.texture);
        gl.deleteFramebuffer(t.fb);
    }
    size = 1024;
    atlasSize = 512;
    scene = target(size);
    distances = [target(size), target(size)];
    cascades = Array.from({ length: levels }, () => target(atlasSize));
    gathered = target(size);
    displayed = target(size);
    clear();
    disk(256, 512, 32, [0.8, 0.4, 0.1, 1]);
    result = render("production-disk");
    const distant = value(result.output, 768, 512);
    const distantExpected = Math.asin(32 / 512.5) / Math.PI * 0.8 ** 2.2;
    check("Production-resolution light reaches distant probes", Math.abs(distant / distantExpected - 1) < 0.2, { distant, distantExpected });
    rect(512, 0, 513, size);
    result = render("production-one-pixel-wall");
    const dark = mean(result.output, 640, 256, 896, 768);
    check("Production-resolution one-pixel wall blocks distant light", dark < 0.0001, dark);
    rect(512, 0, 513, size, [0, 0, 0, 0]);
    rect(512, 384, 520, 640);
    disk(800, 224, 16, [0.1, 0.5, 1, 1]);
    disk(800, 800, 16, [0.7, 0.1, 1, 1]);
    render("production-soft-shadows");
    clear();
    disk(8, 8, 8, [1, 0, 0, 1]);
    result = render("production-diagonal", 1, 1);
    const diagonalLight = value(result.output, 1016, 1016);
    check("Terminal interval covers the full scene diagonal at minimum interval", diagonalLight > 0.0001, diagonalLight);

    return { renderer: gl.getParameter(gl.RENDERER), checks, images };
};
