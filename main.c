//comment
#include <pob-eye.h>


//move order for pob-proto board
#define MOVE_RUN		0x05
#define MOVE_BACK		0x0A
#define MOVE_LEFT		0x09
#define MOVE_RIGHT		0x06
#define STOP_BOT		0


//main program
int main(void)
{
	int i ;
	RGBFrame rgbFrame ;
	HSLFrame hslFrame ;

	int exitHouseLoop = 0 ;
	UInt8 NbForm ;
	Form ListOfForm[MAX_OF_FORM];

	int BallX = 0 ;
	int BallY = 0 ;

	InitPOBEYE();

	InitLCD();

	InitPobProto();
	SwitchOnAllServo();

	//init graphic buffer
	InitGraphicBuffer(&LCD_Buffer_Video,64,64,ONE_BIT,LCD_Buffer); 

	InitGraphicBuffer(&LCD_Fast_Buffer_Video,64,64,EIGHT_BITS,LCD_Fast_Buffer);
	ClearGraphicBuffer(&LCD_Fast_Buffer_Video);

	//init frame pointer
	GetPointerOnRGBFrame(&rgbFrame);

	GetPointerOnHSLFrame(&hslFrame);

	//init head
	HeadInit();
	HeadUp();

	
	
	
}
