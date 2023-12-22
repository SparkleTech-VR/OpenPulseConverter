#include<iostream>
#include<hidusage.h>
#include<hidclass.h>
#include<hidsdi.h>
#include<hidpi.h>
#include<cstdio>
#include<cstdlib>
#include<Windows.h>
#include<wtypes.h>
#include<array>
#include<ctime>




int main(){


  //Opening warning Message window to Alert users of Experimental Code
  char* WarningMB = "This is a developer tool for other developers by using this you accept any and all liability to hardware or psyche You have been warned";
   char* title  =  "DEVELOPERS TOOL ONLY";

  LPCWSTR stringToWide(char* strarg) //Might have written this function wrong
  {

    const char* strarg  //This is an AI generated block If you can make it simpler please do so
int bufferSize = MultiByteToWideChar(CP_UTF8, 0, strarg, -1, nullptr, 0);
wchar_t* wideArgument = new wchar_t[bufferSize];
MultiByteToWideChar(CP_UTF8, 0, strarg, -1, wideArgument, bufferSize);
// Now you can pass wideArgument as an LPCWSTR to the function or method

      return wideArgument
  };
     LPCWSTR WideTitle = stringToWide(title);
     LPCWSTR WideWarning = stringToWide(WarningMB);

    MessageBox(NULL, WideWarning, WideTitle, MB_OK);

        //-----------------------Functions for gloves

        void delay(double milliseconds) {  //Some reason VScode wants a semicolon here
    clock_t start_time = clock(); //grab local clock
    while (clock() < start_time + milliseconds); //wait while local clock ticks
    
};

    void PrintCapabilities(const PHIDP_CAPS& Caps){
    std::cout << "Usage;" << Caps.Usage << std::endl;
    std::cout << "UsagePage;" << Caps.UsagePage << std::endl;
    std::cout << "InputReportByteLength;" << Caps.InputReportByteLength << std::endl;
    std::cout << "OutputReportByteLength;" << Caps.OutputReportByteLength << std::endl;
    std::cout << "FeatureReportByteLength;" << Caps.FeatureReportByteLength << std::endl;
    std::cout << "Reserved[17]; internal HID use" << Caps.Reserved << std::endl;

    std::cout << "NumberLinkCollectionNodes;" << Caps.NumberLinkCollectionNodes << std::endl;

    std::cout << "NumberInputButtonCaps;" << Caps.NumberInputButtonCaps << std::endl;
    std::cout << "NumberInputValueCaps;" << Caps.NumberInputValueCaps << std::endl;
    std::cout << "NumberInputDataIndices;" << Caps.NumberInputDataIndices << std::endl;

    std::cout << "NumberOutputButtonCaps;" << Caps.NumberOutputButtonCaps << std::endl;
    std::cout << "NumberOutputValueCaps;" << Caps.NumberOutputValueCaps << std::endl;
    std::cout << "NumberOutputDataIndices;" << Caps.NumberOutputDataIndices << std::endl;

    std::cout << "NumberFeatureButtonCaps;" << Caps.NumberFeatureButtonCaps << std::endl;
    std::cout << "NumberFeatureValueCaps;" << Caps.NumberFeatureValueCaps << std::endl;
    std::cout << "NumberFeatureDataIndices;" << Caps.NumberFeatureDataIndices << std::endl;
    };


    HANDLE HidDeviceObject = "1915:EEE0" ;// Right Glove
  PHIDP_PREPARSED_DATA PreparsedData;
  PHIDP_CAPS           Capabilities;
      void InitializeGlove(){
        HidD_GetPreparsedData(HidDeviceObject, &PreparsedData); // Not sure if this is working, needs preparsed data for getData function
      HidP_GetCaps(PreparsedData, Capabilities);
    return PrintCapabilities(Capabilities);
      };
      
    InitializeGlove();

    


  printf("Please check your Right Glove is running data into this prompt \n");
      system("pause");


    if(!HidD_GetInputReport(HidDeviceObject, 01, 15)){ 
      printf("ERROR NO GLOVE DETECTED: Attempting to connect, if failing please RESTART PROGRAM! \n"); 
      InitializeGlove(); 
      if(!HidD_GetInputReport(HidDeviceObject, 01, 15)){ return 1;}
      };
    

    while(HidD_GetInputReport(HidDeviceObject, 01, 15)){ //If glove report exists lets see it
        PHIDP_DATA DataList ;
        HidP_GetData(HidP_Input,  DataList, 15, PreparsedData, 01, 15);
        char data = 
        printf("this is RAW BYTES \n");
    printf(&data); // sent to command prompt----Change this to std::cout and run data inline
    delay(14.97); //Delay of 14.97 ms to achieve 67Hz 
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