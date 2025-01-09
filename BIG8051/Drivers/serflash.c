/*
  Serial Flash driver
*/

/************************************************************************
* SerialFlash Commands
* -----------------------------------------------------------------------
* Overview: Constants that represent commands
* Input: none
* Output: none
************************************************************************/

#include <C8051F040.h>
#include <spi.h>
#include <serflash.h>

const unsigned short _SERIAL_FLASH_CMD_RDID  = 0x9F;  // 25P80
const unsigned short _SERIAL_FLASH_CMD_READ  = 0x03;
const unsigned short _SERIAL_FLASH_CMD_WRITE = 0x02;
const unsigned short _SERIAL_FLASH_CMD_WREN  = 0x06;
const unsigned short _SERIAL_FLASH_CMD_RDSR  = 0x05;
const unsigned short _SERIAL_FLASH_CMD_ERASE = 0xC7;   // 25P80
const unsigned short _SERIAL_FLASH_CMD_EWSR  = 0x06;   // 25P80
const unsigned short _SERIAL_FLASH_CMD_WRSR  = 0x01;
const unsigned short _SERIAL_FLASH_CMD_SER   = 0xD8;    //25P80

sbit CS_Serial_Flash_bit = P2^3;

/************************************************************************
* Function SerialFlash_init()
* -----------------------------------------------------------------------
* Overview: Function that initializes SerialFlash by setting Chip select
* Input: none
* Output: none
************************************************************************/
void SerialFlash_init(){
  CS_Serial_Flash_bit = 1;			
}

void SerialFlash_WriteEnable(){
  CS_Serial_Flash_bit = 0;
  SPI_Write(_SERIAL_FLASH_CMD_WREN);
  CS_Serial_Flash_bit = 1;
}


unsigned char SerialFlash_IsWriteBusy(){
  unsigned char temp;

  CS_Serial_Flash_bit = 0;
  SPI_Write(_SERIAL_FLASH_CMD_RDSR);
  temp = SPI_Read();
  CS_Serial_Flash_bit = 1;

  return (temp&0x01);
}

void SerialFlash_WriteByte(unsigned char _data, unsigned long address){
    SerialFlash_WriteEnable();
    
    CS_Serial_Flash_bit = 0;
    SPI_Write(_SERIAL_FLASH_CMD_WRITE);
    SPI_Write(address/65536);
    SPI_Write((address & 0xFFFF) / 256);
    SPI_Write(address % 256);
    SPI_Write(_data);
    CS_Serial_Flash_bit = 1;

    // Wait for write end
    while(SerialFlash_IsWriteBusy());
}

void SerialFlash_WriteWord(unsigned int _data, unsigned long address){
  SerialFlash_WriteByte(_data/256,address);
  SerialFlash_WriteByte(_data%256,address+1);
}

unsigned char SerialFlash_ReadID(void){
  unsigned char temp;

  CS_Serial_Flash_bit = 0;
  SPI_Write(_SERIAL_FLASH_CMD_RDID);
  temp = SPI_Read();
  CS_Serial_Flash_bit = 1;
  
  return temp;
}

unsigned char SerialFlash_ReadByte(unsigned long address){
  unsigned char temp;

  CS_Serial_Flash_bit = 0;

  SPI_Write(_SERIAL_FLASH_CMD_READ);
  SPI_Write(address/65536);
  SPI_Write((address & 0xFFFF) / 256);
  SPI_Write(address % 256);
  temp = SPI_Read();

  CS_Serial_Flash_bit = 1;
  return temp;
}

unsigned int SerialFlash_ReadWord(unsigned long address){
  return (SerialFlash_ReadByte(address) *256 + SerialFlash_ReadByte(address+1));

}

unsigned char SerialFlash_WriteArray(unsigned long address, unsigned char* pData, unsigned int nCount){
  unsigned long addr;
  unsigned char* pD;
  unsigned int counter;

  addr = address;
  pD   = pData;

  // WRITE

  for(counter = 0; counter < nCount; counter++){
      SerialFlash_WriteByte(*pD++, addr++);
  }


  // VERIFY

  for (counter=0; counter < nCount; counter++){
    if (*pData != SerialFlash_ReadByte(address))
        return 0;
    pData++;
    address++;
  }

  return 1;
}

void SerialFlash_ReadArray(unsigned long address, unsigned char* pData, unsigned int nCount){
  CS_Serial_Flash_bit = 0;
  SPI_Write(_SERIAL_FLASH_CMD_READ);
  SPI_Write(address % 65536);
  SPI_Write((address & 0xFFFF) / 256);
  SPI_Write(address % 256);
  
  while(nCount--){
    *pData++ = SPI_Read();
  }
  CS_Serial_Flash_bit = 1;
}

void SerialFlash_ChipErase(void){

  SerialFlash_WriteEnable();

  CS_Serial_Flash_bit = 0;
  SPI_Write(_SERIAL_FLASH_CMD_ERASE);
  CS_Serial_Flash_bit = 1;

  // Wait for write end
  while(SerialFlash_IsWriteBusy());
}

void SerialFlash_ResetWriteProtection(){

  CS_Serial_Flash_bit = 0;
  SPI_Write(_SERIAL_FLASH_CMD_EWSR);
  CS_Serial_Flash_bit = 1;


  CS_Serial_Flash_bit = 0;
  SPI_Write(_SERIAL_FLASH_CMD_EWSR);
  SPI_Write(0);
  CS_Serial_Flash_bit = 1;
}

void SerialFlash_SectorErase(unsigned long address){

  SerialFlash_WriteEnable();

  CS_Serial_Flash_bit = 0;
  SPI_Write(_SERIAL_FLASH_CMD_SER);
  SPI_Write(address / 65536);
  SPI_Write((address & 0xFFFF) / 256);
  SPI_Write(address %256);
  CS_Serial_Flash_bit = 1;

  // Wait for write end
  while(SerialFlash_IsWriteBusy());
}
