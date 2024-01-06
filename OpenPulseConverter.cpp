



// Including code contributions by Jagrosh, https://github.com/jagrosh/Pulse2OpenGloves/blob/master/src/main.cpp Thanks mate




#include <iostream>
#include <array>
#include <thread>
#include <chrono>
#include<cstdio>
// https://github.com/libusb/hidapi
#include <hidapi.h>
#include <windows.h> 
#include <cstring>









        //-----------------------Functions for gloves
#define TOP_RANGE 64 //This is the top of the spring stop lowered in relation to the OpG data
#define BOTTOM_RANGE 127 //Bottom of the spring
#define LOG(x) std::cout << "[" << __FILE__ << " Line" << __LINE__ << "] " << x << std::endl;
#define DISPLAY(x) std::cout << "OpenPulse Converter:"<< x << std::endl;
#define MAX_STR 255
#define debugPause system("pause")
#define CR ;printf("\r")
const int VENDOR_ID = 0x1915;
const int RIGHT_GLOVE_PRODUCT_ID = 0xEEE0;
const int LEFT_GLOVE_PRODUCT_ID = 0xEEE1;
#define RIGHT_PIPE  "\\\\.\\pipe\\vrapplication\\input\\glove\\v2\\right"
#define LEFT_PIPE   "\\\\.\\pipe\\vrapplication\\input\\glove\\v2\\left"

// OpenGlove data structures
typedef struct OpenGloveInputData
{
    std::array<std::array<float, 4>, 5> flexion;
    std::array<float, 5> splay; // We are only using the Flex and Splay input with Pull and Splay respectively, OpG will emulate or have UI for all else
    float joyX;
    float joyY;
    bool joyButton;
    bool trgButton;
    bool aButton;
    bool bButton;
    bool grab;
    bool pinch;
    bool menu;
    bool calibrate;
    float trgValue;
} OpenGloveInputData;

// Pulse data structures
#pragma pack(push, 1)
typedef struct FingerData
{
    //Grab data as native unsigned char from Pulse glove report buffer
    unsigned char data[3]{};

} FingerData;

typedef struct GloveInputReport
{
    unsigned char reportId : 8;
    FingerData thumb, index, middle, ring, pinky;
} GloveInputReport;

union HIDBuffer
{
    GloveInputReport glove;
    unsigned char buffer[sizeof(glove)];
};

typedef struct OutputStructure { //FFB output struct
    int A;
    int B;
    int C;
    int D;
    int E;
    float F;
    float G;
    float H;
} OutputStructure;

typedef struct finT {
    //Paired Data for Return needs
    unsigned int pull;
    unsigned int splay;
}finT;

#pragma pack(pop)

class whatIsGlove //baby, Don't hurt me, don't hurt me; no mo'
{

public:
    whatIsGlove( // Derp, this is a Constructor
        int vid, int pid) : m_wstring{}, m_handle{ hid_open(vid, pid, nullptr) },OgInput{}, m_ogPipe{}, r_buffer{} {}
    //virtual ~whatIsGlove() { hid_close(m_handle); }; //CAREFUL!!! THIS LEADS TO CRASH BEHAVIOR, I know destructors are proper code, however here we are cleaning with hid_exit

    // true if the glove is connected
    const bool isValid() const { return m_handle; }

    //Glove Functions
   
     const auto& read() {

         if (m_handle != INVALID_HANDLE_VALUE) {
             auto result = hid_read(m_handle, r_buffer.buffer, 16);
             std::cout << "Handle being Read:" << &m_handle << std::endl;
             if (result == -1) {
                 const auto error = hid_error(m_handle);
                 std::cout << "Error while reading HID data: " << error << "Handle with bad Read:" << m_handle << std::endl;

             }
             

             return r_buffer;
         }
     };
    const void write(unsigned char* HapticData) { hid_write(m_handle, HapticData, 21); };

    //Run the pipe open with a for loop internally
   
