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

    BOOLEAN HidD_GetPreparsedData(
  [in]  HANDLE               HidDeviceObject,
  [out] PHIDP_PREPARSED_DATA *PreparsedData
);


    if(HidD_GetInputReport(HidDeviceObject, 01, 15)){
        char data = HidP_GetData(HidP_Input, data, 15, *PreparsedData, 01, 15)
    printf(&data);
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
