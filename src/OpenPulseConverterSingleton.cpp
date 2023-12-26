
// #include<hidusage.h>
// #include<hidclass.h>
// #include<hidsdi.h>
// #include<hidpi.h>


// Including code contributions by Jagrosh, https://github.com/jagrosh/Pulse2OpenGloves/blob/master/src/main.cpp Thanks mate




#include <iostream>
#include <array>
#include <thread>
#include <chrono>
#include<cstdio>
// https://github.com/libusb/hidapi
#include <hidapi.h>
//#include<fileapi.h>
#include <windows.h> 
#include <cstring>





  

        //-----------------------Functions for gloves


#define LOG(x) std::cout << "[" << __FILE__ << " L" << __LINE__ << "] " << x << std::endl;
#define MAX_STR 255

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
    unsigned char data[3];
    //unsigned int pull : 14;
    //unsigned int splay : 10;

    // Extracting the real numbers
    unsigned int pull = static_cast<unsigned int>(data[0] | (data[1] >> 2));
    unsigned int splay = static_cast<unsigned int>((data[1] << 6) | data[2]);
} FingerData;

typedef struct GloveInputReport
{
    unsigned char reportId : 8;
    FingerData thumb, index, middle, ring, pinky;
} GloveInputReport;

union OpGdata {
    OpenGloveInputData OgInput; //some todos here about pulling the wanton variables out of the functions;

    LPDWORD TrackingData_d;
    


};

typedef struct OutputStructure {
    int A;
    int B;
    int C;
    int D;
    int E;
    float F;
    float G;
    float H;
} OutputStructure;

union HIDBuffer
{
    GloveInputReport glove;
    unsigned char buffer[sizeof(glove)];
};
#pragma pack(pop)

class Glove
{
public:
    Glove(int vid, int pid, LPCSTR pipename) : m_handle{ hid_open(vid, pid, nullptr) }, d_Buffer{}, OpgData_buffer{}, m_wstring {}, m_buffer{},
        m_ogPipe{ CreateFile(pipename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr) } {}
    virtual ~Glove() { hid_close(m_handle); }

    // true if the glove is connected
    const bool isValid() const { return m_handle; }

    //Glove Functions
    const auto& read() { hid_read(m_handle, m_buffer.buffer, 25); return m_buffer; };
    const auto& write(unsigned char* HapticData) { return hid_write(m_handle, HapticData, 21);  };

    //OpenGlovesDriver Functions
    const auto& Feel() { ReadFile(m_ogPipe, d_Buffer, 32, OpgData_buffer.TrackingData_d, NULL); return d_Buffer; };
    const auto& Touch(LPCVOID TrackingData) {return WriteFile(m_ogPipe, TrackingData, 24, OpgData_buffer.TrackingData_d, NULL); };
   
    //Data Functions cause it's neater to shove them here
    const int HapticConvert(int input) { int output = input / 10 * 2.55; return output; }
    

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

    // temp vars
    wchar_t m_wstring[MAX_STR];
    HIDBuffer m_buffer;
    LPVOID d_Buffer;
    OpGdata OpgData_buffer;
};


/*
 * Main function that runs upon execution
 */
