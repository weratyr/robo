#include <pob-eye.h>
#include <pob-proto.h>


#include "grip_pattern.h" //Pattern file
#include "grip_bitmap.h"  //Bitmap file

#include "grip.h" //Servomotor grip order


//function for pob-proto and move bot
static void InitPobProto (void);
static void MoveAndStop(UInt8 Way,UInt32 time);
static void MoveBot(UInt8 Way);

//move order for pob-proto board
#define DISTANCE_SENSOR_FRONT 5
#define DISTANCE_SENSOR_RIGHT 2 
#define DISTANCE_SENSOR_LEFT  3

#define MOVE_RUN		0x05
#define MOVE_BACK		0x0A
#define MOVE_LEFT		0x09
#define MOVE_RIGHT		0x06
#define STOP_BOT		0


//array for display real time video
static UInt8         LCD_Fast_Buffer[64*64*BYTES];
static GraphicBuffer LCD_Fast_Buffer_Video;

static GraphicBuffer LCD_Fast_Buffer_Video1;

//array for display the graphic interface
static GraphicBuffer LCD_Buffer_Video;
static UInt8         LCD_Buffer[64*64*BITS];

//function for displat real time video
static void DrawVision(RGBFrame *ptr);
static void DrawVisionRight(RGBFrame *ptr);
static void HindernisAusweichen(int front, int left, int right);


int sensorFront ;
int sensorLeft ;
int sensorRight ;

//main program
int main(void)
{
    
	int i ;
	RGBFrame rgbFrame ;
	HSLFrame hslFrame ;
	
	int exitLoop = 0 ;
	UInt8 NbForm ;
	Form ListOfForm[MAX_OF_FORM];
	
	
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
	
	//init grip
	GripMiddle();
	GripOpen();
	GripDown();
	
	//set head to see ball
	HeadDownToBall();
	ClearGraphicBuffer(&LCD_Buffer_Video);
	ClearGraphicBuffer(&LCD_Buffer_Video);
	//house loop
	
	
	
    while( exitLoop == 0 )
	{
		GrabRGBFrame();
		BinaryRGBFrame(&rgbFrame);
		DrawVision(&rgbFrame);
		
		sensorFront = GetPortAnalog(DISTANCE_SENSOR_FRONT);
		sensorLeft = GetPortAnalog(DISTANCE_SENSOR_LEFT);
		sensorRight = GetPortAnalog(DISTANCE_SENSOR_RIGHT);
		
		//Secutrity function, the pod-pot cannot drive to anything (except the cup ;-) ) closer than about 8cm
		HindernisAusweichen(sensorFront, sensorLeft, sensorRight);
		
		//draw camera frame
		DrawVision(&rgbFrame);
		
		NbForm = IdentifyForm(&rgbFrame,ListOfForm,grip_pattern);								
		
		if( NbForm == 0 )
		{
			//looking after the 
			MoveAndStop(MOVE_LEFT,30000);
			MoveAndStop(MOVE_RUN,100000);
		}
		else 
		{ 
			exitLoop = GoToCup(ListOfForm, NbForm); 
		}
        
        PrintTextOnPobTerminal("Front Sensor %d, %d, %d", sensorFront, sensorLeft, sensorRight);
	    PrintTextOnPobTerminal("IdentifyForm value %d ", NbForm );
		
	}
	
	return 0;
}


/** Go to the cup function.
 *
 * @param formArray : pointer to a list of form in the frame
 * @param nbForm : form's number
 *
 * @return -1 if bot is just front of the cup, 0 else.
 *
 */
int GoToCup( Form *formArray, int nbForm)
{
	int i ;
	int MoveOrder = STOP_BOT ;
	int HeadPosition = 110 ;
	
	for(i=0 ; i<nbForm ; i++ )
	{
		if( formArray[i].id == IDP_BECHER )
		{
			
            PrintTextOnPobTerminal("form at w=%d h=%d", formArray[i].x ,formArray[i].y );
			//move bot to center the form
			if( formArray[i].x > 55 )
			{
				MoveAndStop(MOVE_RIGHT,40000);
			}
			else if( formArray[i].x < 35 )
			{
				MoveAndStop(MOVE_LEFT,40000);
			}
			else //go to cup with the bot
			{
				//becher sollte in der mitte des Bildes sein
				
				//fragt den Frontsensor ab
				//sensorFront = GetPortAnalog(DISTANCE_SENSOR_FRONT); 
				if (sensorFront > 90) {
					//stop move and open gri
					GripClose();
					Wait(200000);
					return -1 ;
				}
				else
				{
					//go fast to the house
					MoveOrder=MOVE_RUN;
				}
				
			}
			
		}
	}
	
	MoveBot(MoveOrder);
	
	return 0 ;
}



/**
 * Initialize the POB-PROTO board. PORTA set as analogic, PORTC as servomotor.
 */
void InitPobProto (void)
{
	PobProto board;
	
	
	//to get the position of the analogic joystick, you have to set the PORTA as analogic input
	board.porta=ALL_PORTA_AS_ANA;	
	
	//all pin of PORTC are configured to manage servomotors
	board.portc=RC7_AS_SERVO |RC6_AS_SERVO |RC3_AS_SERVO |RC2_AS_SERVO |RC1_AS_SERVO |RC0_AS_SERVO;
	
	
	//RD0 RD1 RD2 RD3 are configured as digitals output to gear DC motor
	//RD4 RD5 RD6 RD7 are configured as digitals input
	board.portd=RD7_AS_DI |RD6_AS_DI |RD5_AS_DI |RD4_AS_DI |RD3_AS_DO |RD2_AS_DO |RD1_AS_DO |RD0_AS_DO;
	
	//set the pob proto
	SetPobProto(&board);
	
}

/**
 * Move and stop bot.
 *
 * @param Way : move order
 * @param time : time to move
 */
void MoveAndStop( UInt8 Way, UInt32 time )
{
	MoveBot(Way);
	Wait(time);
	MoveBot(STOP_BOT);
	Wait(10000);
}

/**
 * Move the bot
 *
 * @param Way : move bot order
 */
void MoveBot (UInt8 Way)
{	
	SetPortD(Way);
}

/** 
 * Draw the frame binarised 
 *
 * @param ptr : pointer to a valid RGBFrame
 */
void DrawVision (RGBFrame *ptr)
{	
	//draw the red buffer (88*120 pixels) on the Buffer for for the left screen (64*64 pixels)
	DrawComponentInABufferVideo(ptr->red,&LCD_Fast_Buffer_Video);
	
	DrawLeftLCD(&LCD_Fast_Buffer_Video); //refresh the left screen
}


void DrawVisionRight(RGBFrame *ptr) 
{
	DrawComponentInABufferVideo(ptr->red,&LCD_Fast_Buffer_Video);
	DrawRightLCD(&LCD_Fast_Buffer_Video);
}


void HindernisAusweichen(int front, int left, int right)
{
    if (front > 85)
    {
        MoveAndStop(MOVE_BACK,30000);
        MoveAndStop(MOVE_LEFT,1000000);
    }
	
    if (left > 70)
    {
        MoveAndStop(MOVE_RIGHT,30000);
        MoveAndStop(MOVE_RUN,1000000);
    }
	
    if (right > 70)
    {
        MoveAndStop(MOVE_LEFT,30000);
        MoveAndStop(MOVE_RUN,1000000);
    }
	
}