//------------------------------------------------------------------------------------
// NetCAN.c - aplicatie pentru microsistemul BIG8051 - Mikroelektronika
//------------------------------------------------------------------------------------

// se modifica CAN_Init() in fisierul can.c astfel incat CAN0CN = 0x41 in loc de 0xC1
// si BITREG = 0x7CF1 (BRP = 1 in loc de 0)

#include <c8051F040.h>		// declaratii SFR
 
#include <wdt.h>
#include <osc.h>
#include <port.h>
#include <timer.h>
#include <uart0.h>
#include <lcd.h>
#include <keyboard.h>
#include <can.h>
#include <stdio.h>
#include <stdbool.h>

#define DSC		'A'
#define WDI		'B'
#define RDI		'C'
#define CDI		'D'
#define IOCI	'#'

#define Ab		207.652
#define A		220
#define Bb		233.082
#define B		246.942
#define C		261.626
#define Db		277.183
#define D		293.665
#define Eb		311.127
#define E		329.628
#define F		349.228
#define Gb		369.994
#define G 		391.995

// #define US

#ifdef US
	#define USE_US	true
#endif

#ifndef US
	#define USE_US	false
#endif

extern CAN_Msg CAN_MsgTX, CAN_MsgRX;	// structuri de transmisie  si receptie mesaje CAN

sbit buz = P2^7;

//************************************************************************************
// INIT
//************************************************************************************

void BIG8051_Init(void){

	WDT_Disable();							// dezactiveaza WDT
	SYSCLK_Init();							// initializeaza si selecteaza oscilatorul ales in osc.h
	PORT_Init ();							// conecteaza perifericele la pini si stabileste tipul pinilor
	LCD_Init();    							// 2 linii, display ON, cursor OFF, pozitia initiala (0,0)
	KEYB_Init();							// initializeaza driver tastatura matriciala
	Timer0_Init();							// initializeaza Timer 0 - intreruperi la 1 ms
	UART0_Init(EIGHT_BIT, 9600);			// initializeaza UART0 - conectare terminal la UART-USB
	CAN_Init();								// initializeaza controlerul CAN, 1 Mbit/sec
	EA = 1;									// validare globala intreruperi
}
void wait(int t, bool in_us){
	(in_us == 0) ? Delay_us(t) : Delay(t);
}
//************************************************************************************
// Print menu
//************************************************************************************

void print_menu(){
	UART0_Putstr("\r\n");
	UART0_Putstr("Please select option from menu:			\r\n");
	UART0_Putstr("A. Diagnostic Session Control:\t\t0x10. \r\n");
	UART0_Putstr("B. Write Data by Identifier:		0x2E. \r\n");
	UART0_Putstr("C. Read Data by Identifier:			0x22. \r\n");
	UART0_Putstr("D. Clear Diagnostic Information:	0x14. \r\n");
	UART0_Putstr("#. I/O Control by Identifier:		0x2F. \r\n");
	UART0_Putstr("\r\n												\r\n");
}

void option(unsigned char opt){
	if(opt == DSC){
		UART0_Putstr("\t- Diagnostic Session Control: 	0x10. \r\n");
		LCD_PutStr(0,0, "Trimis: 0x10"); 		// afiseaza pe LCD octetul trimis	
		CAN_MsgTX.Date.Byte[0] = 0x10;			// Pune in buffer octetul de transmis
	}	
	else if(opt == WDI){
		UART0_Putstr("\t- Write Data by Identifier:		0x2E. \r\n");
		LCD_PutStr(0,0, "Trimis: 0x2E"); 		// afiseaza pe LCD octetul trimis	
		CAN_MsgTX.Date.Byte[0] = 0x2E;			// Pune in buffer octetul de transmis
	}	
	else if(opt == RDI){
		UART0_Putstr("\t- Read Data by Identifier:		0x22. \r\n");
		LCD_PutStr(0,0, "Trimis: 0x22"); 		// afiseaza pe LCD octetul trimis	
		CAN_MsgTX.Date.Byte[0] = 0x22;			// Pune in buffer octetul de transmis
	}	
	else if(opt == CDI){
		UART0_Putstr("\t- Clear Diagnostic Information:	0x14. \r\n");
		LCD_PutStr(0,0, "Trimis: 0x14"); 		// afiseaza pe LCD octetul trimis	
		CAN_MsgTX.Date.Byte[0] = 0x14;			// Pune in buffer octetul de transmis
	}
	else if(opt == IOCI){
		UART0_Putstr("\t- I/O Control by Identifier:		0x2F. \r\n");
		LCD_PutStr(0,0, "Trimis: 0x2F"); 		// afiseaza pe LCD octetul trimis	
		CAN_MsgTX.Date.Byte[0] = 0x2F;			// Pune in buffer octetul de transmis
	}
	else{
		UART0_Putstr("\t- Invalid option. \r\n");
		LCD_PutStr(0,0, "Trimis: 0xFF"); 		// afiseaza pe LCD octetul trimis	
		CAN_MsgTX.Date.Byte[0] = 0xFF;			// Pune in buffer octetul de transmis
	}
}

