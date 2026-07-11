// ALEX80u counter with output to the onboard LEDs, using the ALEX80u library
#include <SPI.h>
#include <ALEX80u.h>

#define START_RAM   0x0300

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

/*******************
 Program for the Z80:
    LD SP, 0x400
    
    XOR A
loop:
    OUT (PORT_OUT), A
    INC A
    LD DE, 600
    CALL delay

    JP loop

delay:
    EX AF, AF'
delay1_loop:
    DEC DE
    LD A,D
    OR E
    JP NZ, delay1_loop
    EX AF, AF'
    RET
********************************************************************/
const PROGMEM byte intROM[] = {
    0x31, 0x00, 0x04, 0xAF, 0xD3, 0x00, 0x3C, 0x11, 0x58, 0x02, 0xCD, 0x10, 0x00, 0xC3, 0x04, 0x00,
    0x08, 0x1B, 0x7A, 0xB3, 0xC2, 0x11, 0x00, 0x08, 0xC9
};

ALEX80u a80u(RAM_SPEED, MCP_SPEED);

// Tracks the current direction of the Arduino data-bus pins
bool data_dir = 1;  // 1 = INPUT | 0 = OUTPUT

bool rd_mem = 0;
bool wr_mem = 0;
bool rd_io = 0;

bool z_rd_prev = HIGH;
bool z_wr_prev = HIGH;

void setup() {
    delay(1);
    Serial.begin(SER_SPEED); 
    while (!Serial);
    delay(1);

    a80u.begin_UNO();
    a80u.begin_RAM();
    a80u.begin_MCP();
    delay(1);

    Serial.println();
    Serial.println(BOARD_MSG);  // Prints the Arduino board
    Serial.println();

    // Reset the Z80
    a80u.set_RST(LOW); 
    for (uint8_t rst_cnt = 0; rst_cnt < 16; rst_cnt++) { 
        a80u.set_CLK(HIGH);
        delay(1);
        a80u.set_CLK(LOW);
        delay(1);
    }
    a80u.set_RST(HIGH);
}

void loop() {
    const uint8_t cmd = a80u.read_CMD();
    const bool z_iorq = bitRead(cmd, 3);
    const bool z_mreq = bitRead(cmd, 4);
    const bool z_wr = bitRead(cmd, 5);
    const bool z_rd = bitRead(cmd, 7);

    if (!data_dir) {
        a80u.pinMode_DATA(INPUT);
        data_dir = true;
    }

    a80u.set_CLK(HIGH);

    const bool rd_start = (z_rd == LOW && z_rd_prev == HIGH);
    const bool wr_start = (z_wr == LOW && z_wr_prev == HIGH);

    z_rd_prev = z_rd;
    z_wr_prev = z_wr;

    bool read_memory = false;
    bool write_memory = false;

    if (wr_start) {
        write_memory = (z_mreq == LOW);
    } else if (rd_start) {
        read_memory = (z_mreq == LOW);
    }

    a80u.set_CLK(LOW);

    if (read_memory) {                                                           
        const uint16_t z_addr = a80u.read_ADDR();
        const uint8_t z_data = (z_addr < START_RAM)
            ? pgm_read_byte_near(intROM + z_addr)
            : a80u.read_RAM(z_addr);

        a80u.pinMode_DATA(OUTPUT);
        data_dir = false;
        a80u.write_DATA(z_data);  // Sends to the Z80 the data read from ROM or RAM
    } else if (write_memory) { 
        const uint16_t z_addr = a80u.read_ADDR(); 
        
        // Write Z80 data to Arduino RAM when the address is in the RAM area
        if (z_addr >= START_RAM) { 
            const uint8_t z_data = a80u.read_DATA();
            a80u.write_RAM(z_addr, z_data);
        }
    }
}
