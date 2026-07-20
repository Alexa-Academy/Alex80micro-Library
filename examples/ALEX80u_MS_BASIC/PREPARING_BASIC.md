# Preparing Microsoft BASIC for ALEX80u

The BASIC sources are not included in this repository.

A compatible version of the Microsoft BASIC sources, including Grant Searle's adaptations, can be obtained from Grant Searle's website.

Users are responsible for obtaining the sources legally and for complying with the applicable copyright and licence terms.

The following procedure describes the changes required to adapt the BASIC sources for use with the ALEX80u library.

## Requirements

- Python 3
- `z88dk-z80asm` installed and available either:
  - in your system `PATH`, or
  - in `~/z88dk/bin`

## Preparing the sources

1. Extract the BASIC source archive.

2. Copy all extracted files into:

   ```
   examples/ALEX80u_MS_BASIC/basic
   ```

3. The supplied `build_rom.py` script automatically converts assembler directives required by `z88dk-z80asm`.

   The original source files are **never modified**.

   The script automatically performs the following syntax conversions on temporary copies:

   - `.BYTE` → `BYTE`
   - `.WORD` → `WORD`
   - `.EQU` → `EQU`
   - `.ORG` → `ORG`
   - Adds missing `:` characters to label definitions when required.

## Required source modifications

The following functional changes must be applied before generating the ROM image.

### 1. Modify `intmini.asm`

Change:

```
basicStarted
```

to:

```
$2000
```

Change:

```
TEMPSTACK
```

to:

```
$80ED
```

Remove lines 16 through 30 containing the serial communication buffers, which are not used by ALEX80u.

Also remove the following routines:

- `SerialInt`
- `RST38` (Interrupt Mode 1 handler)

---

### 2. Replace the `RXA` routine

Replace the entire routine with:

```asm
RXA:
waitForChar:
        CALL    CKINCHAR
        JR      Z, waitForChar
        IN      A,($FF)
        RET
```

---

### 3. Replace the `TXA` routine

Replace the entire routine with:

```asm
TXA:
        LD      ($1FFF),A
        RET
```

Writing to address `$1FFF` is intercepted by the ALEX80u library and forwarded to the Arduino serial interface.

---

### 4. Replace the `CKINCHAR` routine

Replace the entire routine with:

```asm
CKINCHAR:
        IN      A,($FE)
        AND     $01
        CP      $00
        RET
```

---

### 5. Modify the `INIT` routine

Remove the following initialization code:

```asm
LD      HL,serBuf
LD      (serInPtr),HL
LD      (serRdPtr),HL
XOR     A
LD      (serBufUsed),A
LD      A,RTS_LOW
OUT     ($80),A
IM      1
EI
```

These initializations are not required because serial communication is handled by the ALEX80u library.

## Building the ROM

After completing the required modifications, execute:

```bash
python build_rom.py
```

The script assembles the modified sources and generates:

```
generated/basic_rom.h
```

The Arduino sketch can then be compiled normally.

## Notes

This repository intentionally does **not** include:

- Microsoft BASIC source code
- Microsoft BASIC ROM images
- Modified BASIC source files

Only the ALEX80u integration code, build tools and documentation are provided.