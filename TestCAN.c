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

extern CAN_Msg CAN_MsgTX, CAN_MsgRX;	// structuri de transmisie  si receptie mesaje CAN

//************************************************************************************
// Programul principal
//************************************************************************************

void BIG8051_Init(void){

	WDT_Disable();							// dezactiveaza WDT
   SYSCLK_Init();							// initializeaza si selecteaza oscilatorul ales in osc.h
   PORT_Init ();							// conecteaza perifericele la pini si stabileste tipul pinilor
	LCD_Init();    						// 2 linii, display ON, cursor OFF, pozitia initiala (0,0)
	KEYB_Init();							// initializeaza driver tastatura matriciala
	Timer0_Init();							// initializeaza Timer 0 - intreruperi la 1 ms
	UART0_Init(EIGHT_BIT, 9600);		// initializeaza UART0 - conectare terminal la UART-USB
	CAN_Init();								// initializeaza controlerul CAN, 1 Mbit/sec
	EA = 1;									// validare globala intreruperi
}

//************************************************************************************
// Programul principal
//************************************************************************************
void main (void) {
	unsigned char tasta;

	BIG8051_Init();							// Initializare resurse HW & SW BIG8051

	CAN_Init_Object(1, CANID_STD, 0x001, 1, MSGOBJ_TX);	 // MasgObj = 1, STD, CANID = 0, DLC = 1, TX
 	CAN_Init_Object(2, CANID_STD, 0x000, 1, MSGOBJ_RX);	 // MasgObj = 2, STD, CANID = 1, DLC = 1, RX
	
	while(1){

		if (tasta = KEYB_Input()){
			
			CAN_MsgTX.Date.Byte[0] = tasta;			// Pune in buffer octetul de transmis
			CAN_Transmit(1,0,&CAN_MsgTX);				// Transmite datele din buffer cu obiectul 1
	
			UART0_Putstr("\r\n\r\nTransmis: ");
			UART0_Putch(CAN_MsgTX.Date.Byte[0]);				// Afiseaza pe terminal octetul transmis
			LCD_PutStr(0,0, "Transmis: "); 						
			LCD_PutByte(LCD_line, LCD_col, CAN_MsgTX.Date.Byte[0]); 		// Afiseaza pe LCD octetul transmis
		
		}
		if(CAN_MsgRX.NewDat){									// S-au primit date noi?
			CAN_MsgRX.NewDat = 0;									// daca da, sterge indicatorul date noi
			
			UART0_Putstr("\r\nPrimit: ");
			UART0_Putch(CAN_MsgRX.Date.Byte[0]);				// afiseaza pe terminal octetul primit
			LCD_PutStr(1,0, "Primit: "); 							
			LCD_PutByte(LCD_line, LCD_col, CAN_MsgRX.Date.Byte[0]); 		// afiseaza pe LCD octetul primit
		}
	}
}