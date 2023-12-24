
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
    const auto& write(unsigned char* HapticData) { hid_write(m_handle, HapticData, 21); return HapticData; };

    //OpenGlovesDriver Functions
    const auto& Touch(LPVOID TrackingData) { WriteFile(m_ogPipe, TrackingData, 24, OpgData_buffer.TrackingData_d, NULL); return TrackingData; };
    const auto& Feel() { ReadFile(m_ogPipe, d_Buffer, 32, OpgData_buffer.TrackingData_d, NULL); return d_Buffer; };

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

    LOG("Hello World! Beginning of JagRosh Contribution, thank you!"); //Maybe change to a general Contribution thank you
    system("pause");
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
    
    

    
    LPVOID TrackingData; // make a variable for our data to get held in
    unsigned char* HapticData;
       
    //FFB section-----------------------------------------------------------------------------------






    



    // begin loop to run everything at 67hz
    bool exit = false;
    while (!exit)
    {
        if (left.isValid())
        {
            //------Tracking
            const auto& buffer = left.read();
            OpenGloveInputData ogid{};
            // TODO: move data from buffer to ogid
            

            // TODO: test code, remove later:
            printf("buffer: ");
            for (int i = 0; i < sizeof(buffer); i++)
                printf("%d ", buffer.buffer[i]);
            printf("\r");

            left.Touch(TrackingData); // TODO: write ogid to named pipe


            //------FFB
            const auto& f_buffer = left.Feel();

            printf("Force:");
            
                printf("%d ", int(f_buffer));
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
            OpenGloveInputData ogid{};
            // TODO: move data from buffer to ogid
            // TODO: write ogid to named pipe

            // TODO: test code, remove later:----maybe... it looks pretty cool
            printf("buffer: ");
            for (int i = 0; i < sizeof(buffer); i++)
                printf("%d ", buffer.buffer[i]);
            printf("\r");


            //----FFB


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