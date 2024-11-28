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
	unsigned char tasta, counter = 0, iter = 0, buf[16];
	unsigned int  tx = 0;

	BIG8051_Init();							// Initializare resurse HW & SW BIG8051

	CAN_Init_Object(1, CANID_STD, 0x001, 1, MSGOBJ_TX);	 // MasgObj = 1, STD, CANID = 0, DLC = 1, TX
 	CAN_Init_Object(2, CANID_STD, 0x000, 1, MSGOBJ_RX);	 // MasgObj = 2, STD, CANID = 1, DLC = 1, RX
	
	while(1){

		if (tasta = KEYB_Input()){
			
//			if('0' < tasta < '9')
//				tasta -= '0';
//			else if('A' < tasta < 'D')
//				tasta -= 'A';
//			else if(tasta == '*')
//				tasta = 'E';
//			else if(tasta == '#')
//				tasta = 'F';	
			
			CAN_MsgTX.Date.Byte[counter] = tasta;			// Pune in buffer octetul de transmis
			counter++;
			if (counter >= 8)	{
				tx++;				
				counter = 0;
				CAN_Transmit(1,0,&CAN_MsgTX);				// Transmite datele din buffer cu obiectul 1
				sprintf(buf, "\r\n\r\nTransmis[%d]: %s", tx,CAN_MsgTX.Date.Byte);
				UART0_Putstr(buf);
				sprintf(buf, "Tx[%d]: %s", tx,CAN_MsgTX.Date.Byte);
				LCD_PutStr(0,0, buf); 						

//				for(iter = 0; iter > 8; iter++){
//					UART0_PutByteHex(CAN_MsgTX.Date.Byte[iter]);				// Afiseaza pe terminal octetul transmis
//					LCD_PutByteHex(0, 5 + iter, CAN_MsgTX.Date.Byte[iter]); 		// Afiseaza pe LCD octetul transmis
//				}
			}
		}
		if(CAN_MsgRX.NewDat){									// S-au primit date noi?
			CAN_MsgRX.NewDat = 0;									// daca da, sterge indicatorul date noi
			
			UART0_Putstr("\r\nPrimit: ");
			UART0_Putch(CAN_MsgRX.Date.Byte[0]);				// afiseaza pe terminal octetul primit
			LCD_PutStr(1,0, "Primit: "); 							
			LCD_PutByte(1, LCD_col, CAN_MsgRX.Date.Byte[0]); 		// afiseaza pe LCD octetul primit
		}
	}
}