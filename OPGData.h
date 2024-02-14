#pragma once

#include <array>

// OpenGlove data structures
typedef struct OpenGloveInputData
{
	std::array<std::array<float, 4>, 5> flexion;
	std::array<float, 5> splay;
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

#pragma pack(push, 1)
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
#pragma pack(pop)
