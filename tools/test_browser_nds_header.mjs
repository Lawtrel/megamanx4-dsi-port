import assert from "node:assert/strict";
import {readFile} from "node:fs/promises";
import {headerSize, validateNdsHeader, validateNitroFs} from "./web/nds-header.mjs";

// Artificial coordinates only. No Nintendo logo, banner or executable bytes.
const fixture = new Uint8Array(headerSize);
const view = new DataView(fixture.buffer);
const size = 0x5000;
fixture[0x12] = 2;
for (const [record, offset] of [[0x20, 0x4000], [0x30, 0x4100],
                                [0x1c0, 0x4200], [0x1d0, 0x4300]]) {
  view.setUint32(record, offset, true);
  view.setUint32(record + 12, 0x100, true);
}
view.setUint32(0x40, 0x4400, true);
view.setUint32(0x44, 8, true);
view.setUint32(0x48, 0x4500, true);
view.setUint32(0x4c, 8, true);
view.setUint16(0x15c, 0xe155, true);
view.setUint16(0x15e, 0x260f, true);

assert.doesNotThrow(() => validateNdsHeader(fixture, size, 1));
assert.throws(() => validateNdsHeader(fixture.subarray(0, 0x200), size, 1), /cabeçalho/);
assert.throws(() => validateNdsHeader(fixture, 0x4308, 1), /incompleta/);
assert.throws(() => validateNdsHeader(fixture, size, 2), /NitroFS/);
const corrupt = fixture.slice();
corrupt[0] ^= 0xff;
assert.throws(() => validateNdsHeader(corrupt, size, 1), /CRC/);
const overlapping = fixture.slice();
new DataView(overlapping.buffer).setUint32(0x1c0, 0x4100, true);
assert.throws(() => validateNdsHeader(overlapping, size, 1), /sobrepõem/);

const nameTable = Buffer.alloc(37);
nameTable.writeUInt32LE(16, 0);
nameTable.writeUInt16LE(2, 6);
nameTable.writeUInt32LE(27, 8);
nameTable.writeUInt16LE(0xf000, 14);
nameTable[16] = 0x87;
nameTable.write("content", 17);
nameTable.writeUInt16LE(0xf001, 24);
nameTable[27] = 8;
nameTable.write("test.bin", 28);
const fat = Buffer.alloc(8);
fat.writeUInt32LE(0x4600, 0);
fat.writeUInt32LE(0x4620, 4);
const expected = new Map([["content/test.bin", 32]]);
assert.doesNotThrow(() => validateNitroFs(nameTable, fat, size, expected));
assert.throws(() => validateNitroFs(nameTable, fat, size,
                                   new Map([["content/lost.bin", 32]])), /inesperado/);
assert.throws(() => validateNitroFs(nameTable, fat, size,
                                   new Map([["content/test.bin", 33]])), /endereço/);
assert.throws(() => validateNitroFs(nameTable, fat, 0x4610, expected), /endereço/);
assert.throws(() => validateNitroFs(nameTable, fat, size, expected,
                                   [{offset: 0x4610, length: 0x20, name: "metadata"}]), /sobrepostas/);
const badDirectory = Buffer.from(nameTable);
badDirectory.writeUInt16LE(0xf001, 14);
assert.throws(() => validateNitroFs(badDirectory, fat, size, expected), /diretório/);

if (process.argv[2]) {
  const nds = await readFile(process.argv[2]);
  const tableLength = nds.readUInt32LE(0x4c);
  validateNdsHeader(nds.subarray(0, headerSize), nds.length, tableLength / 8);
}

console.log("PASS NDS header, CRC, DS/DSi sections, NitroFS file names, sizes and bounds");
