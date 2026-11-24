export const headerSize = 0x1000;

function crc16(bytes) {
  let crc = 0xffff;
  for (const byte of bytes) {
    crc ^= byte;
    for (let bit = 0; bit < 8; bit++) {
      crc = (crc >>> 1) ^ ((crc & 1) ? 0xa001 : 0);
    }
  }
  return crc;
}

export function validateNdsHeader(header, fileSize, assetCount) {
  if (header.length !== headerSize || fileSize < 0x4000 || header[0x12] !== 2) {
    throw new Error("O arquivo gerado não tem um cabeçalho válido para DS e DSi.");
  }

  const view = new DataView(header.buffer, header.byteOffset, header.byteLength);
  const word = offset => view.getUint32(offset, true);
  const half = offset => view.getUint16(offset, true);
  if (half(0x15e) !== crc16(header.subarray(0, 0x15e)) ||
      half(0x15c) !== crc16(header.subarray(0xc0, 0x15c))) {
    throw new Error("O cabeçalho do .nds falhou na verificação CRC.");
  }

  const regions = [];
  function addRegion(offset, length, name) {
    if (offset < 0x4000 || !length || offset + length > fileSize) {
      throw new Error(`A região ${name} do .nds está incompleta.`);
    }
    regions.push({offset, length, name});
  }

  for (const [record, name] of [[0x20, "ARM9"], [0x30, "ARM7"],
                             [0x1c0, "ARM9i"], [0x1d0, "ARM7i"]]) {
    addRegion(word(record), word(record + 12), name);
  }
  addRegion(word(0x40), word(0x44), "FNT");
  addRegion(word(0x48), word(0x4c), "FAT");

  if (word(0x44) < 8 || word(0x4c) !== assetCount * 8) {
    throw new Error("A tabela de arquivos NitroFS do .nds está incorreta.");
  }

  regions.sort((left, right) => left.offset - right.offset);
  for (let i = 1; i < regions.length; i++) {
    const previous = regions[i - 1];
    if (previous.offset + previous.length > regions[i].offset) {
      throw new Error(`As regiões ${previous.name} e ${regions[i].name} do .nds se sobrepõem.`);
    }
  }
  return {
    names: {offset: word(0x40), length: word(0x44)},
    allocations: {offset: word(0x48), length: word(0x4c)},
    regions,
  };
}

export function validateNitroFs(names, allocations, fileSize, expected, regions = []) {
  const fnt = new DataView(names.buffer, names.byteOffset, names.byteLength);
  const fat = new DataView(allocations.buffer, allocations.byteOffset, allocations.byteLength);
  if (fnt.byteLength < 8 || fat.byteLength !== expected.size * 8) {
    throw new Error("As tabelas NitroFS do .nds estão incompletas.");
  }

  const directoryCount = fnt.getUint16(6, true);
  const namesStart = directoryCount * 8;
  if (!directoryCount || directoryCount > 0x1000 || namesStart >= names.length) {
    throw new Error("Diretório principal NitroFS inválido.");
  }

  const directories = [{id: 0, prefix: ""}];
  const visitedDirectories = new Set([0]);
  const visitedFiles = new Set();
  const pending = new Map(expected);
  const payload = [];
  for (const {id, prefix} of directories) {
    const row = id * 8;
    let offset = fnt.getUint32(row, true);
    let fileId = fnt.getUint16(row + 4, true);
    if (offset < namesStart || offset >= names.length) {
      throw new Error("Endereço de diretório NitroFS inválido.");
    }

    while (offset < names.length && names[offset]) {
      const entry = names[offset++];
      const length = entry & 0x7f;
      const childDirectory = (entry & 0x80) !== 0;
      if (!length || offset + length + (childDirectory ? 2 : 0) >= names.length) {
        throw new Error("Entrada NitroFS truncada.");
      }
      const name = String.fromCharCode(...names.subarray(offset, offset + length));
      if (!/^[A-Za-z0-9_][A-Za-z0-9_.-]*$/.test(name)) {
        throw new Error("Nome NitroFS inválido.");
      }
      const path = prefix + name;
      offset += length;

      if (childDirectory) {
        const reference = fnt.getUint16(offset, true);
        const child = reference - 0xf000;
        offset += 2;
        if (child <= 0 || child >= directoryCount || visitedDirectories.has(child) ||
            fnt.getUint16(child * 8 + 6, true) !== 0xf000 + id) {
          throw new Error("Referência de diretório NitroFS inválida.");
        }
        visitedDirectories.add(child);
        directories.push({id: child, prefix: path + "/"});
        continue;
      }

      if (fileId >= expected.size || visitedFiles.has(fileId) || !pending.has(path)) {
        throw new Error(`Arquivo NitroFS inesperado: ${path}`);
      }
      const start = fat.getUint32(fileId * 8, true);
      const end = fat.getUint32(fileId * 8 + 4, true);
      if (start < 0x4000 || end > fileSize || end < start || end - start !== pending.get(path)) {
        throw new Error(`Tamanho ou endereço NitroFS incorreto: ${path}`);
      }
      if (end > start) payload.push({offset: start, length: end - start, name: path});
      visitedFiles.add(fileId++);
      pending.delete(path);
    }
    if (offset === names.length) throw new Error("Diretório NitroFS sem terminador.");
  }

  if (pending.size || directories.length !== directoryCount) {
    throw new Error("Faltam arquivos ou diretórios no NitroFS do .nds.");
  }
  const all = [...regions, ...payload].sort((left, right) => left.offset - right.offset);
  for (let i = 1; i < all.length; i++) {
    if (all[i - 1].offset + all[i - 1].length > all[i].offset) {
      throw new Error(`Regiões NitroFS sobrepostas: ${all[i - 1].name} e ${all[i].name}.`);
    }
  }
}
