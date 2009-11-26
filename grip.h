/*
 * GripBall Project.
 * 
 * Copyright 2006 POB-Technology. All rights reserved.
 * POB-Technology PROPRIETARY/CONFIDENTIAL.
 * 
 */
#ifndef GRIP_H
#define GRIP_H


/**
 * Servomotor parameter :
 *
 * change servomotor number if you connect your grip on the pob-bot
 *
 */

#define HEAD_SERVO            0    //servomotor number for head
#define GRIP_SERVO_OPEN_CLOSE 1    //servomotor number for open/close grip
#define GRIP_SERVO_UP_DOWN    2    //servomotor number for up/down grip


/**
 * Grip parameter : 
 *
 * you can change this value for your grip in the RgbAndServo program
 * see www.pob-technology.com/pobdev for download this program.
 *
 * GRIP_CLOSE_POSITION : the two component of the grip must be close
 *
 * GRIP_MIDDLE_POSITION : grip must be horizontal
 *
 * HEAD_INIT_POSITION : Head must be vertical
 *
 */

#define GRIP_CLOSE_POSITION   162   //servomotor grip close position

#define GRIP_MIDDLE_POSITION  50 //servomotor grip middle position

#define HEAD_INIT_POSITION    183 //vomotor head position



// warning, if you change value after this line, grip and head detection can failed !

/**
 * Grip open/close and catch ball
 */
#define GripOpen()	SetServoMotor(GRIP_SERVO_OPEN_CLOSE, GRIP_CLOSE_POSITION-60)
#define GripClose()	SetServoMotor(GRIP_SERVO_OPEN_CLOSE,GRIP_CLOSE_POSITION)
#define GripCatchBall()	 SetServoMotor(GRIP_SERVO_OPEN_CLOSE,GRIP_CLOSE_POSITION-10)

/** 
 * Grip up/down and middle position
 */
#define GripDown()	 SetServoMotor(GRIP_SERVO_UP_DOWN,GRIP_MIDDLE_POSITION-45)
#define GripMiddle() SetServoMotor(GRIP_SERVO_UP_DOWN,GRIP_MIDDLE_POSITION-20)
#define GripUp()     SetServoMotor(GRIP_SERVO_UP_DOWN,GRIP_MIDDLE_POSITION-20)
#define GripDrive()  SetServoMotor(GRIP_SERVO_UP_DOWN,GRIP_MIDDLE_POSITION+100)

/**
 * Head initial position, up and down
 */
#define HeadInit()       SetServoMotor(HEAD_SERVO,HEAD_INIT_POSITION)
#define HeadUp()         SetServoMotor(HEAD_SERVO,HEAD_INIT_POSITION)
#define HeadDownToBall() SetServoMotor(HEAD_SERVO,HEAD_INIT_POSITION-68)

#endif
