#!/usr/bin/env python3
"""
生成 Unicode(CJK) → GB2312 映射表 bin 文件
用于烧录到 W25Q128 外部 Flash，供 MCU 查表将 UTF-8 中文转为 GB2312 编码

映射表结构：
  - 覆盖 Unicode 范围 0x4E00 ~ 0x9FFF (CJK 统一汉字)
  - 每个码位 2 字节 [GB2312_HIGH, GB2312_LOW]，无映射则为 [0x00, 0x00]
  - 总大小: (0x9FFF - 0x4E00 + 1) * 2 = 20992 * 2 = 41984 字节 ≈ 41KB

使用方法：
  python unicode_to_gb2312_table.py
  生成 unicode_gb2312_map.bin，通过串口烧录到 Flash 0x040000 地址
"""

import struct

UNICODE_START = 0x4E00
UNICODE_END   = 0x9FFF
OUTPUT_FILE   = "unicode_gb2312_map.bin"

def main():
    table = bytearray()
    mapped_count = 0

    for code_point in range(UNICODE_START, UNICODE_END + 1):
        char = chr(code_point)
        try:
            gb2312_bytes = char.encode('gb2312')
            if len(gb2312_bytes) == 2:
                table.append(gb2312_bytes[0])
                table.append(gb2312_bytes[1])
                mapped_count += 1
            else:
                table.append(0x00)
                table.append(0x00)
        except (UnicodeEncodeError, UnicodeDecodeError):
            table.append(0x00)
            table.append(0x00)

    with open(OUTPUT_FILE, 'wb') as f:
        f.write(table)

    print(f"Generated: {OUTPUT_FILE}")
    print(f"Table size: {len(table)} bytes")
    print(f"Mapped characters: {mapped_count}")
    print(f"Unicode range: U+{UNICODE_START:04X} ~ U+{UNICODE_END:04X}")

if __name__ == '__main__':
    main()
