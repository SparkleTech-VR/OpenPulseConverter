#include<iostream>
#include<hidusage.h>
#include<hidclass.h>
#include<hidsdi.h>
#include<hidpi.h>
#include<cstdio>
#include<cstdlib>
#include<Windows.h>
#include<wtypes.h>





int main(){

    MessageBox(NULL, "This is a developer tool for other developers, by using this you accept any and all liability to hardware or psyche. You have been warned", "DEVELOPERS TOOL ONLY", MB_OK);

    char HidDeviceObject = "1915:EEE0" // Right Glove

    BOOLEAN HidD_GetPreparsedData(  // Not sure if this is working, needs preparsed data for getData function
  [in]  HANDLE               HidDeviceObject,
  [out] PHIDP_PREPARSED_DATA *PreparsedData
);
    printf("Please check your Right Glove is running data into this prompt \n");

    if(!HidD_GetInputReport(HidDeviceObject, 01, 15)){ printf("ERROR NO GLOVE DETECTED, RESTART PROGRAM! \n"); return 1};


    while(HidD_GetInputReport(HidDeviceObject, 01, 15)){ //If glove report exists lets see it
        char data = HidP_GetData(HidP_Input, data, 15, &PreparsedData, 01, 15)
    printf(&data); // sent to command prompt
    delay(14.97) //Delay of 14.97 ms to achieve 67Hz
    };
    system("pause");





    printf("Please start SteamVR with OpenGloves running \n");
    system("pause");

    //NamedPipe stuff here

    // "\\.\pipe\vrapplication\input\glove\v1\<left/right>"
// struct InputData {
//   const std::array<std::array<float, 4>, 5> flexion;
//   const std::array<float, 5> splay;
//   const float joyX;
//   const float joyY;
//   const bool joyButton;
//   const bool trgButton;
//   const bool aButton;
//   const bool bButton;
//   const bool grab;
//   const bool pinch;
//   const bool menu;
//   const bool calibrate;
// };

// "\\.\pipe\vrapplication\input\glove\v2\<left/right>"
struct InputData {  //We will only be using the Flexion and Splay everything else will be emulated by OpenGloves or within the UI for OG
  const std::array<std::array<float, 4>, 5> flexion;
  const std::array<float, 5> splay;
  const float joyX;
  const float joyY;
  const bool joyButton;
  const bool trgButton;
  const bool aButton;
  const bool bButton;
  const bool grab;
  const bool pinch;
  const bool menu;
  const bool calibrate;

  const float trgValue;
};




    return 0;
};


//FFB math theory

// So A-E comes as 0-1000
// divided /10 to get
// Percent%, round down for whole number
// times x 2.55 for range
// subtract 127 to adjust Byte 1 for endpoint 2
// subtract 64 to adjust Byte 0 for endpoint 1
// Encapsulate a 10 ms delay for bytes 2&3 to emulate duration
// Byte2 is H x 255
// Byte3 is F x 255


//FFB LUCID INFO

// "A" - Thumb force feedback. Integer from 0-1000
// "B" - Index force feedback. Integer from 0-1000
// "C" - Middle force feedback. Integer from 0-1000
// "D" - Ring force feedback. Integer from 0-1000
// "E" - Pinky force feedback. Integer from 0-1000
// "F" - Frequency of haptic vibration. Decimal
// "G" - Duration of haptic vibration. Decimal
// "H" - Amplitude of haptic vibration. Decimal