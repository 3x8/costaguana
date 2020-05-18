#include "bootloader.h"

void bootloaderFlashWrite(uint8_t *data, int length, uint32_t address) {
  uint16_t dataFlashBuffer[length / 2];
  memset(dataFlashBuffer, 0, length / 2);

  for (int i = 0; i < length / 2 ; i ++ ) {
    dataFlashBuffer[i] =  data[i*2+1] << 8 | data[i*2];   // make 16 bit
  }

  // unlock flash
  volatile uint32_t dataFlashLength = length / 2;
  while ((FLASH->SR & FLASH_SR_BSY) != 0) {
    // wait
  }

  if ((FLASH->CR & FLASH_CR_LOCK) != 0) {
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;
  }

  // erase page if address even divisable by 1024
  if ((address % 1024) == 0){
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = address;
    FLASH->CR |= FLASH_CR_STRT;
    while ((FLASH->SR & FLASH_SR_BSY) != 0){
      // wait
    }
    if ((FLASH->SR & FLASH_SR_EOP) != 0){
      FLASH->SR = FLASH_SR_EOP;
    } else{
      // error
    }
    FLASH->CR &= ~FLASH_CR_PER;
  }

  volatile uint32_t writeCounter = 0, index = 0;
  while (index < dataFlashLength) {
    FLASH->CR |= FLASH_CR_PG;
    *(__IO uint16_t*)(address+writeCounter) = dataFlashBuffer[index];
    while ((FLASH->SR & FLASH_SR_BSY) != 0) {
      // wait
    }
    if ((FLASH->SR & FLASH_SR_EOP) != 0) {
      FLASH->SR = FLASH_SR_EOP;
    } else{
    // error
    }
    FLASH->CR &= ~FLASH_CR_PG;
    writeCounter += 2;
    index++;
  }
  SET_BIT(FLASH->CR, FLASH_CR_LOCK);
}

void bootloaderFlashRead(uint8_t*  data , uint32_t address , int out_buff_len){;
  for (int i = 0; i < out_buff_len ; i ++) {
    data[i] = *(uint8_t*)(address + i);
  }
}
