#pragma once

// Pulse data structures
#pragma pack(push, 1)
typedef struct FingerData
{
	std::array<uint8_t, 3> bitData;

	uint16_t getPull() const {
		uint16_t pull = ((bitData[0] & 0b11111111) << 6) | ((bitData[1] & 0b11111100) >> 2);
		return pull;
	}
	uint16_t getSplay() const {
		uint16_t splay = ((bitData[1] & 0b00000011) << 8) | (bitData[2] & 0b11111111);
		return splay;
	}
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
#pragma pack(pop)