int main(int argc, char** argv) 
{

  //Opening warning Message window to Alert users of Experimental Code
  char* WarningMB = "This is a developer tool for other developers by using this you accept any and all liability to hardware or psyche. You have been warned!";
   char* title  =  "DEVELOPERS TOOL ONLY";

  
     

    MessageBox(NULL, WarningMB, title, MB_OK);
    system("pause");

    LOG("Hello World! \n This is the OpenPulse Converter, a simple tool to send tracking and haptic data between Bifrost Pulse Gloves and OpenGloves Driver. \n This is a community development from the Pulse Discord. \n Please thank Jagrosh, KingOfDranovis, and Sheridan in the discord when you see them. \n Thank you for using this tool, you are part of an Awesome Club!");
   
    // initialize HID lib
    hid_init();

    // init gloves
    Glove left{ VENDOR_ID, LEFT_GLOVE_PRODUCT_ID, LEFT_PIPE };
    Glove right{ VENDOR_ID, RIGHT_GLOVE_PRODUCT_ID, RIGHT_PIPE };

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
    
    // Init Splay and Flex Init Tracking and Haptic Data
    std::array<float, 5> splay;
    std::array<std::array<float, 4>, 5> pull;
    LPCVOID TrackingData;
    unsigned char* HapticData;
    // make all the variables for our data to get held in
    
    unsigned char report[21]; // Output Report Variable HID api 

    //FFB section-----------------------------------------------------------------------------------

    



    



    // begin loop to run everything at 67hz
    
    while (1)
    {
        if (left.isValid())
        {
            //------Tracking
            const auto& buffer = left.read();



            
                //Please just print the numbers I want. Found out that it's not the numbers we need

                std::cout << "Pull: " << buffer.glove.thumb.pull << std::endl;
                std::cout << "Splay: " << buffer.glove.thumb.splay << std::endl;
            

                //Whatever I'm making the data structs
                const std::array<float, 5> splay_buffer = [buffer.glove.thumb.splay, buffer.glove.index.splay, buffer.glove.middle.splay, buffer.glove.ring.splay, buffer.glove.pinky.splay];
                const std::array<float, 5> pull_buffer = [buffer.glove.thumb.pull, buffer.glove.index.pull, buffer.glove.middle.pull, buffer.glove.ring.pull, buffer.glove.pinky.pull];

                pull = [pull_buffer];
                splay = [splay_buffer];
                

            
            // TODO: move data from buffer to ogid 
           



            // Convert OpenGloveInputData to LPCvoid
            OpenGloveInputData ogid{ pull, splay }; // Assuming you have an instance of OpenGloveInputData

            // Step 1: Create an LPCvoid pointer variable
            // TrackingData; done above

            // Step 2: Allocate memory for the LPCvoid pointer
            TrackingData = malloc(sizeof(OpenGloveInputData));

            // Step 3: Copy the data from the original structure to the allocated memory
            memcpy((void*)TrackingData, &ogid, sizeof(OpenGloveInputData));

            // Step 4: Cast the allocated memory to LPCvoid
            LPCVOID convertedData = (LPCVOID)TrackingData;

            left.Touch(convertedData); // TODO: write ogid to TrackingData --- DONE
            
            free((void*)TrackingData);// Dump the memory so we don't leak into the ram

            //------FFB
            const auto& f_buffer = left.Feel();

            printf("Force:");
            
            

           

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
            float duration = outputData->G;//Not used by Pulse 
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

            int convertedThumb = left.HapticConvert(thumbForceFeedback);
            int convertedIndex = left.HapticConvert(indexForceFeedback);
            int convertedMiddle = left.HapticConvert(middleForceFeedback);
            int convertedRing = left.HapticConvert(ringForceFeedback);
            int convertedPinky = left.HapticConvert(pinkyForceFeedback);

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
            


            
             HapticData = report;// Just semantics for readability, compiler will ignore this -- no impact on performance
            
             printf("%c", HapticData);

            printf("\r");

            //TODO: Parse f_buffer to learn the OpG data struct more clearly
            //TODO: Apply math to each finger motor
            //TODO: Collect each finger into an bytearray called HapticData

            left.write(HapticData);
         


        }

        if (right.isValid())
        {
            //----Tracking
            const auto& buffer = right.read();


           
            // TODO: move data from buffer to ogid
            
            pull = [buffer];
            splay = [buffer];



            // Convert OpenGloveInputData to LPCvoid
            OpenGloveInputData ogid{ pull, splay }; // Assuming you have an instance of OpenGloveInputData

            // Step 1: Create an LPCvoid pointer variable
            // TrackingData; done above

            // Step 2: Allocate memory for the LPCvoid pointer
            TrackingData = malloc(sizeof(OpenGloveInputData));

            // Step 3: Copy the data from the original structure to the allocated memory
            memcpy((void*)TrackingData, &ogid, sizeof(OpenGloveInputData));

            // Step 4: Cast the allocated memory to LPCvoid
            LPCVOID convertedData = (LPCVOID)TrackingData;

            left.Touch(convertedData); // TODO: write ogid to TrackingData --- DONE

            free((void*)TrackingData);// Dump the memory so we don't leak into the ram





            //----FFB
            const auto& f_buffer = right.Feel();

            printf("Force:");





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
            float duration = outputData->G;//Not used by Pulse 
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

            int convertedThumb = right.HapticConvert(thumbForceFeedback);
            int convertedIndex = right.HapticConvert(indexForceFeedback);
            int convertedMiddle = right.HapticConvert(middleForceFeedback);
            int convertedRing = right.HapticConvert(ringForceFeedback);
            int convertedPinky = right.HapticConvert(pinkyForceFeedback);

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




            HapticData = report;// Just semantics for readability, compiler will ignore this -- no impact on performance

            printf("%c", HapticData);

            printf("\r");

            //TODO: Parse f_buffer to learn the OpG data struct more clearly
            //TODO: Apply math to each finger motor
            //TODO: Collect each finger into an bytearray called HapticData

            left.write(HapticData);



        }
        std::this_thread::sleep_for(std::chrono::microseconds(1000000 / 67)); // 67 hz  <-- This is really cool


        //--------writing FFB Output Reports from the open pipe reading from Opengloves driver. Outputs are only triggered after sending input to the driver.
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
