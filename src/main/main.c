#include "main.h"

uint8_t deviceInfo[8] = {0x34,0x37,0x31,0x64,0x1f,0x06,0x06,0x01};  // stm32 device info
//uint8_t deviceInfo[8] = {0x34,0x37,0x31,0x64,0xf3,0x90,0x06,0x01};  // silabs device id
//uint8_t deviceInfo[8] = {0x34,0x37,0x31,0x64,0xe8,0xb2,0x06,0x01};  // blheli_s identifier


char rxByte = 0;
uint8_t rxBuffer[258];
uint8_t payloadBuffer[256];
uint16_t payloadSize;
bool payloadIncoming = false;
bool fourWayCharReceived = false;
uint8_16_u CRC_16;
uint32_t cmdAddress;
uint16_t cmdInvalid = 0;
uint16_t cmdLength;


void delayMicroseconds(uint32_t micros){
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
  uint8_t value = *(uint8_t*)(EEPROM_ADDRESS);

  //ToDo
  // check first byte of eeprom to see if its programmed, if not do not jump
  if (value != 0x01) {
    cmdInvalid = 0;
    return;
  }

  JumpToApplication = (pFunction) JumpAddress;
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
  JumpToApplication();
}

void makeCrc(uint8_t* pBuff, uint16_t length) {
  CRC_16.word=0;
  for (int i = 0; i < length; i++) {
    uint8_t xb = pBuff[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (((xb & 0x01) ^ (CRC_16.word & 0x0001)) !=0 ) {
        CRC_16.word = CRC_16.word >> 1;
        CRC_16.word = CRC_16.word ^ 0xA001;
      } else {
        CRC_16.word = CRC_16.word >> 1;
      }
      xb = xb >> 1;
    }
   }
}

