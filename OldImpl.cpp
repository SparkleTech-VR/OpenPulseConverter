//FFB math theory-----------OLD just read the code don't rely on this

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
