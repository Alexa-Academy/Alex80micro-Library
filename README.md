# ALEX80u Arduino Library

## Overview

The ALEX80u Arduino Library is designed to simplify the development and experimentation process with the ALEX80µ educational computer system.

ALEX80µ is a compact Z80-based educational computer designed as an Arduino Uno shield. Its purpose is to provide a practical and accessible way to explore how a classic microcomputer operates. By exposing the CPU, memory, buses, and input/output devices, ALEX80µ allows students, makers, and electronics enthusiasts to study computer architecture and low-level programming through hands-on experimentation. The Arduino Uno serves as a companion controller, simplifying program loading, debugging, and interaction with the Z80 system.

This library provides a high-level interface for controlling and interacting with the ALEX80µ hardware from an Arduino board, allowing developers to focus on their applications and experiments without having to manage all low-level hardware details.

## Features

- Easy initialization of the ALEX80µ hardware
- Memory read and write operations
- Program upload support
- Bus control functions
- Debug and diagnostic utilities
- Hardware abstraction for common ALEX80µ operations
- Designed for educational and learning purposes

## Hardware Requirements

The library is intended for use with:

- ALEX80µ educational computer
- Arduino Uno R3
- Arduino Uno R4 Minima and WiFi

## Get Your Hardware

Hardware for both the ALEX80µ and ALEX80 educational computer systems is available from the Alexa Academy online store:

https://shop.alexa-academy.it

* ALEX80µ: a compact Arduino Uno shield based on the Z80 microprocessor.
* ALEX80: a modular educational computer system that allows you to build a complete standalone Z80-based computer using dedicated CPU, memory, video, audio, and I/O boards.

These platforms are designed to provide a practical introduction to computer architecture, digital electronics, and low-level programming.

For the latest availability, pricing, and product information, please visit the Alexa Academy shop.

## Installation

### Arduino IDE

1. Download or clone this repository.
2. Copy the library folder into your Arduino `libraries` directory.
3. Restart the Arduino IDE.
4. Open one of the examples from the **Examples** menu.

## Documentation

Additional documentation, tutorials, videos, and educational material can be found on the Alexa Academy project channels and website.

## Contributing

Contributions, bug reports, feature requests, and suggestions are welcome.

Please open an issue or submit a pull request.

## License

This project is released under the terms of the license included in this repository.

## Microsoft BASIC Example

This example does not include the Microsoft BASIC source code or ROM image.

Before generating the ROM image, the BASIC boot code must be adapted to work with the ALEX80u library.

1. Obtain a compatible copy of the BASIC source files.
2. Apply the required ALEX80u boot code modifications.
3. Copy the required files into the `basic/` directory.
4. Run `build_rom.py` to generate `generated/basic_rom.h`.
5. Open `ALEX80u_MS_BASIC.ino` with the Arduino IDE and compile the sketch.

Documentation describing the required modifications will be published separately.

Users are responsible for obtaining the BASIC sources legally and for complying with the applicable copyright and licence terms.

## Learn More

Website: https://www.alexa-academy.it

Store: https://shop.alexa-academy.it

YouTube Channel:
https://www.youtube.com/@alexaacademyit

---

Originally developed by Flavio Carchedi (Tecnow).

Modified, expanded, and maintained by Paolo Godino (Alexa Academy).