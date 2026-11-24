/* Bridges the existing preparation scripts to the same C tools on WebAssembly. */
function mountProject(tool, project) {
  const fs = tool.FS;
  fs.mkdir("/project");
  fs.mount(fs.filesystems.PROXYFS, {root: "/project", fs: project}, "/project");
  fs.chdir("/project");
}

function checkedRun(tool, args, name) {
  const result = tool.callMain(args);
  if (result !== 0 && result !== undefined) {
    throw new Error(`${name} terminou com código ${result}`);
  }
}

export function bridge(project, xa, samples, atlas) {
  mountProject(xa, project);
  mountProject(atlas, project);
  return {
    run_xa: (...args) => checkedRun(xa, args, "Conversão de áudio"),
    run_atlas: (...args) => checkedRun(atlas, args, "Preparação do painel"),
    bake_reset(body, size) {
      const input = body.getBuffer("u8");
      const pointer = samples._malloc(size || 1);
      try {
        if (!pointer || input.data.byteLength !== size) throw new Error("Banco de áudio inválido.");
        samples.HEAPU8.set(input.data, pointer);
        samples._bake_reset(pointer, size);
      } finally {
        if (pointer) samples._free(pointer);
        input.release();
      }
    },
    bake_voice: samples._bake_voice,
    bake_keyoff: samples._bake_keyoff,
    bake_active: samples._bake_active,
    bake_pull(buffer, frames) {
      const output = buffer.getBuffer("i16");
      const pointer = samples._malloc(frames * 4);
      try {
        if (!pointer || output.data.length < frames * 2) throw new Error("Buffer de áudio insuficiente.");
        samples._bake_pull(pointer, frames);
        output.data.set(samples.HEAP16.subarray(pointer >> 1, (pointer >> 1) + frames * 2));
      } finally {
        if (pointer) samples._free(pointer);
        output.release();
      }
    },
  };
}
