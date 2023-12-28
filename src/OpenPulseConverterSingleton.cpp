



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


#define LOG(x) std::cout << "[" << __FILE__ << " Line" << __LINE__ << "] " << x << std::endl;
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
union OpGdata {
    LPVOID OgInput{};

    LPDWORD TrackingData_d;
    DWORD d_buffer;


};




#pragma pack(pop)

class whatIsGlove
{
public:
    whatIsGlove( //baby, Don't hurt me, don't hurt me; no mo'
        int vid, int pid, LPCSTR pipename) : m_handle{ hid_open(vid, pid, nullptr) }, OpgData_buffer{}, m_wstring {}, r_buffer{},
        m_ogPipe{  CreateFile(pipename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr)  } {}
    virtual ~whatIsGlove() { hid_close(m_handle); }

    // true if the glove is connected
    const bool isValid() const { return m_handle; }

    //Glove Functions
    const auto& read() { hid_read(m_handle, r_buffer.buffer, 25); return r_buffer; };
    const auto& write(unsigned char* HapticData) { return hid_write(m_handle, HapticData, 21);  };

    //OpenGlovesDriver Functions
    const auto& Feel() { { ReadFile(m_ogPipe, (LPVOID*)OpgData_buffer.OgInput, sizeof(OpgData_buffer.OgInput), OpgData_buffer.TrackingData_d, NULL); } ; return OpgData_buffer.OgInput;};
    const auto& Touch(LPCVOID TrackingData) {return WriteFile(m_ogPipe, TrackingData, sizeof(TrackingData), OpgData_buffer.TrackingData_d, NULL); };
   
    //Data Functions cause it's neater to shove them here
    const int HapticConvert(int input) { int output = input / 10 * 2.55; return output; }
    const void BitData(const unsigned char data[3] ) { //Took a big bong rip and figured out what I need to do

        

        data0 = data[0];//convert the incoming bytes into bitsets
        data1 = data[1];
        data1Pull = data1 >> 2;//split data 1 into the pull and splay
        data1Splay = data1 & 0b0000000000000011; // this is the mask for int, could also use 0x03 which is the same
        data2 = data[2];

        //We will copy data into the correct form again and use Binary OR to get them combined
        unsigned int  pullLow = (data1Pull);
        unsigned int  pullFar = (data1 << 6);
        unsigned int splayLow = (data2);
        unsigned int splayFar = (data1Splay << 8);

        // Extracting the real numbers via the Binary OR function
        pullBits = (pullLow | pullFar);
        splayBits = (splayLow | splayFar);

    }


    //Callable variables from the FingerData() Function
    unsigned int pull = pullBits;
    unsigned int splay = splayBits;



    

