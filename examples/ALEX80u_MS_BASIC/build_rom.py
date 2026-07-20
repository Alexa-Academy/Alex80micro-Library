#!/usr/bin/env python3
"""Assemble ALEX80 MS BASIC and generate the Arduino PROGMEM header.

The private, git-ignored assembler sources live in ``basic`` and the generated
header is written to ``generated/basic_rom.h``.  The script requires
z88dk-z80asm, either in PATH or in ~/z88dk/bin.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


ROM_SIZE = 0x2000
FILL_BYTE = 0x00
MODULES = (
    ("intmini.asm", 0x0000),
    ("basic.asm", 0x0150),
)

EXAMPLE_DIR = Path(__file__).resolve().parent
SOURCES_DIR = EXAMPLE_DIR / "basic"
DEFAULT_OUTPUT = EXAMPLE_DIR / "generated" / "basic_rom.h"

ARRAY_RE = re.compile(
    r"const\s+PROGMEM\s+byte\s+intROM(?:\s*\[[^\]]*\])?\s*=\s*\{"
    r"(?P<body>.*?)"
    r"\};",
    re.DOTALL,
)
BYTE_RE = re.compile(r"0x([0-9A-Fa-f]{2})")
TASM_DIRECTIVE_RE = re.compile(r"\.(BYTE|WORD|EQU|ORG)\b", re.IGNORECASE)
TASM_LABEL_RE = re.compile(
    r"^(?P<label>[A-Za-z_.$][A-Za-z0-9_.$]*)"
    r"(?P<spacing>[ \t]+)"
    r"(?P<opcode>ADC|ADD|AND|BIT|CALL|CCF|CP|CPD|CPDR|CPI|CPIR|CPL|DAA|"
    r"DEC|DI|DJNZ|EI|EX|EXX|HALT|IM|IN|INC|IND|INDR|INI|INIR|JP|JR|LD|"
    r"LDD|LDDR|LDI|LDIR|NEG|NOP|OR|OTDR|OTIR|OUT|OUTD|OUTI|POP|PUSH|RES|"
    r"RET|RETI|RETN|RL|RLA|RLC|RLCA|RLD|RR|RRA|RRC|RRCA|RRD|RST|SBC|SCF|"
    r"SET|SLA|SLL|SRA|SRL|SUB|XOR)\b",
    re.IGNORECASE | re.MULTILINE,
)
ORG_RE = re.compile(
    r"^(?P<indent>[ \t]*)ORG\s+(?P<address>[^\s;]+)(?P<suffix>.*)$",
    re.IGNORECASE,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Assemble intmini.asm and basic.asm, merge them into an "
            "8 KiB ROM and generate the intROM PROGMEM header."
        )
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT,
        help=f"generated header (default: {DEFAULT_OUTPUT})",
    )
    parser.add_argument(
        "--assembler",
        type=Path,
        help="path to z88dk-z80asm (otherwise searched automatically)",
    )
    return parser.parse_args()


def find_assembler(explicit: Path | None) -> Path:
    if explicit is not None:
        candidate = explicit.expanduser().resolve()
        if candidate.is_file():
            return candidate
        raise RuntimeError(f"assembler not found: {candidate}")

    in_path = shutil.which("z88dk-z80asm")
    if in_path:
        return Path(in_path).resolve()

    home_install = Path.home() / "z88dk" / "bin" / "z88dk-z80asm"
    if home_install.is_file():
        return home_install.resolve()

    raise RuntimeError(
        "z88dk-z80asm not found; install z88dk or pass "
        "--assembler /path/to/z88dk-z80asm"
    )


def assemble_module(
    assembler: Path,
    source_name: str,
    sources_dir: Path,
    build_dir: Path,
) -> bytes:
    source = sources_dir / source_name
    if not source.is_file():
        raise RuntimeError(f"source not found: {source}")

    output_name = f"{source.stem}.bin"
    command = [
        str(assembler),
        "-b",
        f"-O={build_dir}",
        f"-o={output_name}",
        str(source),
    ]
    result = subprocess.run(command, text=True, capture_output=True)
    if result.returncode != 0:
        details = "\n".join(
            part.strip() for part in (result.stdout, result.stderr) if part.strip()
        )
        raise RuntimeError(f"assembly failed for {source_name}:\n{details}")

    output = build_dir / output_name
    if not output.is_file():
        raise RuntimeError(f"assembler did not create: {output}")
    return output.read_bytes()


def convert_repeated_orgs(source: str) -> str:
    """Replace later TASM ORG directives with explicit z88dk padding."""
    org_count = 0
    converted_lines: list[str] = []

    for line in source.splitlines(keepends=True):
        match = ORG_RE.match(line.rstrip("\r\n"))
        if match is not None:
            org_count += 1
            if org_count > 1:
                newline = line[len(line.rstrip("\r\n")) :]
                line = (
                    f"{match.group('indent')}DEFS {match.group('address')} - $"
                    f"{match.group('suffix')}{newline}"
                )
        converted_lines.append(line)

    return "".join(converted_lines)


def prepare_sources(build_dir: Path) -> Path:
    """Copy TASM-style sources and normalize their syntax for z88dk-z80asm."""
    prepared_dir = build_dir / "sources"
    shutil.copytree(SOURCES_DIR, prepared_dir)

    for source in prepared_dir.rglob("*.asm"):
        text = source.read_text(encoding="utf-8")
        converted = TASM_DIRECTIVE_RE.sub(lambda match: match.group(1), text)
        converted = convert_repeated_orgs(converted)
        converted = TASM_LABEL_RE.sub(
            lambda match: (
                f"{match.group('label')}:"
                f"{match.group('spacing')}{match.group('opcode')}"
            ),
            converted,
        )
        if converted != text:
            source.write_text(converted, encoding="utf-8")

    return prepared_dir


def build_rom(assembler: Path) -> tuple[bytes, list[tuple[str, int, int]]]:
    rom = bytearray([FILL_BYTE]) * ROM_SIZE
    ranges: list[tuple[int, int, str]] = []
    report: list[tuple[str, int, int]] = []

    with tempfile.TemporaryDirectory(prefix="alex80-basic-") as temporary:
        build_dir = Path(temporary)
        sources_dir = prepare_sources(build_dir)
        for source_name, origin in MODULES:
            binary = assemble_module(assembler, source_name, sources_dir, build_dir)
            end = origin + len(binary)

            if end > ROM_SIZE:
                raise RuntimeError(
                    f"{source_name} ends at 0x{end - 1:04X}, beyond "
                    f"the ROM limit 0x{ROM_SIZE - 1:04X}"
                )

            for other_start, other_end, other_name in ranges:
                if origin < other_end and end > other_start:
                    raise RuntimeError(
                        f"{source_name} [0x{origin:04X}, 0x{end - 1:04X}] "
                        f"overlaps {other_name} "
                        f"[0x{other_start:04X}, 0x{other_end - 1:04X}]"
                    )

            rom[origin:end] = binary
            ranges.append((origin, end, source_name))
            report.append((source_name, origin, len(binary)))

    return bytes(rom), report


def format_array(rom: bytes) -> str:
    lines = [
        "#pragma once",
        "",
        "#include <Arduino.h>",
        "",
        "// Generated by build_rom.py; do not edit byte values manually.",
        "const PROGMEM byte intROM[] = {",
    ]
    for offset in range(0, len(rom), 16):
        chunk = rom[offset : offset + 16]
        suffix = "," if offset + len(chunk) < len(rom) else ""
        lines.append("  " + ", ".join(f"0x{byte:02X}" for byte in chunk) + suffix)
    lines.append("};")
    return "\n".join(lines) + "\n"


def read_rom_header(path: Path) -> bytes | None:
    if not path.is_file():
        return None
    match = ARRAY_RE.search(path.read_text(encoding="utf-8"))
    if match is None:
        return None
    return bytes(int(value, 16) for value in BYTE_RE.findall(match.group("body")))


def main() -> int:
    args = parse_args()
    try:
        assembler = find_assembler(args.assembler)
        rom, modules = build_rom(assembler)

        output = args.output.expanduser()
        if not output.is_absolute():
            output = (Path.cwd() / output).resolve()
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(format_array(rom), encoding="utf-8")

        digest = hashlib.sha256(rom).hexdigest()
        print(f"Assembler: {assembler}")
        for name, origin, size in modules:
            print(
                f"{name}: {size} bytes, "
                f"0x{origin:04X}-0x{origin + size - 1:04X}"
            )
        print(f"ROM: {len(rom)} bytes")
        print(f"SHA-256: {digest}")
        print(f"Generated: {output}")

        current = read_rom_header(output)
        if current is None:
            print("Generated header comparison: intROM not found")
        elif current == rom:
            print("Generated header comparison: identical")
        else:
            common = min(len(current), len(rom))
            differences = sum(
                current[index] != rom[index] for index in range(common)
            ) + abs(len(current) - len(rom))
            print(
                f"Generated header comparison: different "
                f"({differences} byte positions, header size {len(current)})"
            )
        return 0
    except (OSError, RuntimeError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
