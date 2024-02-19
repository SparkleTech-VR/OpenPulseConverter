// Including code contributions by Jagrosh, https://github.com/jagrosh/Pulse2OpenGloves/blob/master/src/main.cpp Thanks mate
// https://github.com/libusb/hidapi
#include <hidapi.h>
#include <iostream>
#include <array>
#include <thread>
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <limits>
#include <windows.h> 
#include <cstring>
#include<OneEuroFilter.h>
#include "InputTypes.h"
#include "ExponentialFilter.h"
#include "OPGData.h"


//-----------------------Functions for gloves------------------------------
#define TOP_SpringPoint 25 //This is the top of the spring stop extended to default: 25
#define Default_Range 25 //use this to change the Default range on the reset report if you prefer a different tension 
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

typedef struct finT {
	//Paired Data for Return needs
	 int pull;
	 int splay;
} finT;
//Calibration variables, side note:ordering is important here because Tracking, whatIsGlove, and runCalib use these
int thumbPimp{};
int indexPimp{};
int middlePimp{};
int ringPimp{};
int pinkyPimp{};
int thumbSpread{};
int indexSpread{};
int middleSpread{};
int ringSpread{};
int pinkySpread{};
int thumbFist{};
int indexFist{};
int middleFist{};
int ringFist{};
int pinkyFist{};
int thumbDrag{};
int indexDrag{};
int middleDrag{};
int ringDrag{};
int pinkyDrag{};
class whatIsGlove //baby, Don't hurt me, don't hurt me; no mo'
{

public:
	whatIsGlove(int vid, int pid) {
		m_handle = hid_open(vid, pid, nullptr);
	}
	//virtual ~whatIsGlove() { hid_close(m_handle); }; //CAREFUL!!! THIS LEADS TO CRASH BEHAVIOR, I know destructors are proper code, however here we are cleaning with hid_exit

	// true if the glove is connected
	const bool isValid() const { return m_handle; }

	//Glove Functions

