void SerialFlash_init();								// dezactivare CS serial Flash
void SerialFlash_WriteEnable();						// validare scriere in serial Flash
unsigned char SerialFlash_IsWriteBusy();			// verifica daca s-a terminat ciclul de scriere
void SerialFlash_WriteByte(unsigned char _data, unsigned long address);	// scriere octet
void SerialFlash_WriteWord(unsigned int _data, unsigned long address);	// scriere cuvant de 16 biti
unsigned char SerialFlash_ReadID(void);			// citire cod de identificare chip
unsigned char SerialFlash_ReadByte(unsigned long address);	// citire octet
unsigned int SerialFlash_ReadWord(unsigned long address);	// citire cuvant de 16 biti
unsigned char SerialFlash_WriteArray(unsigned long address, unsigned char* pData, unsigned int nCount); // scriere bloc de date
void SerialFlash_ReadArray(unsigned long address, unsigned char* pData, unsigned int nCount); // citire bloc de date
void SerialFlash_ChipErase(void);					// stergere chip
void SerialFlash_ResetWriteProtection();			// resetare protectie la scriere
void SerialFlash_SectorErase(unsigned long address);	// stergere sector din care face parte adresa