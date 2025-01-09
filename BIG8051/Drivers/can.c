#include <C8051F040.h>                 // SFR declarations
#include <osc.h>
#include <can.h>

//******************************************************************************
//Functii CAN
//******************************************************************************

CAN_Msg CAN_MsgTX, CAN_MsgRX; 

//Initializare CAN
	void CAN_Init(void) {
/* Calculaeza durata unui bit :
	
System clock        f_sys = 25 MHz
System clock 		t_sys = 1/f_sys = 40 ns.
Cuanta de timp CAN  tq = t_sys (la BRP = 0)

Rata de bit dorita este  1 MBit/s, durata dorita a unui bit este 1000 ns.
Durata unui bit = 25 tq = 1000 ns
Rata de bit este 1 MBit/s = rata de bit dorita

Lungimea magistralei CAN = 10 m, cu 5 ns/m intarziere.
Durata de propagare a semnalului: 2*(intarziere transceiver + intarziere propagare pe magistrala) = 400 ns
(intarzierea maxima între nodurile CAN)

Prop_Seg = 10 tq = 400 ns ( >= 400 ns).
Sync_Seg = 1 tq

Phase_seg1 + Phase_Seg2 = (24-10) tq = 14 tq
Phase_seg1 <= Phase_Seg2,  =>  Phase_seg1 = 6 tq and Phase_Seg2 = 8 tq
SJW = (min(Phase_Seg1, 4) tq = 4 tq

TSEG1 = (Prop_Seg + Phase_Seg1 - 1) = 15
TSEG2 = (Phase_Seg2 - 1)            = 7
SJW_p = (SJW - 1)                   = 3

Bit Timing Register = BRP + SJW_p * 0x0040 = TSEG1 * 0x0100 + TSEG2 * 0x1000 = 2640

Toleranta semnalului de tact df :

A: df < min(Phase_Seg1, Phase_Seg2) / (2 * (13*bit_time - Phase_Seg2))
B: df < SJW / (20 * bit_time)

A: df < 6/(2*(13*25-8)) = 6/(325-8) = 6/317 = 1,8927%
B: df < 4/(20*25)                   = 4/500 = 0.8%

*/
	unsigned int i;
	unsigned char SFRPAGE_save = SFRPAGE; 	// salveaza SFRPAGE
	SFRPAGE = CAN0_PAGE;

	CAN0CN  |= 0x41;       		// seteaza Test, CCE si INIT
	
	//CAN0TST = 0x18;				// Silent + Lback
	
	CAN0ADR  = BITREG;  			// selecteaza Bit Timing Register
	CAN0DAT  = 0x7FC1;     		// programeaza rata de bit (1 Mbit/sec) si configuratia unui bit

	CAN0ADR  = IF1CMDMSK;  		// Selecteaza Command Mask 1
	CAN0DATL  = 0xFF;					// Configureaza IF1 pentru scriere pentru toate campurile

	CAN0ADR  = IF1ARB1;  		// Selecteaza IF1ARB1
	CAN0DATL  = 0x00;
	CAN0DATL  = 0x00;				// pune MsgVal = 0, XTD = 0, Dir=0 in IF1ARB2
	
	for (i=1; i<33; i++){
      CAN0ADR = IF1CMDRQST; 	// dezactiveaza toate obiectele
      CAN0DATL = i;
	}

	CAN0ADR  = IF1CMDMSK;  	// Selecteaza Command Mask 1
	CAN0DATL  = 0x87;     	// Configureaza IF1 pentru scriere: 1000 0111: WRITE to CAN RAM, write data bytes,
													// set TXrqst/NewDat, clr IntPnd
	
	CAN0ADR  = IF2CMDMSK;  	// Selecteaza Command Mask 2
	CAN0DATL = 0x1F;       	// Configureaza IF2 pentru citire: 0001 1111: READ CAN RAM, read Control, data bytes,
													// clr NewDat and IntPnd
	
	CAN0CN  |= 0x02;       	// Valideaza intreruperile IE
	CAN0CN  &= ~0x41;      	// Sterge bitii CCE and INIT, s-a terminat initializarea, porneste controlerul CAN

	CAN_MsgTX.ID = CAN_MsgRX.ID = 0;				// initializeaza structurile folosite pentru transmisie si receptie
	CAN_MsgTX.DLC = CAN_MsgRX.DLC = 0;
	CAN_MsgTX.NewDat = CAN_MsgRX.NewDat = 0;
	for(i=0; i<8; i++){
		CAN_MsgTX.Date.Byte[i] = 0;
		CAN_MsgRX.Date.Byte[i] = 0;
   }
	EIE2 |= 0x20;				// valideaza intreruperile de la CAN0

	SFRPAGE = SFRPAGE_save;
}


