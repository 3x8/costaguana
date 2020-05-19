#include "main.h"

//uint8_t deviceInfo[8] = {0x34,0x37,0x31,0x64,0x1f,0x06,0x06,0x01};  // stm32 device info
//uint8_t deviceInfo[8] = {0x34,0x37,0x31,0x64,0xf3,0x90,0x06,0x01};  // silabs device id
uint8_t deviceInfo[8] = {0x34,0x37,0x31,0x64,0xe8,0xb2,0x06,0x01};  // blheli_s identifier

char fourWayRxByte = 0;
bool fourWayCharReceived = false;
uint8_t fourWayRxBuffer[258];
uint8_t payloadBuffer[256];
uint16_t payloadSize;
bool payloadIncoming = false;


uint32_t cmdAddress;
uint16_t cmdLength;
crc_16_u cmdCrc16;
uint16_t cmdInvalid = 0;


void delayMicroseconds(uint32_t micros) {
  TIM2->CNT = 0;
  while (TIM2->CNT < micros) {
    // wait
  }
}

void jump() {
  uint32_t JumpAddress;
  pFunction JumpToApplication;

  __disable_irq();
  JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
  uint8_t value = *(uint8_t*)(EEPROM_START_ADDRESS);

  if (value != EEPROM_CONF_VERSION) {
    cmdInvalid = 0;
    return;
  }

  JumpToApplication = (pFunction) JumpAddress;
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
  JumpToApplication();
}

void makeCrc(uint8_t* pBuff, uint16_t length) {
  cmdCrc16.word=0;
  for (int i = 0; i < length; i++) {
    uint8_t xb = pBuff[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (((xb & 0x01) ^ (cmdCrc16.word & 0x0001)) !=0 ) {
        cmdCrc16.word = cmdCrc16.word >> 1;
        cmdCrc16.word = cmdCrc16.word ^ 0xA001;
      } else {
        cmdCrc16.word = cmdCrc16.word >> 1;
      }
      xb = xb >> 1;
    }
   }
}

bool checkCrc(uint8_t* pBuff, uint16_t length) {
  makeCrc(pBuff,length);
  if ((cmdCrc16.bytes[0] == pBuff[length])  && (cmdCrc16.bytes[1] == pBuff[length+1])) {
    return (true);
  } else {
    return (false);
  }
}

void fourWaySetReceive() {
  LL_GPIO_SetPinMode(INPUT_GPIO, INPUT_PIN, LL_GPIO_MODE_INPUT);
}

void fourWaySetTransmit() {
  LL_GPIO_SetPinMode(INPUT_GPIO, INPUT_PIN, LL_GPIO_MODE_OUTPUT);
}

void fourWayPutDeviceInfo() {
  fourWayPutBuffer(deviceInfo,sizeof(deviceInfo));
  fourWayPutChar(CMD_ACK_OK);
}

void decodeInput() {
  if (payloadIncoming) {
    if (checkCrc(fourWayRxBuffer,cmdLength)) {
      memcpy(payloadBuffer, fourWayRxBuffer, payloadSize);
      payloadIncoming = false;
      fourWayPutChar(CMD_ACK_OK);
    }
    return;
  }

  for (int i = 2; i < sizeof(fourWayRxBuffer); i++) {
    if ((fourWayRxBuffer[i] == 0x7d) && (fourWayRxBuffer[i - 1] == 0xf4) && (fourWayRxBuffer[i - 2] == 0x69)) {
      fourWayPutDeviceInfo();
      return;
    }
  }

  if (fourWayRxBuffer[0] == CMD_PROG_FLASH) {
    cmdLength = 2;
    if (checkCrc(fourWayRxBuffer,cmdLength)) {
      bootloaderFlashWrite((uint8_t*)payloadBuffer, sizeof(payloadBuffer),cmdAddress);
      fourWayPutChar(CMD_ACK_OK);
    }
    return;
  }

  // CMD, 00 , cmdAddressHiByte, cmdAddressLoByte, crcLoByte ,crcHiByte
  if (fourWayRxBuffer[0] == CMD_SET_ADDRESS) {
    cmdLength = 4;
    if (checkCrc((uint8_t*)fourWayRxBuffer,cmdLength)) {
      //ToDo
      //cmdAddress = 0x08000000 + (fourWayRxBuffer[2] << 8 | fourWayRxBuffer[3]);
      cmdAddress = EEPROM_START_ADDRESS;
      cmdInvalid = 0;
      fourWayPutChar(CMD_ACK_OK);
    }
    return;
  }

  // for reading buffer rx buffer 0 = command byte.
  if (fourWayRxBuffer[0] == CMD_SET_BUFFER) {
    cmdLength = 4;
    if (checkCrc((uint8_t*)fourWayRxBuffer,cmdLength)) {
      if (fourWayRxBuffer[2] == 0x01) {
        payloadSize = 256;
      } else {
        payloadSize = fourWayRxBuffer[3];
      }
      payloadIncoming = true;
      fourWaySetReceive();
    }
    return;
  }

  if (fourWayRxBuffer[0] == CMD_KEEP_ALIVE) {
    cmdLength = 2;
    if (checkCrc((uint8_t*)fourWayRxBuffer,cmdLength)){
      //ToDo Nack
      fourWayPutChar(CMD_ACK_OK);
    }
    return;
  }

  if (fourWayRxBuffer[0] == CMD_ERASE_FLASH) {
    cmdLength = 2;
    if (checkCrc((uint8_t*)fourWayRxBuffer,cmdLength)) {
      fourWayPutChar(CMD_ACK_OK);
    }
    return;
  }

  if (fourWayRxBuffer[0] == CMD_READ_EEPROM) {
    // noop
    return;
  }

  // for sending contents of flash memory at the memory location set in bootloader.c need to still set memory with data from set mem command
  if (fourWayRxBuffer[0] == CMD_READ_FLASH_SIL) {
    cmdLength = 2;
    if (checkCrc((uint8_t*)fourWayRxBuffer,cmdLength)) {
      uint16_t dataBufferSize = fourWayRxBuffer[1];
      if (dataBufferSize == 0) {
        dataBufferSize = 256;
      }
      uint8_t dataBuffer[dataBufferSize + 3];

      memset(dataBuffer, 0, sizeof(dataBuffer));
      bootloaderFlashRead((uint8_t*)dataBuffer , cmdAddress, dataBufferSize);
      makeCrc(dataBuffer,dataBufferSize);
      dataBuffer[dataBufferSize] = cmdCrc16.bytes[0];
      dataBuffer[dataBufferSize + 1] = cmdCrc16.bytes[1];
      dataBuffer[dataBufferSize + 2] = CMD_ACK_OK;
      fourWayPutBuffer(dataBuffer, dataBufferSize+3);
    }
    return;
  }

  cmdInvalid++;
  fourWayPutChar(CMD_ACK_KO);
}