	const auto& read() {
		if (m_handle != INVALID_HANDLE_VALUE) {
			auto result = hid_read(m_handle, r_buffer.buffer, 16);
			if (result == -1) {
			std::cout << "Handle being Read:" << &m_handle << std::endl;
				const auto error = hid_error(m_handle);
				std::cout << "Error while reading HID data: " << error << "||Handle with bad Read:" << m_handle << std::endl;
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

			if (i == 9) { printf("Pipe about to timeout\n"); std::cout << "Named pipe Attempt Ended! NO NAMED PIPE WORKING!" << std::endl; break; }

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
	const auto& Feel() {
		char buffer[sizeof(OutputStructure)];
		
		DWORD dwRead;
		bool returnCheck = ReadFile(m_ogPipe, buffer, sizeof(OutputStructure), &dwRead, NULL);
		if (returnCheck) {
			OutputStructure OutData = reinterpret_cast<OutputStructure&>(buffer);
			return OutData;
		}
		else {
			OutputStructure trashzero{};
			auto error = GetLastError();
			DISPLAY("NO HAPTICS > BAD READ" + error)
			return trashzero;
		}
	};
	template <typename T>
	const bool Touch(const T& TrackingData) {
		DWORD dwWritten{};
		return WriteFile(m_ogPipe, (LPCVOID)&TrackingData, sizeof(TrackingData), &dwWritten, NULL);
	};
	const bool pipeIsValid() const { return m_ogPipe; };

	void closePipe() {
		CloseHandle(m_ogPipe);
	}

	//Data Functions cause it's neater to shove them here
	const float isCurled(int finData, int minFin, int maxFin) { float sentFloat = std::abs((((float)finData-(float)minFin) / ((float)maxFin-(float)minFin))-1); return sentFloat; }; 
	const float splayNormalized(int finData, int minFin, int maxFin) { float sentFloat = std::abs((((float)finData - (float)minFin) / ((float)maxFin - (float)minFin)) - 1); return sentFloat; };
	const finT BitData(FingerData data) { //Took a big bong rip and figured out what I need to do
		// Extracting the real numbers via the Bitfield shorts aka OnionDicer
		Bits = data;
		splayBits = Bits.getSplay();
		pullBits = Bits.getPull();
		//     ________.
		//	 /        / \
		//	/ _______/ ` \
		//	||| | |||| | |  <----The Onion Dicer with sample onion aboard
		//	|||(_)|||| | |
		//	|_|_|_|__| |/
		//	|_|_|_|__| / 
		//
		//Sorry pix I made some workspace for my Onion dicer
		//OneEuroFilter courtesy of https://github.com/casiez/OneEuroFilter
	 //OneEuroFilter vars
		//double frequency = 67; // Hz
		//double mincutoff = 1.0; // Hz
		//double beta = 10; //Tolerance, adjust for smoothness
		//double dcutoff = 1;//Timing, don't mess with this one
		//std::cout << "timestamp,noisy,filtered,smooth" << std::endl;
		//OneEuroFilter f(frequency, mincutoff, beta, dcutoff);
		//// Get the current system time
		//auto currentTime = std::chrono::system_clock::now();
		//
		//double noisyPull = (double)pullBits;
		//double noisySplay = (double)splayBits;
		//double ts = std::chrono::duration<double>(currentTime.time_since_epoch()).count(); // Convert the system time to a double value
		//
		//double filteredPull = f.filter(noisyPull, ts);
		//double filteredSplay = f.filter(noisySplay, ts);
		////double smoothPull = ExpFilter.filter(filteredPull);
		////double smoothSplay = ExpFilter.filter(filteredSplay); needs a slightly different filter for splay if needed at all
		////This is an idea to use exponential smoothing to force the finger data to be more useful at the cost of *significant* fidelity and lag
		//std::cout << std::setprecision(std::numeric_limits<double>::digits10)
		//	<< ts << ","
		//	<< pullBits << ","
		//	<< splayBits << ","
		//	<< noisyPull << ","
		//	<< noisySplay << ","
		//	<< filteredPull << ","
		//	<< filteredSplay <<","
		//	//<< smoothPull
		//	<< std::endl;

		finT ParsedData{ (int)pullBits, (int)splayBits };

		return ParsedData;
	}

	// device info
	const std::string getManufacturer() { hid_get_manufacturer_string(m_handle, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
	const std::string getProduct() { hid_get_product_string(m_handle, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
	const std::string getSerialNumber() { hid_get_serial_number_string(m_handle, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }
	const std::string getIndexedString(const int i) { hid_get_indexed_string(m_handle, i, m_wstring, MAX_STR); std::wstring temp{ m_wstring }; return { temp.begin(), temp.end() }; }

private:
	// connection to glove and pipe
	hid_device* m_handle = nullptr;
	HANDLE m_ogPipe{};
	//Init the finger Bytes -- not a snack!
	FingerData Bits{};
	unsigned int pullBits{};
	unsigned int splayBits{};
	// temp vars
	wchar_t m_wstring[MAX_STR] = {};
	HIDBuffer r_buffer = {};
};
const void runCalibration(whatIsGlove glove) {//Holy Pasta help me
	int secAvg = 5;
	int inSecAvg{ 67 }; 
	std::this_thread::sleep_for(std::chrono::seconds(1));
	DISPLAY(glove.getSerialNumber());
	DISPLAY("CALIBRATION STARTING...");
	int thumbSpreadSum {};
	int indexSpreadSum {};
	int middleSpreadSum{};
	int ringSpreadSum  {};
	int pinkySpreadSum {};
	DISPLAY("Please SPREAD your hand comfortably...");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	for (int i{ secAvg }; i > 0; i--) {
		for (int l{}; l < inSecAvg; l++) {
			//------Tracking
			auto& buffer = glove.read();

			// run the buffer to bit convert our data into the data struct
			finT thumbTracking = glove.BitData(buffer.glove.thumb);
			finT indexTracking = glove.BitData(buffer.glove.index);
			finT middleTracking = glove.BitData(buffer.glove.middle);
			finT ringTracking = glove.BitData(buffer.glove.ring);
			finT pinkyTracking = glove.BitData(buffer.glove.pinky);
			//Splays from the paired Data
			unsigned int thumbSplay = thumbTracking.splay;
			unsigned int indexSplay = indexTracking.splay;
			unsigned int middleSplay=middleTracking.splay;
			unsigned int  ringSplay =   ringTracking.splay;
			unsigned int pinkySplay = pinkyTracking.splay;
			//Assign the flat Spread
			  thumbSpread  += thumbSplay;
			  indexSpread  += indexSplay;
			 middleSpread += middleSplay;
			   ringSpread   += ringSplay;
			  pinkySpread  += pinkySplay;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / inSecAvg));
		}
		thumbSpreadSum  += thumbSpread  /inSecAvg;
		indexSpreadSum  += indexSpread  /inSecAvg;
		middleSpreadSum += middleSpread/inSecAvg;
		ringSpreadSum   += ringSpread	  /inSecAvg;
		pinkySpreadSum  += pinkySpread  /inSecAvg; 
		DISPLAY(i << "...");
	}
	thumbSpread	= (thumbSpreadSum/ secAvg) / 3;
	indexSpread	= (indexSpreadSum/ secAvg) / 3;
	middleSpread= (middleSpreadSum/ secAvg) / 3;
	ringSpread	= (ringSpreadSum/ secAvg) / 3;
	pinkySpread = (pinkySpreadSum/ secAvg) / 3;
	int pimpThumbSum{};//b*tch
	int pimpIndexSum{};//b*tch
	int pimpMiddleSum{};//b*tch
	int pimpRingSum{};//b*tch
	int pimpPinkySum{};//b*tch
	int flattenThumbSum{};//b*tch
	int flattenIndexSum{};//b*tch
	int flattenMiddleSum{};//b*tch
	int flattenRingSum{};//b*tch
	int flattenPinkySum{};//b*tch
	DISPLAY("Please FLATTEN your hand comfortably...");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	for (int i{secAvg}; i > 0; i--) {
		for (int l{}; l < inSecAvg; l++) {
			//------Tracking
			auto& buffer = glove.read();

			// run the buffer to bit convert our data into the data struct
			finT thumbTracking = glove.BitData(buffer.glove.thumb);
			finT indexTracking = glove.BitData(buffer.glove.index);
			finT middleTracking = glove.BitData(buffer.glove.middle);
			finT ringTracking = glove.BitData(buffer.glove.ring);
			finT pinkyTracking = glove.BitData(buffer.glove.pinky);
			//Pulls from the paired Data
			unsigned int thumbPull = thumbTracking.pull;
			unsigned int indexPull = indexTracking.pull;
			unsigned int middlePull = middleTracking.pull;
			unsigned int ringPull = ringTracking.pull;
			unsigned int pinkyPull = pinkyTracking.pull;
			//Assign the flat Drag
			thumbDrag += thumbPull;
			indexDrag  += indexPull;
			middleDrag += middlePull;
			ringDrag   += ringPull;
			pinkyDrag  += pinkyPull;
			//Splays from the paired Data
			unsigned int thumbSplay = thumbTracking.splay;
			unsigned int indexSplay = indexTracking.splay;
			unsigned int middleSplay = middleTracking.splay;
			unsigned int  ringSplay = ringTracking.splay;
			unsigned int pinkySplay = pinkyTracking.splay;
			//Assign the flat Pimp
			 thumbPimp += thumbSplay;
			 indexPimp += indexSplay;
			middlePimp += middleSplay;
			  ringPimp += ringSplay;
			 pinkyPimp += pinkySplay;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / inSecAvg));
		}
		flattenThumbSum += thumbDrag /inSecAvg;
		flattenIndexSum += indexDrag /inSecAvg;
		flattenMiddleSum+= middleDrag/inSecAvg;
		flattenRingSum  += ringDrag  /inSecAvg;
		flattenPinkySum += pinkyDrag /inSecAvg;
		pimpThumbSum += thumbPimp/inSecAvg;
		pimpIndexSum += indexPimp/inSecAvg;
		pimpMiddleSum += middlePimp/inSecAvg;
		pimpRingSum  += ringPimp/inSecAvg;
		pimpPinkySum += pinkyPimp/inSecAvg;
		DISPLAY(i << "...");
	}
	thumbDrag  = (flattenThumbSum /secAvg) / 3;
	indexDrag  = (flattenIndexSum /secAvg) / 3;
	middleDrag = (flattenMiddleSum/secAvg) / 3;
	ringDrag   = (flattenRingSum  /secAvg) / 3;
	pinkyDrag  = (flattenPinkySum /secAvg) / 3;
	thumbPimp  = (pimpThumbSum/secAvg) / 3;
	indexPimp  = (pimpIndexSum/secAvg) / 3;
	middlePimp = (pimpMiddleSum/secAvg) / 3;
	ringPimp   = (pimpRingSum/secAvg) / 3;
	pinkyPimp  = (pimpPinkySum/secAvg) / 3;
	DISPLAY("Please CURL your hand into a FIST comfortably...")
		std::this_thread::sleep_for(std::chrono::seconds(1));
		int thumbFistSum{};
		int indexFistSum{};
		int middleFistSum{};
		int ringFistSum{};
		int pinkyFistSum{};
		for (int i{ secAvg }; i > 0; i--) {
			for (int l{}; l < inSecAvg; l++) {
				//------Tracking
				auto& buffer = glove.read();

				// run the buffer to bit convert our data into the data struct
				finT thumbTracking = glove.BitData(buffer.glove.thumb);
				finT indexTracking = glove.BitData(buffer.glove.index);
				finT middleTracking = glove.BitData(buffer.glove.middle);
				finT ringTracking = glove.BitData(buffer.glove.ring);
				finT pinkyTracking = glove.BitData(buffer.glove.pinky);
				//Pulls from the paired Data
				unsigned int thumbPull = thumbTracking.pull;
				unsigned int indexPull = indexTracking.pull;
				unsigned int middlePull = middleTracking.pull;
				unsigned int ringPull = ringTracking.pull;
				unsigned int pinkyPull = pinkyTracking.pull;
				//Assign the curled Fist
				 thumbFist += thumbPull;
				 indexFist += indexPull;
				middleFist += middlePull;
				  ringFist += ringPull;
				 pinkyFist += pinkyPull;
				std::this_thread::sleep_for(std::chrono::milliseconds(1000 / inSecAvg));
			}
			thumbFistSum += thumbFist / inSecAvg;
			indexFistSum += indexFist / inSecAvg;
			middleFistSum += middleFist / inSecAvg;
			ringFistSum += ringFist / inSecAvg;
			pinkyFistSum += pinkyFist / inSecAvg;
			DISPLAY(i << "...");
		}
		thumbFist = (thumbFistSum/ secAvg) / 3;
		indexFist = (indexFistSum/ secAvg) / 3;
		middleFist =(middleFistSum/ secAvg) / 3;
		ringFist = (ringFistSum / secAvg) / 3;
		pinkyFist = (pinkyFistSum/ secAvg)/3;

		DISPLAY("Index Drag:" << indexDrag << "Index Curl:" << indexFist << "Index Spread:" << indexSpread << "Index Pimp:" << indexPimp);
		DISPLAY("Please confirm your index finger min and max seem normal. Pulse measures 0 when the finger string is fully pulled out of the module, and measures 10K when fully retracted into the module");
		printf("Please CONFIRM YOUR CALIBRATION! Do you need to repeat Calibaration? Y/N \n");
};
unsigned int clamp(int value, int minT, int maxT) {unsigned int yeet = max(minT, min(value,maxT));return yeet;}
void Tracking(whatIsGlove glove) {

	//------Tracking
	auto& buffer = glove.read();

	// run the buffer to bit convert our data into the data struct
	finT thumbTracking = glove.BitData(buffer.glove.thumb);
	finT indexTracking = glove.BitData(buffer.glove.index);
	finT middleTracking = glove.BitData(buffer.glove.middle);
	finT ringTracking = glove.BitData(buffer.glove.ring);
	finT pinkyTracking = glove.BitData(buffer.glove.pinky);


	printf("buffer: ");
	for (int i = 0; i < sizeof(buffer); i++)
		printf("%d ", buffer.buffer[i]);
	printf("\n");


	//Pulls from the paired Data
	unsigned int thumbPull = clamp(thumbTracking.pull,thumbFist,thumbDrag);
	unsigned int indexPull = clamp(indexTracking.pull,indexFist,indexDrag);
	unsigned int middlePull = clamp(middleTracking.pull,middleFist,middleDrag);
	unsigned int ringPull = clamp(ringTracking.pull,ringFist,ringDrag);
	unsigned int pinkyPull = clamp(pinkyTracking.pull,pinkyFist,pinkyDrag);

	//Splays from the Paired Data
	unsigned int thumbSplay = clamp(thumbTracking.splay,thumbPimp,thumbSpread);
	unsigned int indexSplay = clamp(indexTracking.splay,indexPimp,indexSpread);
	unsigned int middleSplay =clamp(middleTracking.splay,middlePimp,middleSpread);
	unsigned int ringSplay =  clamp(ringTracking.splay,ringPimp, ringSpread);
	unsigned int pinkySplay = clamp(pinkyTracking.splay,pinkyPimp,pinkySpread);

	//test code to confirm we are getting the data we want
	std::cout << "Pull: " << indexPull << " (" << buffer.glove.index.getPull() << ")"; printf("\n");
	std::cout << "Splay: " << indexSplay << " (" << buffer.glove.index.getSplay() << ")"; // CR CR;


	//The data structs for our finger buffer data

	// Create std::array for splay_buffer
	const std::array<float, 5> splay_buffer = {
		glove.splayNormalized(thumbSplay,thumbPimp,thumbSpread),
		glove.splayNormalized(indexSplay,indexPimp,indexSpread),
		glove.splayNormalized(middleSplay,middlePimp,middleSpread),
		glove.splayNormalized(ringSplay,ringPimp,ringSpread),
		glove.splayNormalized(pinkySplay,pinkyPimp,pinkySpread)
	};


	// Create std::array for pull_buffer
	std::array < std::array < float, 4 >, 5 > pull_buffer{};
	for (int phalanx = 0; phalanx < 4; phalanx++) {
		pull_buffer[0][phalanx] = { glove.isCurled(thumbPull,thumbFist,thumbDrag) };
	}
	for (int phalanx = 0; phalanx < 4; phalanx++) {
		pull_buffer[1][phalanx] = { glove.isCurled(indexPull,indexFist,indexDrag) };
	}
	for (int phalanx = 0; phalanx < 4; phalanx++) {
		pull_buffer[2][phalanx] = { glove.isCurled(middlePull,middleFist,middleDrag) };
	}
	for (int phalanx = 0; phalanx < 4; phalanx++) {
		pull_buffer[3][phalanx] = { glove.isCurled(ringPull,ringFist,ringDrag) };
	}
	for (int phalanx = 0; phalanx < 4; phalanx++) {
		pull_buffer[4][phalanx] = { glove.isCurled(pinkyPull,pinkyFist,pinkyDrag) };
	}
	// Init Splay and Flex
	// make all the variables for our data to get held in
	std::array<float, 5> splay;
	std::array<std::array<float, 4>, 5> flexion;
	splay = splay_buffer; //Semantics for readability -- no impact on performance
	flexion = pull_buffer;

	OpenGloveInputData ogid{};
	//Write your Input data to ogid
	ogid.flexion = flexion;
	ogid.splay = splay;
	//buttons to be emulated, menu, joyx,joyY, maybe others as I care in games
	ogid.grab = ((ogid.flexion[3][0]) > 0.75f) && ((ogid.flexion[4][0]) > 0.75f);//Possible Grab
	ogid.trgButton = ((ogid.flexion[1][0]) > 0.75f); //Possible Trg, might empty mags accidently
	ogid.trgValue = (ogid.flexion[1][0]); //example code for rest of buttons
	ogid.aButton = ((ogid.flexion[0][0])>0.75f);
	ogid.pinch = ((ogid.flexion[0][0]) > 0.90f);
	
	glove.Touch(ogid);
	LOG("wrote");
	if (GetLastError()) {
		DWORD errorCode = GetLastError();
		std::cout << "bad error:__" << errorCode << std::endl;
		debugPause;
	};
}
const int HapticConvert(int input) { int output = input / 40; return output; } // set over 40 this reduces the output haptics to a Pulse reasonable standard
void Haptics(OutputStructure ogod, whatIsGlove glove) {

	//--------When reading FFB Output Reports from the open pipe from Opengloves driver, Outputs are only triggered after sending input to the driver.

	//------FFB
	

	// Step 1: Extract the values from the  incoming OutputStructure and assign them to the corresponding fields of the output structure

	OutputStructure outputData = ogod;

	//subStep 1: Show data
	const int f_buffer = outputData.B;//Oh Flying Spaghetti Monster, Bless this variable *Praise Be to his noodly goodness*

	//Check if we are receiving force

	printf("%d \n", f_buffer);
	// Step 2: Access the fields of the structure
	int thumbForceFeedback =	outputData.A;
	int indexForceFeedback =	outputData.B;
	int middleForceFeedback =	outputData.C;
	int ringForceFeedback =		outputData.D;
	int pinkyForceFeedback =	outputData.E;
	float frequency =			outputData.F;
	float duration =			outputData.G;//Not used by Pulse ;DONE: thought of a way we can use this by running a sleepfor on this timing
	float amplitude =			outputData.H;

	// Now you have the output structure with the extracted values


	//Let's manipulate them with math to get data points the glove understands

	// Bytes 2 and 3 are easy math
	int convertedFreq = frequency * 255.f; //Byte 3
	int convertedAmp = amplitude * 255.f; // Byte 2

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

	int thumb0 = TOP_SpringPoint; //byte 0 uses a extension point at the finger tip to consistently give spring tension, more will increase tension, less will be springier; default: 25
	int thumb1 = convertedThumb; // Byte 1 uses 40th'd Data to adjust down along the force of OpG; Incoming 1000/40 = 25 <- your fingertip
	int index0 = TOP_SpringPoint;
	int index1 = convertedIndex;
	int middle0 = TOP_SpringPoint;
	int middle1 = convertedMiddle;
	int ring0 = TOP_SpringPoint;
	int ring1 = convertedRing;
	int pinky0 = TOP_SpringPoint;
	int pinky1 = convertedPinky;

	unsigned char* HapticData;

	unsigned char report[21]; // Output Report Variable HID api 

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

	std::cout << HapticData << std::endl;

	//CR;

	glove.write(HapticData);//Feel the VR beneath your finger tips!

	//TODO: Write a sleepfor on the duration G variable followed by a write call with a second output report as above but with 0's for the vibrations

	std::this_thread::sleep_for(std::chrono::milliseconds((int)duration * 1000));

	//Now to reset our default tracking as described in pulse basics-- https://www.bifrostvr.com/pulse/basics 
	// THIS IS IMPORTANT --- By default endpoint 1 is set to 25 and endpoint 2 is set to 0
	// Output Report Creator

	report[0] = 0x02;// First Byte needs to be 02 for the Pulse to read it

	//Convert and store the data into the rest of the report

	// Thumb force feedback (A)
	report[1] = Default_Range & 0xFF;
	report[2] = 0 & 0xFF;
	report[3] = 0 & 0xFF;
	report[4] = 0 & 0xFF;
	// Index force feedback (B)
	report[5] = Default_Range & 0xFF;
	report[6] = 0 & 0xFF;
	report[7] = 0 & 0xFF;
	report[8] = 0 & 0xFF;
	// Middle force feedback (C)
	report[9] = Default_Range & 0xFF;
	report[10] = 0 & 0xFF;
	report[11] = 0 & 0xFF;
	report[12] = 0 & 0xFF;
	// Ring force feedback (D)
	report[13] = Default_Range & 0xFF;
	report[14] = 0 & 0xFF;
	report[15] = 0 & 0xFF;
	report[16] = 0 & 0xFF;
	// Pinky force feedback (E)
	report[17] = Default_Range & 0xFF;
	report[18] = 0 & 0xFF;
	report[19] = 0 & 0xFF;
	report[20] = 0 & 0xFF;

	printf("Force:");


	HapticData = report;// Just semantics for readability, compiler will ignore this -- no impact on performance

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


	printf("Hello World! \n This is the OpenPulse Converter. \n A simple tool to send tracking and haptic data between Bifrost Pulse Gloves and OpenGloves Driver. \n This is a community development from the Pulse Discord. \n Please thank Pixelmod, Jagrosh, N10A, KingOfDranovis, and Sheridan in the discord when you see them. \n Thank you for using this tool, you are part of an Awesome Club! \n PLEASE TURN ON YOUR GLOVES! \n");
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

	printf("Please EQUIP YOUR GLOVE(s) NOW. Prepare for CALIBRATION! \n");
	system("pause");
//This is the Spread, Flatten, and Fist Variable setup
	char choice;
	do {
		
		
		std::cout << "Do you want to Begin the Calibration process? (Y/N): ";
		std::cin >> choice;
		if (choice == 'Y' || choice == 'y') {
			//Calib Right
			if (right.isValid()) {
				DISPLAY("RIGHT GLOVE CALIBRATION...")
					runCalibration(right);
			}
			//Calib Left
			if (left.isValid()) {
				DISPLAY("LEFT GLOVE CALIBRATION...")
					runCalibration(left);
			}
			std::cout << "Repeating the process..." << std::endl;
		}
		else if (choice == 'N' || choice == 'n') {
			std::cout << "Stopping the process." << std::endl;
			break;  // Exit the loop
		}
		else {
			std::cout << "Invalid input. Please enter Y or N." << std::endl;
		}
	} while (true);
	
	printf("OpenPulse Primed for game pipes, please begin game boot flow and Good Luck! REMEMBER TO START THE DATA STREAM BELOW!! \n");
	system("pause");

	//Below this line is only for runtime code; all startup code should be above this line-----------------------------------------------------



	//begin loop to run everything at 67hz
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
		 //Functions after the glove Data-------
		std::this_thread::sleep_for(std::chrono::microseconds(1000000 / 67)); // 67 hz  <-- This is really cool

	}
	//Clean Up Data
	if (left.isValid()) { left.closePipe(); }
	if (right.isValid()) { right.closePipe(); }

	// close the hidapi library
	return hid_exit();
}
