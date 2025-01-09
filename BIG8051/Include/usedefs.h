//===============================================================================================
// Selectie module de aplicatie (se elimina comentariul)
//===============================================================================================
//#define USE_BUTTON_LED		// starea led-ului P7_0 urmareste starea butonului P7_7	
//#define USE_LED_BLINK		// starea led-ului P1_6 comuta periodic, la un numar de depasiri ale T2	
//#define USE_LCDTest		// un mesaj defileaza continuu pe LCD
//#define USE_UART0Test		// se transmite peridic mesajul "Hello world!" pe UART0
//#define USE_KEYBTest		// codul tastei apasate se afiseaza pe LCD si se transmite pe UART0 - foloseste T3
//#define USE_ANALOGTest	// valoare tensiunii aplicate de potentiometrul P3 la CH0 este afisata pe LCD si transmisa pe UART0
#define USE_CANTest		// un mesaj CAN se transmite periodic (1 singur octet cuprins intre '0'-'9'
									// un mesaj primit (1 singur caracter) se afiseaza pe LCD si se transmite pe UART0
#ifdef USE_LED_BLINK			
	#define USE_PORTS			// definitii utilizare periferice interne
#endif

#ifdef USE_BUTTON_LED
	#define USE_PORTS			// definitii utilizare periferice interne
#endif

#ifdef USE_UART0Test
	#define USE_UART0			// definitii utilizare periferice interne
#endif

#ifdef USE_LCDTest
	#define USE_PORTS			// definitii utilizare periferice interne
	#define USE_LCD
#endif

#ifdef USE_KEYBTest
	#define USE_PORTS			// definitii utilizare periferice interne si externe
	#define USE_KEYBOARD
	#define USE_UART0
	#define USE_LCD
#endif

#ifdef USE_CANTest
	#define USE_PORTS			// definitii utilizare periferice interne si externe
	#define USE_LCD
	#define USE_UART0
#endif

#ifdef USE_ANALOGTest
	#define USE_PORTS			// definitii utilizare periferice interne si externe
	#define USE_LCD
	#define USE_UART0
#endif