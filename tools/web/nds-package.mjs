// Browser worker adapter for the pinned ndstool WebAssembly build.
import {headerSize, validateNdsHeader} from "./nds-header.mjs";

export const outputName = "mmx4_ds_beta.nds";
const title = "Megaman X4 Ds;Native ARM9 engine;Beta 9";

export async function validateIcon(icon) {
  const header = new DataView(await icon.slice(0, 54).arrayBuffer());
  if (header.byteLength !== 54 || header.getUint16(0, true) !== 0x4d42 ||
      header.getUint32(14, true) !== 40 || header.getInt32(18, true) !== 32 ||
      Math.abs(header.getInt32(22, true)) !== 32 || header.getUint16(26, true) !== 1 ||
      header.getUint16(28, true) !== 4 || header.getUint32(30, true) !== 0) {
    throw new Error("Selecione um BMP indexado de 32×32 e 4 bits, sem compressão.");
  }
  const offset = header.getUint32(10, true);
  if (offset < 118 || offset + 512 > icon.size) throw new Error("O BMP está incompleto.");
}

export async function packageNds(createNdstool, {arm9, arm7, icon, assets}) {
  const files = [
    {name: "arm9.elf", data: arm9},
    {name: "arm7.elf", data: arm7},
    {name: "icon.bmp", data: icon},
  ];
  const paths = new Set();
  for (const [path, data] of assets) {
    const elements = path.split("/");
    if (path.length > 240 || elements.some(element => !/^[A-Za-z0-9_][A-Za-z0-9_.-]{0,126}$/.test(element))
        || paths.has(path) || !(data instanceof Blob)) {
      throw new Error(`Caminho inválido ou duplicado no NitroFS: ${path}`);
    }
    paths.add(path);
    files.push({name: `data/${path}`, data});
  }
  if (!paths.size) throw new Error("Nenhum arquivo foi preparado para o NitroFS.");

  const ndstool = await createNdstool();
  const fs = ndstool.FS;
  fs.mkdir("/input");
  fs.mount(fs.filesystems.WORKERFS, {blobs: files}, "/input");
  const output = `/${outputName}`;

  try {
    const result = ndstool.callMain([
      "-c", output, "-9", "/input/arm9.elf", "-7", "/input/arm7.elf",
      "-b", "/input/icon.bmp", title, "-d", "/input/data",
    ]);
    if (result) throw new Error(`Falha ao montar o .nds (código ${result}).`);

    // Open the output once and pass it to File in bounded pieces.
    const parts = [];
    const size = fs.stat(output).size;
    const stream = fs.open(output, "r");
    try {
      const header = new Uint8Array(headerSize);
      if (fs.read(stream, header, 0, header.length, 0) !== headerSize) {
        throw new Error("O cabeçalho do .nds está incompleto.");
      }
      validateNdsHeader(header, size, paths.size);
      for (let pos = 0; pos < size;) {
        const chunk = new Uint8Array(Math.min(1024 * 1024, size - pos));
        const read = fs.read(stream, chunk, 0, chunk.length, pos);
        if (read !== chunk.length) throw new Error("Arquivo .nds incompleto.");
        parts.push(chunk);
        pos += read;
      }
    } finally {
      fs.close(stream);
    }
    return new File(parts, outputName, {type: "application/octet-stream"});
  } finally {
    if (fs.analyzePath(output).exists) fs.unlink(output);
    fs.unmount("/input");
  }
}