//Initializeaza un Message Object
int CAN_Init_Object(char MsgObjNum, char CANID_Type, unsigned long CANID, char ObjDLC, char MsgObjDir)
{					 
	unsigned int val;
	unsigned char SFRPAGE_save = SFRPAGE; 	// salveaza SFRPAGE

	if(MsgObjNum > 32) return 0;
	if ((CANID_Type != CANID_XTD) && (CANID_Type != CANID_STD)) return 0; // CANID_TYpe necunoscut
	if ((CANID_Type == CANID_XTD) && (CANID > 0x1FFFFFFF)) return 0;		// CAN_ID prea mare pentru XTD
	if ((CANID_Type == CANID_STD) && (CANID > 0x07FF)) return 0;			// CAN_ID prea mare pentru STD

  SFRPAGE = CAN0_PAGE;

  CAN0ADR = IF1CMDMSK;  	// Selecteaza Command Mask 1
  CAN0DATL = 0xB0;     		// 1011 0000: Selecteaza scriere cu IF1 Arb, Control

  CAN0ADR = IF1ARB1;    	// Selecteaza Arb 1

	if (CANID_Type == CANID_XTD){
		CAN0DAT = (unsigned int) (CANID & 0x0000FFFF);			// incarca cei mai putin semnificativi 16 biti ai ID-ului extins
		val = (unsigned int)(((CANID >> 16 ) & 0x0000FFFF) | 0x0000C000); // pregateste cei mai semnificativi 13 biti si pune MsgVal si XTD pe 1
	}
	else {
		CAN0DAT = 0;										// ID-ul e standard, IF1ARB1 nu conteaza
		val = ((CANID << 2) & 0x1FFF) | 0x8000;	// pregateste ID-ul standard aliniat la stanga si pune MsgVal=1 si XTD=0 
	}
	if (MsgObjDir == MSGOBJ_TX)
			val |= 0x2000;			 	// adauga Dir = 1

	CAN0DAT = val;				// scrie in IF1ARB2

 	val = ObjDLC;						// pregateste pentru incarcare campul DLC
	if (MsgObjDir == MSGOBJ_RX)
		val |= 0x0480;					// RxIE=1, EOB=1 (mesaj individual), fara raspuns automat la cadre de cerere

  	CAN0DAT = val; 					// scrie in IF1MSGC

  	CAN0ADR = IF1CMDRQST; 	// Selecteaza Command Request reg.
  	CAN0DAT = MsgObjNum;    // -- declanseaza scrierea registrelor TF1 in Msg Obj
  	// 3-6 perioade ale CAN clock pentru a finaliza scrierea registrelor IF1 in Msg Obj.

  	SFRPAGE = SFRPAGE_save;
	return 1;
}