    // device info
    const std::string getManufacturer() { hid_get_manufacturer_string(m_handle, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
    const std::string getProduct() { hid_get_product_string(m_handle, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
    const std::string getSerialNumber() { hid_get_serial_number_string(m_handle, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
    const std::string getIndexedString(const int i) { hid_get_indexed_string(m_handle, i, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
    
private:
    // connection to glove
    hid_device* m_handle;
    // pipe to opengloves
    HANDLE m_ogPipe;

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
    OpGdata OpgData_buffer;
};
// Init Splay and Flex ; Init Tracking and Haptic Data
std::array<float, 5> splay;
std::array<std::array<float, 4>, 5> flexion;
LPCVOID TrackingData;
unsigned char* HapticData;
// make all the variables for our data to get held in

unsigned char report[21]; // Output Report Variable HID api 




void Tracking(whatIsGlove glove) {
    //------Tracking
    const auto& buffer = glove.read();

        glove.BitData(buffer.glove.thumb.data);
        glove.BitData(buffer.glove.index.data);
        glove.BitData(buffer.glove.middle.data);
        glove.BitData(buffer.glove.ring.data);
        glove.BitData(buffer.glove.pinky.data);



    // run the buffer to bit convert our data into the data struct

       //test code to confirm we are getting the data we want

    std::cout << "Pull: " << glove.pull CR;
    std::cout << "Splay: " << glove.splay CR;


    //The data structs for our finger buffer data

    // Create std::array for splay_buffer
    const std::array<float, 5> splay_buffer = {
        static_cast<float>(glove.splay),
        static_cast<float>(glove.splay),
        static_cast<float>(glove.splay),
        static_cast<float>(glove.splay),
        static_cast<float>(glove.splay)
    };


    // Create std::array for pull_buffer
    const std::array < std::array < float, 4 >, 5 > pull_buffer = {
        static_cast<float>(glove.pull),
        static_cast<float>(glove.pull),
        static_cast<float>(glove.pull),
        static_cast<float>(glove.pull),
        static_cast<float>(glove.pull)
    };

    splay = splay_buffer; //Semantics for readability -- no impact on performance
    flexion = pull_buffer;


    // TODO: move data from buffer to ogid -- DONE




    // Convert OpenGloveInputData to LPCvoid
    OpenGloveInputData ogid{}; // Assuming you have an instance of OpenGloveInputData

    //Write your Input data to ogid
    ogid.flexion = flexion;
    ogid.splay = splay;

    // Step 1: Create an LPCvoid pointer variable
    //TrackingData; Done above;

    // Step 2: Allocate memory for the LPCvoid pointer
    TrackingData = malloc(sizeof(OpenGloveInputData));

    // Step 3: Copy the data from the original structure to the allocated memory
    memcpy((void*)TrackingData, &ogid, sizeof(OpenGloveInputData));

    // Step 4: Cast the allocated memory to LPCvoid
    LPCVOID convertedData = (LPCVOID)TrackingData;

    glove.Touch(convertedData); // TODO: write ogid to TrackingData --- DONE

    free((void*)TrackingData);// Dump the memory so we don't leak into the ram
}

void Haptics(whatIsGlove glove) {
    //------FFB
    const auto& f_buffer = glove.Feel();







    if (f_buffer) {//Check if we are receiving force

        printf("%p", f_buffer);

        // Step 1: Extract the values from the LPvoid and assign them to the corresponding fields of the output structure
    // 
    // Convert LPVOID to OutputStructure
        f_buffer; // Assuming you have the LPVOID input data

        // Step 1: Cast the LPVOID to the appropriate structure type
        OutputStructure* outputData = static_cast<OutputStructure*>(f_buffer);

        // Step 2: Access the fields of the structure
        int thumbForceFeedback = outputData->A;
        int indexForceFeedback = outputData->B;
        int middleForceFeedback = outputData->C;
        int ringForceFeedback = outputData->D;
        int pinkyForceFeedback = outputData->E;
        float frequency = outputData->F;
        float duration = outputData->G;//Not used by Pulse ; TODO: thought of a way we can use this by running a sleepfor on this timing
        float amplitude = outputData->H;

        // Now you have the output structure with the extracted values


        //Let's manipulate them with math to get data points the glove understands

        // Bytes 2 and 3 are easy math
        int convertedFreq = frequency * 255; //Byte 3
        int convertedAmp = amplitude * 255; // Byte 2

        // Now its for the rest of the fingers endpoints to be estimated by how far the OpG finger is allowed 
        // OpG uses 1000 as max value to hold users' fingers straight out, value 0 is unrestricted tracking

        // Using the function in the glove class HapticConvert to change our 2 bytes from OpG into 1 byte for Pulse
        // Then we manipulate that to estimate 2 end points of a spring at where the users' fingers should stop

        int convertedThumb = glove.HapticConvert(thumbForceFeedback);
        int convertedIndex = glove.HapticConvert(indexForceFeedback);
        int convertedMiddle =glove.HapticConvert(middleForceFeedback);
        int convertedRing = glove.HapticConvert(ringForceFeedback);
        int convertedPinky = glove.HapticConvert(pinkyForceFeedback);

        //We are going to use a naming convention to align our bytes easier

        int thumb0 = convertedThumb - 64; //byte 0 uses a magic number to consistently give a quarter spring tension, more will increase tension, less will be springier
        int thumb1 = convertedThumb - 127; // Byte 1 uses the half range to adjust down along the force of OpG
        int index0 = convertedIndex - 64;
        int index1 = convertedIndex - 127;
        int middle0 = convertedMiddle - 64;
        int middle1 = convertedMiddle - 127;
        int ring0 = convertedRing - 64;
        int ring1 = convertedRing - 127;
        int pinky0 = convertedPinky - 64;
        int pinky1 = convertedPinky - 127;

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


        glove.write(HapticData);


        //TODO: Write a sleepfor on the duration variable followed by a write call with a second output report as above but with 0's for the vibrations
    };
}

/*
 * Main function that runs upon execution
 */
int main(int argc, char** argv) 
{

  //Opening warning Message window to Alert users of Experimental Code
  char* WarningMB = "This is a developer tool for other developers by using this you accept any and all liability to hardware or psyche. You have been warned!";
   char* title  =  "DEVELOPERS TOOL ONLY";

  
     

    MessageBox(NULL, WarningMB, title, MB_OK);
    

   printf("Hello World! \n This is the OpenPulse Converter, a simple tool to send tracking and haptic data between Bifrost Pulse Gloves and OpenGloves Driver. \n This is a community development from the Pulse Discord. \n Please thank Jagrosh, KingOfDranovis, and Sheridan in the discord when you see them. \n Thank you for using this tool, you are part of an Awesome Club! \n PLEASE TURN ON YOUR GLOVES! \n");
    system("pause");
    // initialize HID lib
    hid_init();
    LOG("HID INIT");
    debugPause;

    // init gloves
    whatIsGlove left{ VENDOR_ID, LEFT_GLOVE_PRODUCT_ID, LEFT_PIPE };
    whatIsGlove right{ VENDOR_ID, RIGHT_GLOVE_PRODUCT_ID, RIGHT_PIPE };
    
    // print diagnostics
    if (!left.isValid() && !right.isValid())
    {
        LOG("No gloves are connected!")
        hid_exit();
        system("pause");
        return 1;
    }

    if (left.isValid())
    {
        LOG("Left Glove Connected!");
        LOG("Manufacturer: " + left.getManufacturer());
        LOG("Product     : " + left.getProduct());
        LOG("SerialNumber: " + left.getSerialNumber());
    }
    else
    {
        LOG("Left Glove was not found!");
    }

    if (right.isValid())
    {
        LOG("Right Glove Connected!");
        LOG("Manufacturer: " + right.getManufacturer());
        LOG("Product     : " + right.getProduct());
        LOG("SerialNumber: " + right.getSerialNumber());
    }
    else
    {
        LOG("Right Glove was not found!");
    }


        //Init Pipe and Opengloves connection
    printf("Attempting connection to OpenGloves Driver, please start SteamVR with OpG running, then continue with this plugin.");
    system("pause");
    
    


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

           // Force Feedback Haptics----------------------

            Haptics(left);
         


        }

        if (right.isValid())
        {
            
            //Tracking---------------

            Tracking(right);

            // Force Feedback Haptics----------------------

            Haptics(right);


        }
        std::this_thread::sleep_for(std::chrono::microseconds(1000000 / 67)); // 67 hz  <-- This is really cool


        //--------When writing FFB Output Reports from the open pipe reading from Opengloves driver, Outputs are only triggered after sending input to the driver.
    }

   












    // close the hidapi library
    hid_exit();

    return 0;
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



//Notes to be deprecated---------------------


/// <summary>
/// FingerData finger;

//finger.data[0] = 0xAB;
//finger.data[1] = 0xCD;
//finger.data[2] = 0xEF;

// Extracting the real numbers
//unsigned int pull = (finger.data[0] << 6) | (finger.data[1] >> 2);
//u/nsigned int splay = ((finger.data[1] & 0x03) << 8) | finger.data[2];

//std::cout << "Pull: " << pull << std::endl;
//std::cout << "Splay: " << splay << std::endl;

/// </summary>
