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
}
