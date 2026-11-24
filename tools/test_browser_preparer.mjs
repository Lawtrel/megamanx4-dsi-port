/* Verify the Python-to-WASM bridge using generated audio input. */
import assert from "node:assert/strict";
import {loadPyodide} from "../pages/node_modules/pyodide/pyodide.mjs";
import createXA from "../build/browser-tools/wasm/xa.mjs";
import createSamples from "../build/browser-tools/wasm/samples.mjs";
import createAtlas from "../build/browser-tools/wasm/atlas.mjs";
import {bridge} from "./web/browser-native.mjs";

const pyodide = await loadPyodide();
const [xa, samples, atlas] = await Promise.all([createXA(), createSamples(), createAtlas()]);
const fs = pyodide.FS;
fs.mkdir("/project");
const data = new Uint8Array(2352 * 8);
data.set([1, 0, 0x44, 4], 16);
for (let group = 0; group < 18; ++group) {
  const offset = 24 + group * 128;
  data.fill(0x21, offset + 16, offset + 128);
}
fs.writeFile("/project/test.bin", data);
pyodide.registerJsModule("x4_browser_native", bridge(fs, xa, samples, atlas));
assert.equal(pyodide.runPython(`
from pathlib import Path
from array import array
from x4_browser_native import run_xa, bake_reset, bake_pull

pcm = "/project/fixture.raw"
run_xa("/project/test.bin", "0", "0", "0", "1", pcm)
first = Path(pcm).read_bytes()
assert len(first) == 8064 and any(first)
run_xa("/project/test.bin", "0", "0", "0", "1", pcm)
assert first == Path(pcm).read_bytes()
body = bytes(16)
bake_reset(body, len(body))
samples = array("h", [123]) * 882
bake_pull(samples, 441)
assert not any(samples)
True
`), true);
console.log("PASS synthetic Python-to-WebAssembly audio preparation");
