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

#define MAIN_MENU		0
#define DSC_MENU		1
#define WDI_MENU		2	
#define RDI_MENU		3
#define CDI_MENU		4
#define IOCI_MENU		5


extern CAN_Msg CAN_MsgTX, CAN_MsgRX;	// structuri de transmisie  si receptie mesaje CAN

//************************************************************************************
// INIT
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
// Print menu
//************************************************************************************

void print_submenu(){
		UART0_Putstr("A. Subfunction 1: 	0x01. \r\n");
		UART0_Putstr("B. Subfunction 2:	0x02. \r\n");
		UART0_Putstr("C. Subfunction 3:	0x03. \r\n");
		UART0_Putstr("D. Subfunction 4:	0x04. \r\n");
		UART0_Putstr("\r\n");
		UART0_Putstr("0. To main menu: 	0x10. \r\n\r\n");
}


void print_menu(unsigned char menu){
	if(menu == MAIN_MENU){		
		UART0_Putstr("\r\n");
		UART0_Putstr("Please select option from menu:\r\n");
		UART0_Putstr("1. Diagnostic Session Control: 	0x10. \r\n");
		UART0_Putstr("2. Write Data by Identifier:		0x2E. \r\n");
		UART0_Putstr("3. Read Data by Identifier:			0x22. \r\n");
		UART0_Putstr("4. Clear Diagnostic Information:	0x14. \r\n");
		UART0_Putstr("5. I/O Control by Identifier:		0x2F. \r\n");
		UART0_Putstr("\r\n\r\n");
	}
	else if(menu == DSC_MENU){
		UART0_Putstr("\t- 1. Diagnostic Session Control: 	0x10. \r\n");
		print_submenu();
	}	
	else if(menu == WDI_MENU){
		UART0_Putstr("\t- 2. Write Data by Identifier:		0x2E. \r\n");
		print_submenu();
	}	
	else if(menu == RDI_MENU){
		UART0_Putstr("\t- 3. Read Data by Identifier:			0x22. \r\n");
		print_submenu();
	}	
	else if(menu == CDI_MENU){
		UART0_Putstr("\t- 4. Clear Diagnostic Information:	0x14. \r\n");
		print_submenu();
	}
	else if(menu == IOCI_MENU){
		UART0_Putstr("\t- 5. I/O Control by Identifier:		0x2F. \r\n");
		print_submenu();
	}
}


//************************************************************************************
// Do something
//************************************************************************************

void dsc(){
	unsigned char tasta, submenu[] = {'A', 'B', 'C', 'D'}, iter = 0, msg = 0x10FF, buf[20], flag = 0;
	while(1){
		if(tasta = KEYB_Input())
			for(iter = 0; iter < 4; iter++){
				if(tasta == submenu[iter]){
					msg  |= (tasta - 'A');
					sprintf(buf, "MSG = %x", msg);
					UART0_Putstr(buf);
					LCD_PutStr(0,0, buf); 
					flag = 1;
					break;
				}
			}
			if(flag == 1)
				break;
		}
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
	
	print_menu(MAIN_MENU);
	while(1){
		if (tasta = KEYB_Input()){
			if(tasta == '0' and here != MAIN_MENU){
				here = MAIN_MENU;
				print_menu(MAIN_MENU);
			}
			else if(tasta == '1' and here != DSC_MENU){
				here = DSC_MENU;
				print_menu(DSC_MENU);
				dsc();
			}
			else if(tasta == '2' and here != WDI_MENU){
				here = WDI_MENU;
				print_menu(WDI_MENU);
				wdi();
			}
			else if(tasta == '3' and here != RDI_MENU){
				here = RDI_MENU;
				print_menu(RDI_MENU);
				rdi();
			}
			else if(tasta == '4' and here != CDI_MENU){
				here = CDI_MENU;
				print_menu(CDI_MENU);
				
			}
			else if(tasta == '5' and here != IOCI_MENU){
				here = IOCI_MENU;
				print_menu(IOCI_MENU);
				ioci();
			}
//			
//			CAN_MsgTX.Date.Byte[counter] = tasta;			// Pune in buffer octetul de transmis
//			counter++;
//			if (counter >= 8)	{
//				tx++;				
//				counter = 0;
//				CAN_Transmit(1,0,&CAN_MsgTX);				// Transmite datele din buffer cu obiectul 1
//				sprintf(buf, "\r\n\r\nTransmis[%d]: %s", tx,CAN_MsgTX.Date.Byte);
//				UART0_Putstr(buf);
//				sprintf(buf, "Tx[%d]: %s", tx,CAN_MsgTX.Date.Byte);
//				LCD_PutStr(0,0, buf); 						

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