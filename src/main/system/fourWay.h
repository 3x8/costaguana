#pragma once

#include "main.h"

#define FOUR_WAY_BAUD_RATE  19200
#define FOUR_WAY_BIT_TIME (1000000/FOUR_WAY_BAUD_RATE)
#define FOUR_WAY_BIT_TIME_HALF (500000/FOUR_WAY_BAUD_RATE)
#define FOUR_WAY_SHIFT_AMOUNT 2

// USE_SERIAL_4WAY_BLHELI_BOOTLOADER
#define CMD_RUN               0x00
#define CMD_PROG_FLASH        0x01
#define CMD_ERASE_FLASH       0x02
#define CMD_READ_FLASH_SIL    0x03
#define CMD_VERIFY_FLASH      0x03
#define CMD_VERIFY_FLASH_ARM  0x04
#define CMD_READ_EEPROM       0x04
#define CMD_PROG_EEPROM       0x05
#define CMD_READ_SRAM         0x06
#define CMD_READ_FLASH_ATM    0x07
#define CMD_KEEP_ALIVE        0xfd
#define CMD_SET_ADDRESS       0xff
#define CMD_SET_BUFFER        0xfe
// reversed
#define CMD_ACK_OK            0x30
#define CMD_ACK_KO            0xc1
#define CMD_ACK_CRC           0xc2


typedef union __attribute__ ((packed)) {
  uint8_t bytes[2];
  uint16_t word;
} crc_16_u;

extern uint16_t fourWayCmdInvalid;

void delayMicroseconds(uint32_t micros);
void fourWayCrcCalculate(uint8_t* pBuff, uint16_t length);
bool fourWayCrcCompare(uint8_t* pBuff, uint16_t length);
void fourWayConfigReceive();
void fourWayConfigTransmit();
void fourWayPutDeviceInfo();
void fourWayDecodeInput();
void fourWayGetChar();
void fourWayPutChar(char data);
void fourWayGetBuffer();
void fourWayPutBuffer(uint8_t *data, int cmdLength);
