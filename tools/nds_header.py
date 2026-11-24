"""Small, platform-independent Nintendo DS header checks."""
import struct

def crc16(data):
    value = 0xffff
    for byte in data:
        value ^= byte
        for _ in range(8):
            value = (value >> 1) ^ (0xa001 if value & 1 else 0)
    return value


def validate_native_package(rom, nitro):
    """Verify the DS/DSi header, segment bounds and NitroFS payload sizes."""
    with rom.open("rb") as stream:
        header = stream.read(0x1000)
        size = rom.stat().st_size
        if len(header) != 0x1000 or header[0x12] != 2:
            raise ValueError("Cabeçalho DS/DSi inválido.")
        logo_crc, header_crc = struct.unpack_from("<HH", header, 0x15c)
        if logo_crc != crc16(header[0xc0:0x15c]) or header_crc != crc16(header[:0x15e]):
            raise ValueError("O cabeçalho do .nds falhou na verificação CRC.")

        def word(position):
            return struct.unpack_from("<I", header, position)[0]

        regions = [(word(offset), word(offset + 12)) for offset in (0x20, 0x30, 0x1c0, 0x1d0)]
        regions.extend(((word(0x40), word(0x44)), (word(0x48), word(0x4c))))
        end = 0x4000
        for offset, length in sorted(regions):
            if offset < end or not length or offset + length > size:
                raise ValueError("Segmento incompleto ou sobreposto no .nds.")
            end = offset + length

        expected = sorted(path.stat().st_size for path in nitro.rglob("*") if path.is_file())
        if not expected or word(0x44) < 8 or word(0x4c) != len(expected) * 8:
            raise ValueError("Tabelas de arquivos NitroFS inválidas.")
        stream.seek(word(0x48))
        entries = list(struct.iter_unpack("<II", stream.read(word(0x4c))))
        if sorted(stop - start for start, stop in entries) != expected:
            raise ValueError("O tamanho de um arquivo NitroFS é inválido.")
        occupied = [(offset, offset + length) for offset, length in regions]
        occupied.extend((start, stop) for start, stop in entries if start != stop)
        end = 0x4000
        for start, stop in sorted(occupied):
            if start < end or stop < start or stop > size:
                raise ValueError("Um arquivo NitroFS está truncado ou sobreposto.")
            end = stop
