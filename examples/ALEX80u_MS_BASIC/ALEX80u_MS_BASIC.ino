// ALEX80u Basic FC rev_1.3c Multiboard With Alex80u Library
#include <SPI.h>
#include <ALEX80u.h>
#if __has_include("generated/basic_rom.h")
#include "generated/basic_rom.h"
#else
#error "basic_rom.h not found. Run build_rom.py first."
#endif

#define START_RAM   0x2000

// Automatically detects the Arduino board and uses the optimized code path.
#ifdef ARDUINO_AVR_UNO  // Uno R3
#define SER_SPEED 9600
#define SERIAL_BUFFER_SIZE 128
#define MCP_SPEED 8000000
#define RAM_SPEED 8000000
#define BOARD_MSG "ARDUINO UNO R3"
#endif

#ifdef ARDUINO_UNOR4_WIFI  // Uno R4 WIFI
#define SER_SPEED 115200
#define SERIAL_BUFFER_SIZE 1024
#define MCP_SPEED 10000000
#define RAM_SPEED 20000000
#define BOARD_MSG "ARDUINO UNO R4 WIFI"
#endif

#ifdef ARDUINO_MINIMA  // Uno R4 MINIMA
#define SER_SPEED 115200
#define SERIAL_BUFFER_SIZE 1024
#define MCP_SPEED 10000000
#define RAM_SPEED 20000000
#define BOARD_MSG "ARDUINO UNO R4 MINIMA"
#endif

ALEX80u a80u(RAM_SPEED, MCP_SPEED);

// Variables for the Z80 address and data buses.
uint16_t z_addr = 0x0000;
uint8_t z_data = 0x00;

// Data-bus pin direction, used by DataIN() and DataOUT() to avoid redundant operations.
bool data_dir = 1;  // 1 = INPUT | 0 = OUTPUT

bool rd_mem = 0;
bool wr_mem = 0;
bool rd_io = 0;

bool z_rd_prev = HIGH;
bool z_wr_prev = HIGH;

bool RD_state = LOW;
bool WR_state = LOW;

// Variables for serial communication.
uint8_t serialBuffer[SERIAL_BUFFER_SIZE];
uint16_t serialHead = 0;
uint16_t serialTail = 0;
uint16_t serialCount = 0;
bool serialOverflow = false;
uint8_t CLRSCR = 0x0C;  // Carattere per pulire il terminale seriale

bool popSerialByte(uint8_t &value) {
    if (serialCount == 0) {
        return false;
    }

    value = serialBuffer[serialTail];
    serialTail++;
    if (serialTail == SERIAL_BUFFER_SIZE) {
        serialTail = 0;
    }
    serialCount--;
    return true;
}

void pollSerialInput() {
    constexpr uint8_t MAX_BYTES_PER_LOOP = 8;
    uint8_t processed = 0;

    while (processed < MAX_BYTES_PER_LOOP && Serial.available() > 0) {
        const int received = Serial.read();
        if (received < 0) {
            break;
        }
        processed++;

        if (serialCount == SERIAL_BUFFER_SIZE) {
            serialOverflow = true;
            continue;
        }

        serialBuffer[serialHead] = static_cast<uint8_t>(received);
        serialHead++;
        if (serialHead == SERIAL_BUFFER_SIZE) {
            serialHead = 0;
        }
        serialCount++;
    }
}

void setup() {
    delay(1);
    Serial.begin(SER_SPEED); 
    const unsigned long serialTimeout = millis() + 1000;
    while (!Serial && millis() < serialTimeout) {}  // Timeout needed for R4 if a serial terminal is not connected

    a80u.begin_UNO();
    a80u.begin_RAM();
    a80u.begin_MCP();
    delay(1);

    int t = 0;
    while (!Serial || !Serial.available()) {  // Also services USB CDC when the terminal connects late
        if (t >= 5000) {
            Serial.write(CLRSCR);  // Clear the screen every 5 seconds
            Serial.println("Alex80u MS BASIC");
            Serial.println("PRESS ANY KEY TO CONTINUE...");
            t = 0;
        }
        delay(1);
        ++t;
    }

    char bin_ch = Serial.read();      // Read and discard the received character so it is not processed at startup
    Serial.println();
    Serial.println(BOARD_MSG);  // Board type in use
    Serial.print("START");
    for (uint8_t p = 0; p < 3; p++) {  // Delay to read the message (3 seconds)
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.write(CLRSCR);  // Clear Screen

    a80u.set_RST(LOW);  // RST LOW
    for (uint8_t rst_cnt = 0; rst_cnt < 16; ++rst_cnt) {  // Run several clock cycles with reset low
        a80u.set_CLK(HIGH); // CLK HIGH
        delay(1);
        a80u.set_CLK(LOW);  // CLK LOW
        delay(1);
    }
    a80u.set_RST(HIGH);  // RST HIGH
}

void loop() {
    const uint8_t cmd = a80u.read_CMD();
    const bool z_iorq = bitRead(cmd, 3);
    const bool z_mreq = bitRead(cmd, 4);
    const bool z_wr = bitRead(cmd, 5);
    const bool z_rd = bitRead(cmd, 7);

    // Set data_dir and the data bus correctly for every I/O request.
    if (data_dir == 0) {
        data_dir = 1;
        a80u.pinMode_DATA(INPUT);
    }

    a80u.set_CLK(HIGH);

    const bool rd_start = (z_rd == LOW && z_rd_prev == HIGH);
    const bool wr_start = (z_wr == LOW && z_wr_prev == HIGH);

    z_rd_prev = z_rd;
    z_wr_prev = z_wr;

    bool read_memory = false;
    bool write_memory = false;
    bool read_io = false;

    if (wr_start) {
        write_memory = (z_mreq == LOW);
    } else if (rd_start) {
        read_memory = (z_mreq == LOW);
        read_io = (z_iorq == LOW);
    }

    a80u.set_CLK(LOW);

    if (read_memory || write_memory || read_io) {
        z_addr = a80u.read_ADDR();
    }

    if (read_memory) {
         const uint8_t z_data = (z_addr < START_RAM)
            ? pgm_read_byte_near(intROM + z_addr)
            : a80u.read_RAM(z_addr);

        a80u.pinMode_DATA(OUTPUT);
        data_dir = false;
        a80u.write_DATA(z_data);  // Sends to the Z80 the data read from ROM or RAM
    } else if (write_memory) {      
        // Write Z80 data to Arduino RAM when the address is in the RAM area

        const uint8_t z_data = a80u.read_DATA();
        if (z_addr == START_RAM - 1) {
            Serial.write(z_data);
        } else if (z_addr >= START_RAM) { 
            a80u.write_RAM(z_addr, z_data);
        }
    } else if (read_io) {
        uint8_t lowAddr = z_addr;   // Prendo solo la prima metà poichè gli indirizzi degli I/O sono a 8 bit
        if (lowAddr == 0xFF) {    // Se Z80 vuole leggere dalla seriale un nuovo carattere (ho impostato indirizzo FF per la lettura seriale nel codice basic)
            if (popSerialByte(z_data)) {
                if (data_dir == 1) {
                    data_dir = 0;
                    a80u.pinMode_DATA(OUTPUT);
                }
                a80u.write_DATA(z_data); // Send the buffered character to the Z80
            }
        } else if (lowAddr == 0xFE) {  // If the Z80 checks whether serial input is available
            if (data_dir == 1) {
                data_dir = 0;
                a80u.pinMode_DATA(OUTPUT);
            }
            a80u.write_DATA(serialCount != 0);
        }
    }

    pollSerialInput();
}