void fourWayGetChar() {
  fourWaySetReceive();
  fourWayRxByte=0;

  while(!(INPUT_GPIO->IDR & INPUT_PIN)) {
    // wait for rx to go high
  }

  while((INPUT_GPIO->IDR & INPUT_PIN)){
    // wait for rx to go low
    if(TIM2->CNT > 250 && fourWayCharReceived){
      return;
    }
  }
  // wait to get the center of bit time
  delayMicroseconds(FOUR_WAY_BIT_TIME_HALF);

  uint8_t bitIndex = 0;
  while (bitIndex < 8) {
    delayMicroseconds(FOUR_WAY_BIT_TIME);
    fourWayRxByte = fourWayRxByte | ((( INPUT_GPIO->IDR & INPUT_PIN)) >> FOUR_WAY_SHIFT_AMOUNT) << bitIndex;
    bitIndex++;
  }
  // wait till the stop bit time begins
  delayMicroseconds(FOUR_WAY_BIT_TIME_HALF);
  fourWayCharReceived = true;
}

void fourWayPutChar(char data) {
  fourWaySetTransmit();
  LED_OFF(LED_RED);
  LED_ON(LED_BLUE);
  // start bit
  INPUT_GPIO->BRR = INPUT_PIN;
  uint8_t bitIndex = 0;
  while (bitIndex < 8) {
    delayMicroseconds(FOUR_WAY_BIT_TIME);
    if (data & 0x01) {
      INPUT_GPIO->BSRR = INPUT_PIN;
    }else{
      INPUT_GPIO->BRR = INPUT_PIN;
    }
    bitIndex++;
    data = data >> 1;
  }

  delayMicroseconds(FOUR_WAY_BIT_TIME);
  // stop bit
  INPUT_GPIO->BSRR = INPUT_PIN;
  LED_OFF(LED_BLUE);
  fourWaySetReceive();
}

void fourWayGetBuffer() {
  fourWayCharReceived = false;
  memset(fourWayRxBuffer, 0, sizeof(fourWayRxBuffer));

  for(int i = 0; i < sizeof(fourWayRxBuffer); i++){
    fourWayGetChar();
    if(TIM2->CNT >= 250) {
      break;
    }else{
      fourWayRxBuffer[i] = fourWayRxByte;
      if(i == 257) {
        cmdInvalid += 20;       // needs one hundred to trigger a jump but will be reset on next set cmdAddress commmand
      }
    }
  }
  decodeInput();
}

void fourWayPutBuffer(uint8_t *data, int cmdLength) {
  for(int i = 0; i < cmdLength; i++){
    fourWayPutChar(data[i]);
    delayMicroseconds(FOUR_WAY_BIT_TIME);
  }
  //fourWayPutChar(10);     // for new line
}


int main(void) {
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  // prefetch buffer enable
  FLASH->ACR |= FLASH_ACR_PRFTBE;

  systemClockConfig();
  systemGpioInit();
  systemTim2Init();
  ledInit();
  ledOff();

  while (true) {
    fourWayGetBuffer();
    if (cmdInvalid > 100) {
      LED_ON(LED_RED);
      jump();
    }
  }
}
