//------------------------------------------------------------------------------------
// NetCAN.c - aplicatie pentru microsistemul BIG8051 - Mikroelektronika
//------------------------------------------------------------------------------------

// se modifica CAN_Init() in fisierul can.c astfel incat CAN0CN = 0x41 in loc de 0xC1
// si BITREG = 0x7CF1 (BRP = 1 in loc de 0)

#include <stdio.h>
#include <c8051F040.h>		// declaratii SFR
 
#include <wdt.h>
#include <osc.h>
#include <port.h>
#include <timer.h>
#include <uart0.h>
#include <lcd.h>
#include <keyboard.h>
#include <can.h>

sbit led0 = P2 ^ 0;
sbit buzz = P2 ^ 7;



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

void buzzer(t){
    int ms = 0;
    while(ms < t/2)
    {
        buzz = 1;
        Delay(1);
        buzz = 0;
        Delay(1);
        ms++;
    }
}

void led_pulse(t){
    int ms = 0;
    while(ms < t/2)
    {
        led0 = 1;
        Delay(500);
        led0 = 0;
        Delay(500);
        ms++;
    }
}


//************************************************************************************
// Programul principal
//************************************************************************************
void main (void) {
	unsigned char tasta;
    unsigned char text[20];
    unsigned int ServicesList[5] = {0x10, 0x2E, 0x22, 0x2F, 0x14};
    unsigned char Negative_Response[8];
    int i;
    int Service_Flag = 0;
    led0 = 0;
	BIG8051_Init();							// Initializare resurse HW & SW BIG8051

	CAN_Init_Object(1, CANID_STD, 0x000, 1, MSGOBJ_TX);	 // MasgObj = 1, STD, CANID = 0, DLC = 1, TX
 	CAN_Init_Object(2, CANID_STD, 0x001, 1, MSGOBJ_RX);	 // MasgObj = 2, STD, CANID = 1, DLC = 1, RX
	
  
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
			for (i = 0; i < 5; i++)
            {
//                LCD_PutByte(1, 0, (unsigned int)CAN_MsgRX.Date.Byte[0]); 						
//                LCD_PutByte(1, 5, ServicesList[i]);
//                
                if((unsigned int)CAN_MsgRX.Date.Byte[0] == ServicesList[i])
                    Service_Flag = 1;
                
            }
            if(Service_Flag == 1)
                {
                    Service_Flag = 0;
                    //sprintf(text, " :: %04x", CAN_MsgRX.Date.Byte[0]);
                    UART0_Putstr("\r\nPrimit: ");
                    //UART0_Putstr(text);				// afiseaza pe terminal octetul primit  
                    UART0_PutByteHex(CAN_MsgRX.Date.Byte[0]);
                    CAN_MsgTX.Date.Byte[0] = (CAN_MsgRX.Date.Byte[0] + 0x40);
                    CAN_Transmit(1,0,&CAN_MsgTX);
                    if((unsigned int)CAN_MsgRX.Date.Byte[0] == 0x10)
                    {
                        LCD_PutStr(0, 0, "Request");
                        LCD_PutStr(1, 0, "Service 10     ");
                        buzzer(2000);
                    }
                    else if((unsigned int)CAN_MsgRX.Date.Byte[0] == 0x2E)
                    {
                        LCD_PutStr(0, 0, "Request");
                        LCD_PutStr(1, 0, "Service 2E     ");
                        led0 = 1;
                    }else if((unsigned int)CAN_MsgRX.Date.Byte[0] == 0x22)
                    {
                        LCD_PutStr(0, 0, "Request");
                        LCD_PutStr(1, 0, "Service 22     ");
                        led0 = 0;
                        
                    }else if((unsigned int)CAN_MsgRX.Date.Byte[0] == 0x2F)
                    {
                        LCD_PutStr(0, 0, "Request");
                        LCD_PutStr(1, 0, "Service 2F     ");
												led_pulse(5000);
                        
                    }else if((unsigned int)CAN_MsgRX.Date.Byte[0] == 0x14)
                    {
                        LCD_PutStr(0, 0, "Request");
                        LCD_PutStr(1, 0, "Service 14     ");
												Delay(3000);
												LCD_Clear();
												LCD_PutStr(0, 0, "Clear Diagnostic ");
                        LCD_PutStr(1, 0, "Information ");
												Delay(3000);
												LCD_Clear();
												LCD_PutStr(0, 0, "Clear Diagnostic ");
                        LCD_PutStr(1, 0, "Information ");
												Delay(3000);
												LCD_Clear();
												led0 = 0;
												buzz = 0;
                    }
                    
                }else                    
                {
                    LCD_PutStr(0, 0, "Request");
                    LCD_PutStr(1, 0, "Not implemented");
                    UART0_Putstr("\r\nPrimit: ");
                    Negative_Response[0] = 0x7F;
                    Negative_Response[1] = (unsigned int)CAN_MsgRX.Date.Byte[0];
                    Negative_Response[2] = 0x11;
                    UART0_PutByteHex(Negative_Response[1]);				// afiseaza pe terminal octetul primit
                    CAN_MsgTX.Date.Byte[0] = (Negative_Response[1]);
                    
                    CAN_Transmit(1,0,&CAN_MsgTX);
                    
   
                }
		}
	}
    
    
}