bool checkCrc(uint8_t* pBuff, uint16_t length) {
  makeCrc(pBuff,length);
  if ((CRC_16.bytes[0] == pBuff[length])  && (CRC_16.bytes[1] == pBuff[length+1])) {
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

void fourWayPutAck() {
  fourWaySetTransmit();
  fourWayPutChar(0x30); // good ack!
  fourWaySetReceive();
}

void fourWayPutNack() {
  fourWaySetTransmit();
  fourWayPutChar(0xC1); // bad command message.
  fourWaySetReceive();
}

void fourWayPutDeviceInfo() {
  fourWaySetTransmit();
  fourWayPutBuffer(deviceInfo,sizeof(deviceInfo));
  fourWayPutAck();
  fourWaySetReceive();
}


void decodeInput() {
  if(rxBuffer[20] == 0x7d) {
    if(rxBuffer[12] == 13 && rxBuffer[13] == 66) {
      fourWayPutDeviceInfo();
      rxBuffer[20]= 0;
    }
    return;
  }
  if(rxBuffer[40] == 0x7d) {
    if(rxBuffer[32] == 13 && rxBuffer[33] == 66) {
      fourWayPutDeviceInfo();
      rxBuffer[20]= 0;
    }
    return;
  }

  if(rxBuffer[0] == CMD_PROG_FLASH) {
    cmdLength = 2;

    if(checkCrc(rxBuffer,cmdLength)) {
      bootloaderFlashWrite((uint8_t*)payloadBuffer, sizeof(payloadBuffer),cmdAddress);
      fourWayPutAck();
      return;
    }
  }

  if(payloadIncoming) {
    if(checkCrc(rxBuffer,cmdLength)) {
      memcpy(payloadBuffer, rxBuffer, payloadSize);

      fourWayPutAck();
      payloadIncoming = false;
      return;
    }
  }

  // CMD, 00 , cmdAddressHiByte, cmdAddressLoByte, crcLoByte ,crcHiByte
  if(rxBuffer[0] == CMD_SET_ADDRESS){
    cmdLength = 4;  // package without 2 byte crc
    cmdAddress = 0x08000000 + (rxBuffer[2] << 8 | rxBuffer[3]);

    if(checkCrc((uint8_t*)rxBuffer,cmdLength)) {
      fourWayPutAck();
      cmdInvalid = 0;
    }

    return;
  }

  // for reading buffer rx buffer 0 = command byte.
  if(rxBuffer[0] == CMD_SET_BUFFER) {
    cmdLength = 4;  // package without 2 byte crc
      if(checkCrc((uint8_t*)rxBuffer,cmdLength)) {        // no ack with command set buffer;
        if(rxBuffer[2] == 0x01) {
          payloadSize = 256;                          // if nothing in this buffer
        } else {
          payloadSize = rxBuffer[3];
        }
        payloadIncoming = true;
        fourWaySetReceive();
        return;
     }
  }

  if(rxBuffer[0] == CMD_KEEP_ALIVE) {
    cmdLength = 2;
    if(checkCrc((uint8_t*)rxBuffer,cmdLength)){
      //ToDo Nack
      fourWayPutAck();
    }
    return;
  }

  if(rxBuffer[0] == CMD_ERASE_FLASH){
    cmdLength = 2;
    if(checkCrc((uint8_t*)rxBuffer,cmdLength)){
      fourWayPutAck();
    }
    return;
  }

  if(rxBuffer[0] == CMD_READ_EEPROM) {
    // noop
  }


  // for sending contents of flash memory at the memory location set in bootloader.c need to still set memory with data from set mem command
  if(rxBuffer[0] == CMD_READ_FLASH_SIL) {
    cmdLength = 2;
    uint16_t dataBufferSize = rxBuffer[1];
    if(dataBufferSize == 0){
      dataBufferSize = 256;
    }

    if (checkCrc((uint8_t*)rxBuffer,cmdLength)) {
      fourWaySetTransmit();
      uint8_t dataBuffer[dataBufferSize + 3];        // make buffer 3 larger to fit CRC and ACK
      memset(dataBuffer, 0, sizeof(dataBuffer));
      bootloaderFlashRead((uint8_t*)dataBuffer , cmdAddress, dataBufferSize);
      makeCrc(dataBuffer,dataBufferSize);
      dataBuffer[dataBufferSize] = CRC_16.bytes[0];
      dataBuffer[dataBufferSize + 1] = CRC_16.bytes[1];
      dataBuffer[dataBufferSize + 2] = 0x30;
      fourWayPutBuffer(dataBuffer, dataBufferSize+3);
      fourWaySetReceive();
      return;
    }
  }

  fourWaySetTransmit();
  fourWayPutChar(0xC1); // bad command message.
  cmdInvalid++;
  fourWaySetReceive();
}


void fourWayGetChar() {
  rxByte=0;

  while(!(INPUT_GPIO->IDR & INPUT_PIN)) {
    // wait for rx to go high
  }

  while((INPUT_GPIO->IDR & INPUT_PIN)){
    // wait for rx to go low
    if(TIM2->CNT > 250 && fourWayCharReceived){
      return;
    }
  }

  delayMicroseconds(HALFBITTIME); // wait to get the center of bit time

  int bits_to_read = 0;
  while (bits_to_read < 8) {
    delayMicroseconds(BITTIME);
    rxByte = rxByte | ((( INPUT_GPIO->IDR & INPUT_PIN)) >> SHIFT_AMOUNT) << bits_to_read;
    bits_to_read++;
  }

  delayMicroseconds(HALFBITTIME); // wait till the stop bit time begins

  fourWayCharReceived = true;
}


void fourWayPutChar(char data) {
  LED_OFF(LED_RED);
  LED_ON(LED_BLUE);
  INPUT_GPIO->BRR = INPUT_PIN;; // initiate start bit
  char bits_to_read = 0;
  while (bits_to_read < 8) {
    delayMicroseconds(BITTIME);
    if (data & 0x01) {
      INPUT_GPIO->BSRR = INPUT_PIN;
    }else{
      INPUT_GPIO->BRR = INPUT_PIN;
    }
    bits_to_read++;
    data = data >> 1;
  }

  delayMicroseconds(BITTIME);
  INPUT_GPIO->BSRR = INPUT_PIN; // write the stop bit
  LED_OFF(LED_BLUE);
}

void fourWayPutBuffer(uint8_t *data, int cmdLength) {
  for(int i = 0; i < cmdLength; i++){
    fourWayPutChar(data[i]);
    delayMicroseconds(BITTIME);
  }
  //fourWayPutChar(10);     // for new line
}


void fourWayGetBuffer(){
  fourWayCharReceived = false;
  memset(rxBuffer, 0, sizeof(rxBuffer));

  for(int i = 0; i < sizeof(rxBuffer); i++){
    fourWayGetChar();
    if(TIM2->CNT >=250) {
      break;
    }else{
      rxBuffer[i] = rxByte;
      if(i == 257) {
        cmdInvalid+=20;       // needs one hundred to trigger a jump but will be reset on next set cmdAddress commmand
      }
    }
  }
  decodeInput();
}

int main(void) {
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  FLASH->ACR |= FLASH_ACR_PRFTBE;   // prefetch buffer enable

  systemClockConfig();
  systemGpioInit();
  ledInit();
  systemTim2Init();
  ledOff();

  LL_TIM_EnableCounter(TIM2);

  fourWaySetReceive();

  while (true) {
    fourWayGetBuffer();
    if (cmdInvalid > 100) {
      LED_ON(LED_RED);
      jump();
    }
  }
}
