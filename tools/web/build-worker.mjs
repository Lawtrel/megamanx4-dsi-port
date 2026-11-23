import {loadPyodide} from "./kit/pyodide.mjs";
import createNdstool from "./kit/ndstool.mjs";
import createXA from "./kit/xa.mjs";
import createSamples from "./kit/samples.mjs";
import createAtlas from "./kit/atlas.mjs";
import {bridge} from "./browser-native.mjs";
import {packageNds, validateIcon} from "./nds-package.mjs";

const kit = new URL("kit/", import.meta.url);
const status = message => self.postMessage({state: "working", message});

async function get(path, method) {
  const url = new URL(path, kit);
  if (url.origin !== kit.origin || !url.pathname.startsWith(kit.pathname)) {
    throw new Error("Caminho inválido no pacote de compilação.");
  }
  const response = await fetch(url);
  if (!response.ok) throw new Error(`Arquivo do compilador indisponível: ${path}`);
  return response[method]();
}

async function storage() {
  if (!navigator.storage?.getDirectory) return null;
  try {
    const root = await navigator.storage.getDirectory();
    const name = `mmx4-build-${crypto.randomUUID()}`;
    return {root, name, folder: await root.getDirectoryHandle(name, {create: true})};
  } catch {
    return null;
  }
}

async function assetBlob(fs, path, temporary, index) {
  if (!temporary) {
    const blob = new Blob([fs.readFile(path)]);
    fs.unlink(path);
    return blob;
  }
  const entry = await temporary.folder.getFileHandle(String(index), {create: true});
  if (!entry.createWritable) return assetBlob(fs, path, null, index);
  const writer = await entry.createWritable();
  const stream = fs.open(path, "r");
  try {
    const bytes = new Uint8Array(1024 * 1024);
    for (;;) {
      const count = fs.read(stream, bytes, 0, bytes.length);
      if (!count) break;
      await writer.write(bytes.subarray(0, count));
    }
    await writer.close();
  } catch (error) {
    await writer.abort().catch(() => {});
    throw error;
  } finally {
    fs.close(stream);
  }
  fs.unlink(path);
  return entry.getFile();
}

async function prepare(disc, temporary) {
  const files = await get("sources.json", "json");
  const pyodide = await loadPyodide({indexURL: kit.href});
  const fs = pyodide.FS;
  fs.mkdirTree("/project/disc");
  fs.mount(fs.filesystems.WORKERFS, {
    blobs: [{name: "original.bin", data: disc}],
  }, "/project/disc");
  for (const path of files) {
    const file = `/project/${path}`;
    fs.mkdirTree(file.slice(0, file.lastIndexOf("/")));
    fs.writeFile(file, new Uint8Array(await get(path, "arrayBuffer")));
  }

  const [xa, samples, atlas] = await Promise.all([createXA(), createSamples(), createAtlas()]);
  pyodide.registerJsModule("x4_browser_status", {status});
  pyodide.registerJsModule("x4_browser_native", bridge(fs, xa, samples, atlas));
  pyodide.setStdout({batched: () => {}});
  pyodide.setStderr({batched: message => console.error(message)});
  const script = await get("prepare.py", "text");
  pyodide.runPython(script);

  const output = pyodide.globals.get("asset_paths");
  try {
    const assets = [];
    for (const [name, path] of output.toJs()) {
      assets.push([name, await assetBlob(fs, path, temporary, assets.length)]);
    }
    return assets;
  } finally {
    output.destroy();
  }
}

self.onmessage = async ({data}) => {
  const temporary = await storage();
  let response;
  try {
    if (!(data.disc instanceof Blob)) throw new Error("Escolha um arquivo BIN.");
    if (data.icon) await validateIcon(data.icon);
    status("Carregando as ferramentas de compilação…");
    const assets = await prepare(data.disc, temporary);
    status("Montando o arquivo .nds…");
    const [arm9, arm7, icon] = await Promise.all([
      get("arm9.elf", "blob"), get("arm7.elf", "blob"),
      data.icon || get("icon.bmp", "blob"),
    ]);
    const file = await packageNds(createNdstool, {arm9, arm7, icon, assets});
    response = {state: "done", file};
  } catch (error) {
    console.error(error);
    response = {state: "error", message: String(error.message || error)};
  } finally {
    if (temporary) await temporary.root.removeEntry(temporary.name, {recursive: true}).catch(() => {});
    self.postMessage(response);
  }
};
