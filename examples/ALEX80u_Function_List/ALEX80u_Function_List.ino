#include "ALEX80u.h"

// UNO R3
#define MCP_SPEED 8000000
#define RAM_SPEED 8000000
#define SER_SPEED 9600

// UNO R4
//#define RAM_SPEED 20000000
//#define MCP_SPEED 10000000
//#define SER_SPEED 115200

ALEX80u a80u(RAM_SPEED, MCP_SPEED);

uint16_t Addr = 0x0000;
uint8_t Data = 0x00;
bool HALT, RFSH, M1, IORQ, MREQ, WR, BUSAK, RD;

void setup() {

  Serial.begin(SER_SPEED);
  while (!Serial)
    ;

  a80u.begin_UNO();  // Imposta Input ed Output di Arduino UNO
  a80u.begin_RAM();  // Inizializza la Ram
  a80u.begin_MCP();  // Imposta Input ed Output dei due Mcp

  a80u.set_RST(LOW);  // HIGH | LOW - Imposta lo stato del pin RST
  a80u.set_CLK(LOW);  // HIGH | LOW - Imposta lo stato del pin CLK

  a80u.set_INT(HIGH);    // HIGH | LOW - Imposta lo stato del pin INT
  a80u.set_NMI(HIGH);    // HIGH | LOW - Imposta lo stato del pin NMI
  a80u.set_BUSRQ(HIGH);  // HIGH | LOW - Imposta lo stato del pin BUSRQ
  a80u.set_WAIT(HIGH);   // HIGH | LOW - Imposta lo stato del pin WAIT

  uint8_t Cmd;
  a80u.read_BUS(Addr, Cmd);  // Snapshot unico di AddressBus e CommandBus
  HALT = bitRead(Cmd, 0);
  RFSH = bitRead(Cmd, 1);
  M1 = bitRead(Cmd, 2);
  IORQ = bitRead(Cmd, 3);
  MREQ = bitRead(Cmd, 4);
  WR = bitRead(Cmd, 5);
  BUSAK = bitRead(Cmd, 6);
  RD = bitRead(Cmd, 7);

  Data = a80u.read_RAM(Addr);  // Lettura Ram
  a80u.write_RAM(Addr, Data);  // Scrittura Ram

  a80u.pinMode_DATA(OUTPUT);  // INPUT | OUTPUT - Direzione DataBus
  a80u.write_DATA(Data);      // Scrittura DataBus

  a80u.pinMode_DATA(INPUT);  // INPUT | OUTPUT - Direzione DataBus
  Data = a80u.read_DATA();   // Lettura DataBus
}

void loop() {
  // put your main code here, to run repeatedly:
}
