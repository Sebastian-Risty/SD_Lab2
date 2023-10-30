#pragma once
#include "common.h"

class globals {
public:
	HANDLE hSerial = INVALID_HANDLE_VALUE;

	enum struct MessageID : int16_t {
		NA = 0
	};

	MessageID currentID = MessageID::NA;	// update this + any other data needed before calling writeData()

	bool connected = false;  // true when a BT connection is established

	string phoneNumber = "8154747480";
	string carrier = "txt.att.net";

	bool alertsEnabled = true;
};

inline globals g_globals;