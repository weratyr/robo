#include <pob-eye.h>
#include <pob-proto.h>

#include "grip_pattern.h" //Pattern file
#include "grip_bitmap.h"  //Bitmap file
#include "grip.h" //Servomotor grip order

//move order for pob-proto board
#define DISTANCE_SENSOR_FRONT 5
#define DISTANCE_SENSOR_RIGHT 2
#define DISTANCE_SENSOR_LEFT  3

#define MAX_FONT_DISTANCE  65  
#define MAX_COUNTNOTFORM   15
#define MAX_COUNTNOTFORM_FOR 50

#define MOVE_RUN		0x05
#define MOVE_BACK		0x0A
#define MOVE_LEFT		0x09
#define MOVE_RIGHT		0x06
#define STOP_BOT		0

//function for pob-proto and move bot
static void InitPobProto (void);
static void MoveAndStop(UInt8 Way,UInt32 time);
static void MoveBot(UInt8 Way);

//function for displat real time video
static void DrawVision(RGBFrame *ptr);
static void DrawVisionRight(RGBFrame *ptr);
static int GoToX(Form *formArray, int nbForm);
static void RandomDrive();
static void HindernisAusweichen(int front, int formCup, int left, int right);
static int is_cup_griped();

//array for display real time video
static UInt8         LCD_Fast_Buffer[64*64*BYTES];
static GraphicBuffer LCD_Fast_Buffer_Video;
static GraphicBuffer LCD_Fast_Buffer_Video1;

//array for display the graphic interface
static GraphicBuffer LCD_Buffer_Video;
static UInt8         LCD_Buffer[64*64*BITS];

int sensorFront ;
int sensorLeft ;
int sensorRight ;
int countNotForm;
int IS_CUP_GRIPED;

//main program
int main(void)
{
	RGBFrame rgbFrame ;
	HSLFrame hslFrame ;

	int exitLoop = 0 ;
	UInt8 NbForm ;
	Form ListOfForm[MAX_OF_FORM] ;
	IS_CUP_GRIPED = 0;

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

	//GripMiddle();
	//GripDown();
	GripOpen();
	GripDrive();


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

		//draw camera frame
		DrawVision(&rgbFrame);

		NbForm = IdentifyForm(&rgbFrame,ListOfForm,grip_pattern);
		//Secutrity function, the pod-pot cannot drive to anything (except the cup ;-) ) closer than about 8cm


		if(NbForm == 0)
		{
			HindernisAusweichen(sensorFront, NbForm, sensorLeft, sensorRight);
			//MoveBot(MOVE_RUN);
			RandomDrive(); 

		}
		else
		{
			exitLoop = GoToX(ListOfForm, NbForm);
		}
        //PrintTextOnPobTerminal("Front Sensor %d, %d, %d", sensorFront, sensorLeft, sensorRight);
	    //PrintTextOnPobTerminal("IdentifyForm value %d ", NbForm );


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

 int GoToX( Form *formArray, int nbForm)
{
	int i ;
	int MoveOrder = STOP_BOT;


	for(i=0 ; i<nbForm ; i++ )
	{
            PrintTextOnPobTerminal("form id %d ", formArray[i].id );
		if( formArray[i].id == IDP_BECHER1 )
		{
            //PrintTextOnPobTerminal("form at w=%d h=%d", formArray[i].x ,formArray[i].y );
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
				MoveAndStop(MOVE_RUN,200000);
				//fragt den Frontsensor ab
				sensorFront = GetPortAnalog(DISTANCE_SENSOR_FRONT);
				if (sensorFront > 65) { // 65 successful value for the cup
					//stop move and open grip
					GripDown();
					MoveAndStop(MOVE_RUN,700000);
					GripClose();
					Wait(200000);
					GripUp();
					IS_CUP_GRIPED = is_cup_griped();
				}
				else
				{
					//go fast to the cup
					MoveOrder=MOVE_RUN;

				}

			}


		} else if( formArray[i].id == IDP_0_CROSS && IS_CUP_GRIPED == 1)
		{

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
				MoveAndStop(MOVE_RUN,200000);
				//fragt den Frontsensor ab
				sensorFront = GetPortAnalog(DISTANCE_SENSOR_FRONT);
				if (sensorFront > 65) // 65 successful value
				{ 

					MoveAndStop(MOVE_RUN,300000);
					GripDown();
					Wait(200000);
					GripOpen();
					IS_CUP_GRIPED = 0;
					MoveAndStop(MOVE_BACK,800000);
					// hier von dem Kreuz wegdrehen und wieder Grip hoch
					MoveAndStop(MOVE_LEFT,1000000);
					//HeadInit();
					GripDrive();
					HeadDownToBall();
				}
				else
				{
					//go fast to the cross
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

void RandomDrive()
{
	if(countNotForm > MAX_COUNTNOTFORM) 
	{
		int j = 0;
		for(j; j<MAX_COUNTNOTFORM_FOR; j++) 
		{
			MoveBot(MOVE_LEFT);
		}
		countNotForm = 0;
	}
	MoveBot(MOVE_RUN);
	countNotForm++;
}

void HindernisAusweichen(int front, int formCup, int left, int right)
{
	int i = 0;
    while (front > 50 && front < 90)
    {
		while(i < 3)
        {
			MoveBot(MOVE_BACK);
			MoveBot(MOVE_LEFT);
			i++;
		}
		front = GetPortAnalog(DISTANCE_SENSOR_FRONT);
    }

    while (left > 80 && left < 110)
    {
        //MoveBot(MOVE_RIGHT);
		while(i < 3)
        {
			MoveBot(MOVE_RIGHT);
			i++;
		}
		left = GetPortAnalog(DISTANCE_SENSOR_LEFT);
    }

    while (right > 80 && right < 110)
    {
		while(i < 3)
        {
			MoveBot(MOVE_LEFT);
			i++;
		}
		right = GetPortAnalog(DISTANCE_SENSOR_RIGHT);
    }
}
int is_cup_griped()
{	//Drehe dich aus möglichem Schatten
	//nimm den Kopf runter und frage abstandssensor ab, ob Becher gegriffen wurde
	int headPosition=115;
	int valueGripped=0;
	
	MoveAndStop(MOVE_LEFT, 400000);
	
	SetServoMotor(HEAD_SERVO,headPosition);
	valueGripped=GetPortAnalog(DISTANCE_SENSOR_FRONT);
	PrintTextOnPobTerminal("Fontsensor is_cup_griped() %d", valueGripped); 
	if (valueGripped>50)
	  {
		HeadUpToCross();
		return 1; 
	  }
	GripOpen();
	GripDrive();
	HeadDownToBall();
	return 0;
}