    void openPipe(const std::string& pipeName) {

        for (int i = 0; i < 10; i++) {

            m_ogPipe = CreateFileA(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            
            if (m_ogPipe != INVALID_HANDLE_VALUE) {
                DWORD mode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
                BOOL success = SetNamedPipeHandleState(m_ogPipe, &mode, NULL, NULL);
                if (success) {
                    std::cout << "Named pipe created successfully." << std::endl;
                    break;
                }
                else {
                    std::cout << "Failed to set named pipe handle state." << std::endl;
                }
            };

            if (i == 9) { printf("Pipe about to timeout\n"); std::cout << "Named pipe created Attempt Ended." << std::endl; break; }

            if (GetLastError() != ERROR_PIPE_BUSY) {
                std::cout << "bad error" << std::endl;
            }

            if (!WaitNamedPipeA(pipeName.c_str(), 1000)) {
                std::cout << "timed out waiting for pipe" << std::endl;
            }

            std::cout << "Failed to setup named pipe.. Waiting 1 second..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            i++;
        }
        
    };

    //OpenGlovesDriver Functions
    const auto& Feel() { DWORD dwRead; bool returnCheck = ReadFile(m_ogPipe, reinterpret_cast<LPVOID>(&OgInput), sizeof(OutputStructure), &dwRead, NULL); if (returnCheck) { return OgInput; } else { OutputStructure trashzero{}; return trashzero; } };
    template <typename T>
    const bool Touch(const T& TrackingData) { DWORD dwWritten{}; return WriteFile(m_ogPipe, (LPCVOID)&TrackingData, sizeof(TrackingData), &dwWritten, NULL); };
    const bool pipeIsValid() { return m_ogPipe; };

    void closePipe() {
        CloseHandle(m_ogPipe);
    }

    //Data Functions cause it's neater to shove them here
    const float isCurled(int finData) { float sentFloat = ((float)finData / 16383.0f); return sentFloat; };
    const finT BitData(const unsigned char data[3]) { //Took a big bong rip and figured out what I need to do

        data0 = data[0];//convert the incoming bytes into bitsets
        data1 = data[1];
        data1Pull = data1 >> 2;//split data 1 into the pull and splay
        data1Splay = data1 & 0b0000000000000011; // this is the mask for int, could also use 0x03 which is the same, Thanks again JagRosh for pointing this out(bitsets are overkill)
        data2 = data[2];

        //We will copy data into the correct form again and use Binary OR to get them combined
        unsigned int  pullLow = (data1Pull);
        unsigned int  pullFar = (data1 << 6);
        unsigned int splayLow = (data2);
        unsigned int splayFar = (data1Splay << 8);

        // Extracting the real numbers via the Binary OR function
        pullBits = (pullLow | pullFar);
        splayBits = (splayLow | splayFar);

        finT ParsedData;
        ParsedData.pull = pullBits;
        ParsedData.splay = splayBits;

        return ParsedData;
    }

    // device info
    const std::string getManufacturer() { hid_get_manufacturer_string(m_handle, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
    const std::string getProduct() { hid_get_product_string(m_handle, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
    const std::string getSerialNumber() { hid_get_serial_number_string(m_handle, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
    const std::string getIndexedString(const int i) { hid_get_indexed_string(m_handle, i, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }

private:
    // connection to glove
    hid_device* m_handle;
    
    //Init the finger Bytes -- not a snack 
    unsigned int data0{};
    unsigned int data1{};
    unsigned int data1Pull{};
    unsigned int data1Splay{};
    unsigned int data2{};
    unsigned int pullBits{};
    unsigned int splayBits{};

    // temp vars
    wchar_t m_wstring[MAX_STR];
    HIDBuffer r_buffer;
    HANDLE m_ogPipe;
    OutputStructure OgInput{};
};
// Init Splay and Flex ; Init Tracking and Haptic Data
std::array<float, 5> splay;
std::array<std::array<float, 4>, 5> flexion;
unsigned char* HapticData;
OpenGloveInputData ogid{};
// make all the variables for our data to get held in
const int HapticConvert(int input) { int output = input / 10 * 2.55; return output; }
unsigned char report[21]; // Output Report Variable HID api 


void Tracking(whatIsGlove glove) {

    //------Tracking
   auto& buffer = glove.read();

    // run the buffer to bit convert our data into the data struct
    finT thumbTracking = glove.BitData(buffer.glove.thumb.data);
    finT indexTracking = glove.BitData(buffer.glove.index.data);
    finT middleTracking = glove.BitData(buffer.glove.middle.data);
    finT ringTracking = glove.BitData(buffer.glove.ring.data);
    finT pinkyTracking = glove.BitData(buffer.glove.pinky.data);


    printf("buffer: ");
    for (int i = 0; i < sizeof(buffer); i++)
        printf("%d ", buffer.buffer[i]);
    printf("\n");


    //Pulls from the paired Data
    unsigned int thumbPull = thumbTracking.pull;
    unsigned int indexPull = indexTracking.pull;
    unsigned int middlePull = middleTracking.pull;
    unsigned int ringPull = ringTracking.pull;
    unsigned int pinkyPull = pinkyTracking.pull;

    //Splays from the Paired Data
    // 
    unsigned int thumbSplay = thumbTracking.splay;
    unsigned int indexSplay = indexTracking.splay;
    unsigned int middleSplay = middleTracking.splay;
    unsigned int ringSplay = ringTracking.splay;
    unsigned int pinkySplay = pinkyTracking.splay;

    //test code to confirm we are getting the data we want

    std::cout << "Pull: " << indexPull; printf("\n");
    std::cout << "Splay: " << indexSplay; // CR CR;


    //The data structs for our finger buffer data

    // Create std::array for splay_buffer
    const std::array<float, 5> splay_buffer = {
        (float)thumbSplay,
        (float)indexSplay,
        (float)middleSplay,
        (float)ringSplay,
        (float)pinkySplay
    };


    // Create std::array for pull_buffer
    std::array < std::array < float, 4 >, 5 > pull_buffer{};
    for (int t =0; t < 3; t++ ) {
        pull_buffer[0][t] = {glove.isCurled(thumbPull)};
    }
    for (int i = 0; i < 4; i++) {
        pull_buffer[1][i] = {glove.isCurled(indexPull)};
    }
    for (int m = 0; m < 4; m++) {
        pull_buffer[2][m] = {glove.isCurled(middlePull)};
    }
    for (int r = 0; r < 4; r++) {
        pull_buffer[3][r] = {glove.isCurled(ringPull)};
    }
    for (int p = 0; p < 4; p++) {
        pull_buffer[4][p] = {glove.isCurled(pinkyPull)};
    }
    
    
    splay = splay_buffer; //Semantics for readability -- no impact on performance
    flexion = pull_buffer;


    //Write your Input data to ogid
    ogid.flexion = flexion;
    ogid.splay = splay;
    //buttons to be emulated, trgValue, grab, menu; maybe pinch, joyx,joyY, maybe others as I care in games
   // ogid.trgValue = indexPull; //example code for rest of buttons
    ogid.grab = (ringPull > 11500);//Possible Grab
    glove.Touch(ogid);
    LOG("wrote");
    if (GetLastError()) {
        DWORD errorCode = GetLastError();
        std::cout << "bad error:__" << errorCode << std::endl;
        debugPause;
    };
}

void Haptics(OutputStructure ogod, whatIsGlove glove) {


    //------FFB
    const int f_buffer = ogod.B;//Oh Flying Spaghetti Monster, Bless this variable *Praise Be to his noodly goodness*

    //Check if we are receiving force

    printf("%d \n", f_buffer);

    // Step 1: Extract the values from the  incoming OutputStructure and assign them to the corresponding fields of the output structure

    OutputStructure outputData = ogod;

    // Step 2: Access the fields of the structure
    int thumbForceFeedback = outputData.A;
    int indexForceFeedback = outputData.B;
    int middleForceFeedback = outputData.C;
    int ringForceFeedback = outputData.D;
    int pinkyForceFeedback = outputData.E;
    float frequency = outputData.F;
    float duration = outputData.G;//Not used by Pulse ; TODO: thought of a way we can use this by running a sleepfor on this timing
    float amplitude = outputData.H;

    // Now you have the output structure with the extracted values


    //Let's manipulate them with math to get data points the glove understands

    // Bytes 2 and 3 are easy math
    int convertedFreq = frequency * 255; //Byte 3
    int convertedAmp = amplitude * 255; // Byte 2

    // Now its for the rest of the fingers endpoints to be estimated by how far the OpG finger is allowed 
    // OpG uses 1000 as max value to hold users' fingers straight out, value 0 is unrestricted tracking

    // Using the function in the glove class HapticConvert to change our 2 bytes from OpG into 1 byte for Pulse
    // Then we manipulate that to estimate 2 end points of a spring at where the users' fingers should stop

    int convertedThumb = HapticConvert(thumbForceFeedback);
    int convertedIndex = HapticConvert(indexForceFeedback);
    int convertedMiddle = HapticConvert(middleForceFeedback);
    int convertedRing = HapticConvert(ringForceFeedback);
    int convertedPinky = HapticConvert(pinkyForceFeedback);

    //We are going to use a naming convention to align our bytes easier

    int thumb0 = convertedThumb - TOP_RANGE; //byte 0 uses a magic number to consistently give a quarter spring tension, more will increase tension, less will be springier; default: 64
    int thumb1 = convertedThumb - BOTTOM_RANGE; // Byte 1 uses the half range to adjust down along the force of OpG; default: 127
    int index0 = convertedIndex - TOP_RANGE;
    int index1 = convertedIndex - BOTTOM_RANGE;
    int middle0 = convertedMiddle - TOP_RANGE;
    int middle1 = convertedMiddle - BOTTOM_RANGE;
    int ring0 = convertedRing - TOP_RANGE;
    int ring1 = convertedRing - BOTTOM_RANGE;
    int pinky0 = convertedPinky - TOP_RANGE;
    int pinky1 = convertedPinky - BOTTOM_RANGE;

    // Output Report Creator

    report[0] = 0x02;// First Byte needs to be 02 for the Pulse to read it

    //Convert and store the data into the rest of the report

    // Thumb force feedback (A)
    report[1] = thumb0 & 0xFF;
    report[2] = thumb1 & 0xFF;
    report[3] = convertedAmp & 0xFF;
    report[4] = convertedFreq & 0xFF;
    // Index force feedback (B)
    report[5] = index0 & 0xFF;
    report[6] = index1 & 0xFF;
    report[7] = convertedAmp & 0xFF;
    report[8] = convertedFreq & 0xFF;

    // Middle force feedback (C)
    report[9] = middle0 & 0xFF;
    report[10] = middle1 & 0xFF;
    report[11] = convertedAmp & 0xFF;
    report[12] = convertedFreq & 0xFF;
    // Ring force feedback (D)
    report[13] = ring0 & 0xFF;
    report[14] = ring1 & 0xFF;
    report[15] = convertedAmp & 0xFF;
    report[16] = convertedFreq & 0xFF;
    // Pinky force feedback (E)
    report[17] = pinky0 & 0xFF;
    report[18] = pinky1 & 0xFF;
    report[19] = convertedAmp & 0xFF;
    report[20] = convertedFreq & 0xFF;

    printf("Force:");


    HapticData = report;// Just semantics for readability, compiler will ignore this -- no impact on performance

    printf("%p", HapticData);

    CR;


    glove.write(HapticData);//Feel the VR beneath your finger tips!


    //TODO: Write a sleepfor on the duration G variable followed by a write call with a second output report as above but with 0's for the vibrations

    std::this_thread::sleep_for(std::chrono::milliseconds((int)duration * 1000));

    //Now to reset our default tracking as described in pulse basics-- https://www.bifrostvr.com/pulse/basics 
    // THIS IS IMPORTANT --- By default endpoint 1 is set to 25 and endpoint 2 is set to 0
    // Output Report Creator

    report[0] = 0x02;// First Byte needs to be 02 for the Pulse to read it

    //Convert and store the data into the rest of the report

    // Thumb force feedback (A)
    report[1] = 25 & 0xFF;
    report[2] = 0 & 0xFF;
    report[3] = 0 & 0xFF;
    report[4] = 0  & 0xFF;
    // Index force feedback (B)
    report[5] = 25 & 0xFF;
    report[6] = 0  & 0xFF;
    report[7] = 0 & 0xFF;
    report[8] = 0 & 0xFF;

    // Middle force feedback (C)
    report[9] = 25 & 0xFF;
    report[10] = 0  & 0xFF;
    report[11] = 0 & 0xFF;
    report[12] = 0 & 0xFF;
    // Ring force feedback (D)
    report[13] = 25 & 0xFF;
    report[14] = 0  & 0xFF;
    report[15] = 0 & 0xFF;
    report[16] = 0 & 0xFF;
    // Pinky force feedback (E)
    report[17] = 25 & 0xFF;
    report[18] = 0  & 0xFF;
    report[19] = 0 & 0xFF;
    report[20] = 0  & 0xFF;

    printf("Force:");


    HapticData = report;// Just semantics for readability, compiler will ignore this -- no impact on performance

    printf("%p", HapticData);

    CR;


    glove.write(HapticData);//GLOVE IS RESET FOR TRACKING

};

/*
 * Main function that runs upon execution
 */
int main(int argc, char** argv)
{

    //Opening warning Message window to Alert users of Experimental Code
    LPCWSTR WarningMB = L"This is a developer tool for other developers by using this you accept any and all liability to hardware, software, or psyche. You have been warned!";
    LPCWSTR title = L"DEVELOPERS TOOL ONLY";

    MessageBox(NULL, WarningMB, title, MB_OK);


    printf("Hello World! \n This is the OpenPulse Converter. \n A simple tool to send tracking and haptic data between Bifrost Pulse Gloves and OpenGloves Driver. \n This is a community development from the Pulse Discord. \n Please thank Jagrosh, KingOfDranovis, and Sheridan in the discord when you see them. \n Thank you for using this tool, you are part of an Awesome Club! \n PLEASE TURN ON YOUR GLOVES! \n");
    system("pause");
    // initialize HID lib
    hid_init();

    // init gloves
    whatIsGlove left{ VENDOR_ID, LEFT_GLOVE_PRODUCT_ID };
    whatIsGlove right{ VENDOR_ID, RIGHT_GLOVE_PRODUCT_ID };

    // print diagnostics
    if (!left.isValid() && !right.isValid())
    {
        printf("No gloves are connected!");
        hid_exit();
        system("pause");
        return 1;
    }

    if (left.isValid())
    {
        DISPLAY("Left Glove Connected!");
        DISPLAY("Manufacturer: " + left.getManufacturer());
        DISPLAY("Product     : " + left.getProduct());
        DISPLAY("SerialNumber: " + left.getSerialNumber());
    }
    else
    {
        printf("Left Glove was not found!");
    }

    if (right.isValid())
    {
        DISPLAY("Right Glove Connected!");
        DISPLAY("Manufacturer: " + right.getManufacturer());
        DISPLAY("Product     : " + right.getProduct());
        DISPLAY("SerialNumber: " + right.getSerialNumber());
    }
    else
    {
        printf("Right Glove was not found!");
    }


    //Init Pipe and Opengloves connection
    printf("Attempting connection to OpenGloves Driver, please start SteamVR with OpG running, then continue with this plugin.\n");
    system("pause");

    //Init Right Pipe
    if (right.isValid()) {
        right.openPipe(RIGHT_PIPE);
    }

    //Init Left Pipe
    if (left.isValid()) {
        left.openPipe(LEFT_PIPE);
    }

    //Init our writable blocks
    OutputStructure ogodR{};
    OutputStructure ogodL{};

    printf("OpenPulse Primed for game pipes, please begin game boot flow and Good Luck!\n");
    system("pause");

    //Below this line is only for runtime code; all startup code should be above this line-----------------------------------------------------



    // begin loop to run everything at 67hz
    //bool quit = false; // could be used for making a better exit experience
    while (true)
    {
        if (left.isValid())
        {

            //Tracking---------------

            Tracking(left);

            if (left.pipeIsValid()) { //

                // Force Feedback Haptics----------------------

                ogodL = left.Feel();
                if (&ogodL) {
                    Haptics(ogodL, left);
                };

            }
        }
        if (right.isValid())
        {

            //Tracking---------------

            Tracking(right);

            if (right.pipeIsValid()) { //
                
                

                // Force Feedback Haptics----------------------

                ogodR = right.Feel();
                if (&ogodR) {
                    Haptics(ogodR, right);
                };

            }
           

        };
        //--------When writing FFB Output Reports from the open pipe reading from Opengloves driver, Outputs are only triggered after sending input to the driver.
        
         //Functions after the glove Data-------
        std::this_thread::sleep_for(std::chrono::microseconds(1000000 / 67)); // 67 hz  <-- This is really cool
        
    }
        //Clean Up Data
        if (left.isValid()) { left.closePipe(); }
        if (right.isValid()) { right.closePipe(); }

        // close the hidapi library
        return hid_exit();
}



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


//Cool but deprecated code
// 
//   // Step 1: Create an LPCvoid pointer variable
    //TrackingData; Done above;
// 
//        // Step 2: Allocate memory for the LPCvoid pointer
//    TrackingData = malloc(sizeof(OpenGloveInputData));
// 
//    // Step 3: Copy the data from the original structure to the allocated memory
//    memcpy((void*)TrackingData, &ogid, sizeof(OpenGloveInputData));
// 
//    // Step 4: Cast the allocated memory to LPCvoid
//    LPCVOID convertedData = (LPCVOID)TrackingData;
// 
//    glove.Touch(convertedData); // TODO: write ogid to TrackingData --- DONE
// 
//    free((void*)TrackingData);// Dump the memory so we don't leak into the ram
//
// 
// This was some test code to try resetting the handle if that became invalid---Test Failed
//      //  if (m_handle == INVALID_HANDLE_VALUE) { printf("Attempting right glove reconnection with HID Handle; Please wait upto 0.1 seconds!\n");  r_buffer = {}; 
//      while (m_handle == INVALID_HANDLE_VALUE) {
//          m_handle = reOpen(VENDOR_ID, RIGHT_GLOVE_PRODUCT_ID);
//          if (m_handle != INVALID_HANDLE_VALUE) break;
//          std::this_thread::sleep_for(std::chrono::milliseconds(100));
//      }
//              };
//      
// 
// 
// 
// 
// 
// 
//  
//