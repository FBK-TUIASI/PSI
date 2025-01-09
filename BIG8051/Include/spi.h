#define SPI_CLOCK SYSCLK / 2     // SPI clock	la frecventa maxima


void SPI_Init(long int spi_clock);
void SPI_Write (unsigned char spi_data) reentrant;
unsigned char SPI_Read(void) reentrant;