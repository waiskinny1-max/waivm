#!/usr/bin/env python3
"""Reserved helper for v0.3 .waibc inspection."""
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: hexdump_waibc.py <file>", file=sys.stderr)
        return 2
    data = Path(sys.argv[1]).read_bytes()
    for offset in range(0, len(data), 16):
        chunk = data[offset:offset + 16]
        hex_part = " ".join(f"{b:02x}" for b in chunk)
        print(f"{offset:08x}  {hex_part}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
