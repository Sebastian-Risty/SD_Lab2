#pragma once
#include "globals.h"

const int16_t SOM_MARKER = -32768;
const int16_t EOM_MARKER = -32767;

struct CommandData {

	globals::MessageID id = globals::MessageID::NA;
	int16_t data = -1;
};

bool readData();

bool initPair(bool isReconnect = false);

bool writeData();

wstring findSerial(BLUETOOTH_ADDRESS addr);