//Transmite un mesaj cu un obiect de iesire
void CAN_Transmit (char MsgObjNum, unsigned char NewID, CAN_Msg *ptr)
{
	int i;
	unsigned char SFRPAGE_save = SFRPAGE; 	// salveaza SFRPAGE
	SFRPAGE  = CAN0_PAGE;

	CAN0ADR  = IF1CMDMSK;  		// Selecteaza Command Mask 1
	if (NewID){						// Transmite cu un alt CAN ID decat a fost configurat initial?
		CAN0DAT  = 0x00B7;     	// Configureaza pentru TX: 1011 0111, comanda TxRqst, scrie 8 octeti
		CAN0ADR = IF1ARB2;		// Selecteaza direct IF1ARB2 (doar mesaje STD)
		CAN0DAT = (((ptr->ID) << 2) & 0x1FFF) | 0xA000; //  incarca CAN ID aliniat la stanga si pune MsgVal=Dir=1, XTD=0
		CAN0DAT = (unsigned int) (ptr->DLC);	// incarca DLC 
	}
	else
		CAN0DAT  = 0x0087;     	// Configureaza pentru TX: 1000 0111, comanda TxRqst, scrie 8 octeti, pastreaza Arb si Control

	CAN0ADR  = IF1DATA1;   		// Selecteaza primul buffer de date

	for (i = 0; i<8; i+=2){		// Scrie 8 octeti, incepand cu LSB
		CAN0DATH = ptr->Date.Byte[i+1]; 	
		CAN0DATL = ptr->Date.Byte[i];
	}
	CAN0ADR  = IF1CMDRQST; 		// Selecteaza Command Request Reg.
	CAN0DATL = MsgObjNum;   	// Transfera registrele IF1 in Msg Obj

	SFRPAGE  = SFRPAGE_save;
}

// Preia un mesaj receptionat intr-un obiect de intrare
void CAN_Receive (char MsgObjNum, CAN_Msg *ptr)
{
	int i;
	unsigned char SFRPAGE_save = SFRPAGE; 	// salveaza SFRPAGE
	SFRPAGE  = CAN0_PAGE;

	// RX-IF2 operation may interrupt TX-IF1 operation
	CAN0ADR  = IF2CMDMSK;  	// Selecteaza Command Mask 2
	CAN0DATL = 0x3F;       	// Configureaza IF2 pentru citire: 0011 1111: READ CAN RAM, read Arb, Control, data bytes,
													// clr NewDat and IntPnd

	CAN0ADR  = IF2CMDRQST;	// IF2 deja configurat pentru citire date
	CAN0DATL = MsgObjNum;   // Transfera din Msg Obj "MsgObjNum" in IF2

	for(i=0;i<2;i++);			// asteapta finalizarea transferului

 	CAN0ADR  = IF2ARB2;  	// Selecteaza adresa Arb 2
	
	ptr->ID = (CAN0DAT >> 2) & 0x07FF;		// Preia CAN ID STD
	ptr->DLC = (unsigned char) (CAN0DAT & 0x000F);	// Preia DLC

	for (i = 0; i<8; i+=2){			// Citeste 8 octeti, incepand cu LSB
		ptr->Date.Byte[i+1] = CAN0DATH;	
		ptr->Date.Byte[i] = CAN0DATL;
	}
	ptr->NewDat = 1;					// semnaleaza adate noi
	SFRPAGE  = SFRPAGE_save;
}

////////////////////////////////////////////////////////////////////////////////
//Functia de intrerupere CAN
////////////////////////////////////////////////////////////////////////////////

void CAN_Isr (void) interrupt 19
{
	unsigned int IntID;
	do{
		CAN0ADR = INTREG;			// Selecteaza INTREG
		IntID = CAN0DAT;			// citeste INTREG - codul intreruperii de prioritate maxima
		switch (IntID){	
			case 0x8000:  			// intrerupere de stare, daca e activata de SIE in CAN0CN
				IntID = CAN0STA;	// achita intreruperea prin citirea registrului de stare
				break;		
			case 2:  				// intrerupere cauzata de obiectul 2 (de receptie)
				CAN_Receive(2,&CAN_MsgRX);		// preia mesajul primit
				break;	
			default: break;
		}
	}
   while(IntID);					// reia cat timp mai sunt si alte solicitari de intrerupre
}
