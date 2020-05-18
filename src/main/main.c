#include "main.h"


char messagereceived = 0;
uint16_t invalid_command = 0;

uint8_t deviceInfo[9] = {0x34,0x37,0x31,0x64,0x1f,0x06,0x06,0x01, 0x30};      // stm32 device info
//uint8_t deviceInfo[9] = {0x34,0x37,0x31,0x64,0xf3,0x90,0x06,0x01, 0x30};       // silabs device id
//uint8_t deviceInfo[9] = {0x34,0x37,0x31,0x64,0xe8,0xb2,0x06,0x01, 0x30};     // blheli_s identifier


uint8_t rxBuffer[258];
uint8_t payLoadBuffer[256];
char rxbyte = 0;
uint32_t address;

typedef union __attribute__ ((packed)) {
    uint8_t bytes[2];
    uint16_t word;
} uint8_16_u;

uint16_t len;
uint8_t received_crc_low_byte;
uint8_t received_crc_high_byte;
uint8_t calculated_crc_low_byte;
uint8_t calculated_crc_high_byte;
uint16_t payload_buffer_size;
char incoming_payload_no_command = 0;


void SystemClock_Config(void);
static void systemGpioInit(void);
static void systemTim2Init(void);

void processmessage(void);
void serialwriteChar(char data);
void sendString(uint8_t data[], int len);
void recieveBuffer();

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
  if (value != 0x01) {      // check first byte of eeprom to see if its programmed, if not do not jump
    invalid_command = 0;
    return;
  }

  JumpToApplication = (pFunction) JumpAddress;
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
  JumpToApplication();
}



void makeCrc(uint8_t* pBuff, uint16_t length) {
  static uint8_16_u CRC_16;
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
  calculated_crc_low_byte = CRC_16.bytes[0];
  calculated_crc_high_byte = CRC_16.bytes[1];
}

char checkCrc(uint8_t* pBuff, uint16_t length) {

  char received_crc_low_byte2 = pBuff[length];          // one higher than len in buffer
  char received_crc_high_byte2 = pBuff[length+1];

  makeCrc(pBuff,length);

  if ((calculated_crc_low_byte==received_crc_low_byte2)  && (calculated_crc_high_byte==received_crc_high_byte2)) {
    return (1);
  } else {
    return (0);
  }
}


void setReceive() {
  systemGpioInit();
}

void setTransmit() {
  LL_GPIO_SetPinMode(INPUT_GPIO, INPUT_PIN, LL_GPIO_MODE_OUTPUT);       // set as reciever // clear bits and set receive bits..
}

void send_ACK() {
  setTransmit();
  serialwriteChar(0x30); // good ack!
  setReceive();
}

void sendDeviceInfo() {
  setTransmit();
  sendString(deviceInfo,9);
  setReceive();
}


