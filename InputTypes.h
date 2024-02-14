#pragma once

// Pulse data structures
#pragma pack(push, 1)
typedef struct FingerData
{
	//unsigned short(2 bytes we are bitfielding to prevent data overflow) from Pulse glove report buffer using bitfields to define the incoming data bitsize
	unsigned short pull : 14;
	unsigned short splay : 10;
} FingerData;
typedef struct FingerInputData
{
	//Grab data as native unsigned char[3] from Pulse glove report buffer 
	unsigned char InputBuffer[3];

} FingerInputData;
typedef struct GloveInputReport
{
	unsigned char reportId : 8;
	FingerInputData thumb, index, middle, ring, pinky;
} GloveInputReport;

union HIDBuffer
{
	GloveInputReport glove;
	unsigned char buffer[sizeof(glove)];
};
#pragma pack(pop)
