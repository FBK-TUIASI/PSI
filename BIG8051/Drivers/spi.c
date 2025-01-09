#include <C8051F040.h>                 // Declaratii SFR
#include <osc.h>
#include <spi.h>

//-----------------------------------------------------------------------------
// SPI_Init
//-----------------------------------------------------------------------------
// Configureaza SPI0 va master unic în modul cu 4 fire, CKPOL=0, CKPHA=0
// (data centrata pe primul front si SCK inactiv pe nivel 0)
//-----------------------------------------------------------------------------
void SPI_Init(long int spi_clock){
   unsigned char SFRPAGE_save = SFRPAGE; // Salveaza SFRPAGE

   SFRPAGE = SPI0_PAGE;                // Comuta SFRPAGE

   SPI0CFG   = 0x40;                   // Configureaza SPI0 ca master, CKPHA=0,CKPOL=0
   SPI0CN    = 0x0D;                   // Master unic cu 4 fire, validare SPI0

   SPI0CKR   = (SYSCLK/(2*spi_clock))- 1;	// programeaza frecventa semnalului de tact SPI0

   SFRPAGE = SFRPAGE_save;             // Reface SFRPAGE
}

//-----------------------------------------------------------------------------
// SPI_Write
//-----------------------------------------------------------------------------
// Scrie un singur octet pentru a fi transmis catre slave
//-----------------------------------------------------------------------------
void SPI_Write(unsigned char spi_data) reentrant {

   unsigned char EA_save = EA;	// salveaza starea sistemului de intrerupere

	unsigned char SFRPAGE_save = SFRPAGE; // Salveaza SFRPAGE

   SFRPAGE = SPI0_PAGE;                	// Comuta SFRPAGE

	EA = 0;						// dezactivare globala intreruperi
	SPIF = 0;					// sterge flag-ul de intrerupere SPIF

 	do{
        WCOL = 0; 			 // Sterge bitul WCOL
        SPI0DAT = spi_data; // incarca bufferul
   }
	while(WCOL);			// incearca din nou daca a aparut coliziune

   while(SPIF == 0);		// Asteapta pana este setat flag-ul de intrerupere
	SPIF = 0;				// Sterge flag-ul d eintrerupere
 
	if(EA_save)	EA = 1;

   SFRPAGE = SFRPAGE_save;  // Reface starea sistemului de intreruperi
}

//-----------------------------------------------------------------------------
// SPI_Read
//-----------------------------------------------------------------------------
// Citeste octetul receptionat
//-----------------------------------------------------------------------------
unsigned char SPI_Read(void) reentrant {
   unsigned char ch, SFRPAGE_save = SFRPAGE; // Salveaza SFRPAGE

   SFRPAGE = SPI0_PAGE;    // Comuta SFRPAGE

 	SPI_Write(0);				// transmite un octet dummy
   ch = SPI0DAT;				// preia octetul receptionat

	SFRPAGE = SFRPAGE_save; // Reface SFRPAGE
   return ch;
}