void decodeInput() {
  if(rxBuffer[20] == 0x7d) {
    if(rxBuffer[12] == 13 && rxBuffer[13] == 66) {
      sendDeviceInfo();
      rxBuffer[20]= 0;
    }
    return;
  }
  if(rxBuffer[40] == 0x7d) {
    if(rxBuffer[32] == 13 && rxBuffer[33] == 66) {
      sendDeviceInfo();
      rxBuffer[20]= 0;
    }
    return;
  }

  if(rxBuffer[0] == CMD_PROG_FLASH) {
    len = 2;

    if(checkCrc(rxBuffer,len)) {
      bootloaderFlashWrite((uint8_t*)payLoadBuffer, payload_buffer_size,address);
      send_ACK();
      return;
    }
  }

  if(incoming_payload_no_command) {
    len = payload_buffer_size;
    if(checkCrc(rxBuffer,len)) {
      memset(payLoadBuffer, 0, sizeof(payLoadBuffer)); // reset buffer

      for(int i = 0; i < len; i++){
        payLoadBuffer[i]= rxBuffer[i];
      }
      send_ACK();
      incoming_payload_no_command = 0;
      return;
    }
  }

  if(rxBuffer[0] == CMD_SET_ADDRESS){ //  command set addressinput format is: CMD, 00 , High byte address, Low byte address, crclb ,crchb
    len = 4;  // package without 2 byte crc
    address = 0x08000000 + (rxBuffer[2] << 8 | rxBuffer[3]);

    if(checkCrc((uint8_t*)rxBuffer,len)) {
      send_ACK(); // will send Ack 0x30 and read input after transfer out callback
      invalid_command = 0;
    }

    return;
  }

  if(rxBuffer[0] == CMD_SET_BUFFER) { // for reading buffer rx buffer 0 = command byte.  command set address, input , format is CMD, 00 , High byte address, Low byte address,
    len = 4;  // package without 2 byte crc
      if(checkCrc((uint8_t*)rxBuffer,len)) {        // no ack with command set buffer;
        if(rxBuffer[2] == 0x01) {
          payload_buffer_size = 256;                          // if nothing in this buffer
        } else {
          payload_buffer_size = rxBuffer[3];
        }
        incoming_payload_no_command = 1;
        setReceive();
        return;
     }
  }

  if(rxBuffer[0] == CMD_KEEP_ALIVE) {
    len = 2;
    if(checkCrc((uint8_t*)rxBuffer,len)){
      setTransmit();
      serialwriteChar(0xC1); // bad command message.
      setReceive();
    }
    return;
  }

  if(rxBuffer[0] == CMD_ERASE_FLASH){
    len = 2;
    if(checkCrc((uint8_t*)rxBuffer,len)){
      send_ACK();
    }
    return;
  }

  if(rxBuffer[0] == CMD_READ_EEPROM) {
    // noop
  }



  if(rxBuffer[0] == CMD_READ_FLASH_SIL) {     // for sending contents of flash memory at the memory location set in bootloader.c need to still set memory with data from set mem command
    len = 2;
    uint16_t out_buffer_size = rxBuffer[1];
    if(out_buffer_size == 0){
      out_buffer_size = 256;
    }

    if (checkCrc((uint8_t*)rxBuffer,len)) {
      setTransmit();
      uint8_t read_data[out_buffer_size + 3];        // make buffer 3 larger to fit CRC and ACK
      memset(read_data, 0, sizeof(read_data));
      bootloaderFlashRead((uint8_t*)read_data , address, out_buffer_size);
      makeCrc(read_data,out_buffer_size);
      read_data[out_buffer_size] = calculated_crc_low_byte;
      read_data[out_buffer_size + 1] = calculated_crc_high_byte;
      read_data[out_buffer_size + 2] = 0x30;
      sendString(read_data, out_buffer_size+3);
      setReceive();
      return;
    }
  }

  setTransmit();
  serialwriteChar(0xC1); // bad command message.
  invalid_command++;
  setReceive();
}


void serialreadChar() {
  rxbyte=0;

  while(!(INPUT_GPIO->IDR & INPUT_PIN)) {
    // wait for rx to go high
  }

  while((INPUT_GPIO->IDR & INPUT_PIN)){
    // wait for rx to go low
    if(TIM2->CNT > 250 && messagereceived){
      return;
    }
  }

  delayMicroseconds(HALFBITTIME);//wait to get the center of bit time

  int bits_to_read = 0;
  while (bits_to_read < 8) {
    delayMicroseconds(BITTIME);
    rxbyte = rxbyte | ((( INPUT_GPIO->IDR & INPUT_PIN)) >> SHIFT_AMOUNT) << bits_to_read;
    bits_to_read++;
  }

  delayMicroseconds(HALFBITTIME); //wait till the stop bit time begins

  messagereceived = 1;
}


void serialwriteChar(char data) {
  INPUT_GPIO->BRR = INPUT_PIN;; //initiate start bit
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
  INPUT_GPIO->BSRR = INPUT_PIN; //write the stop bit
}

void sendString(uint8_t *data, int len) {
  for(int i = 0; i < len; i++){
    serialwriteChar(data[i]);
    delayMicroseconds(BITTIME);
  }
  //serialwriteChar(10);     // for new line
}


void recieveBuffer(){
  messagereceived = 0;
  memset(rxBuffer, 0, sizeof(rxBuffer));

  for(int i = 0; i < sizeof(rxBuffer); i++){
    serialreadChar();
    if(TIM2->CNT >=250) {

      break;
    }else{
      rxBuffer[i] = rxbyte;
      if(i == 257) {
        invalid_command+=20;       // needs one hundred to trigger a jump but will be reset on next set address commmand
      }
    }
  }
  decodeInput();
}

int main(void) {
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  FLASH->ACR |= FLASH_ACR_PRFTBE;   //// prefetch buffer enable

  SystemClock_Config();

  systemGpioInit();
  systemTim2Init();

  LL_TIM_EnableCounter(TIM2);

  setReceive();

  while (true) {
    recieveBuffer();
    if (invalid_command > 100) {
      jump();
    }
  }
}


void SystemClock_Config(void) {
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1){
    // wait
  }

  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1) {
    // wait
  }

  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2, LL_RCC_PLL_MUL_12);
  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1){
    // wait
  }

  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    // wait
  }

  LL_Init1msTick(48000000);
  LL_SetSystemCoreClock(48000000);
}


static void systemTim2Init(void) {
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  TIM_InitStruct.Prescaler = 47;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 0xFFFFFFFF;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM2);
  LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM2);
}


static void systemGpioInit(void) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

  GPIO_InitStruct.Pin = INPUT_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(INPUT_GPIO, &GPIO_InitStruct);
}