void buzzer(int t, double f, bool in_us){
	double period = 0;
	double cycles = 0;	
	unsigned int iter = 0;
	double s = 10^3;
	
	if(f == 0.0){
		Delay(t);
	}
	else{
		if(in_us) s *= s;
		period = s / f;
		cycles = (t * f) / s;

		for(iter = 0; iter < cycles; iter++){
			// jumi-juma buzzer on-off (PWM 50%)
			//	__    __    __    __     
			//	  |__|  |__|  |__|  |__| 
			buz = 1;
			wait((int)(period/2), in_us);
			buz = 0;
			wait((int)(period/2), in_us);
		}
	}
}

//************************************************************************************
// Do something
//************************************************************************************

void song(){
	int bpms = 1600;	// bit per ms

	buzzer(bpms/2, G, USE_US);
	buzzer(bpms/2^3, 0.0, USE_US);
	buzzer(bpms/2, Gb, USE_US);
	buzzer(bpms/2^3, 0.0, USE_US);
	buzzer(bpms/2, F, USE_US);
	buzzer(bpms/2^3, 0.0, USE_US);
	buzzer(bpms/2^3, E, USE_US);
	buzzer(bpms/2^5, 0.0, USE_US);
	buzzer(bpms/2^3, E, USE_US);
	buzzer(bpms/2^5, 0.0, USE_US);
	buzzer(bpms/2^3, E, USE_US);
	buzzer(bpms/2^5, 0.0, USE_US);
	buzzer(bpms/2^3, E, USE_US);
	buzzer(bpms/2^5, 0.0, USE_US);
}

//************************************************************************************
// Programul principal
//************************************************************************************
void main (void) {
	unsigned char tasta, counter = 0, iter = 0, buf[16], here = 0;
	unsigned int  tx = 0;

	BIG8051_Init();							// Initializare resurse HW & SW BIG8051

	CAN_Init_Object(1, CANID_STD, 0x001, 1, MSGOBJ_TX);	 // MasgObj = 1, STD, CANID = 0, DLC = 1, TX
 	CAN_Init_Object(2, CANID_STD, 0x000, 1, MSGOBJ_RX);	 // MasgObj = 2, STD, CANID = 1, DLC = 1, RX
	
	print_menu();
	while(1){
		if (tasta = KEYB_Input()){
			option(tasta);
			CAN_Transmit(1,0,&CAN_MsgTX);	
		}					
		if(CAN_MsgRX.NewDat){				// S-au primit date noi?	- da :)
			CAN_MsgRX.NewDat = 0;			// daca da, sterge indicatorul date noi
			
			UART0_Putstr("\r\nRaspuns primit: ");
			if((CAN_MsgTX.Date.Byte[0]  + 0x40) == CAN_MsgRX.Date.Byte[0]){
				UART0_PutByteHex(CAN_MsgRX.Date.Byte[0]);				// afiseaza pe terminal octetul primit
				UART0_Putstr("\t- OK");
			}
			else{
				UART0_PutByteHex(0x7F);				
				UART0_PutByteHex(CAN_MsgRX.Date.Byte[0]);	
				UART0_PutByteHex(0x11);	
				UART0_Putstr("\t - NOK!");

				// play something sad :)
				song();
			}
			UART0_Putstr("\r\n\r\n ");
			
			LCD_PutStr(1,0, "Primit: "); 							
			LCD_PutByte(1, LCD_col, CAN_MsgRX.Date.Byte[0]); 		// afiseaza pe LCD octetul primit
			
			Delay(1000);
			print_menu();

		}
	}
}