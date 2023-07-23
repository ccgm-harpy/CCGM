#pragma once

#include "pch-il2cpp.h"

class Tips {
public:
	static void IncrementTipTime(float dt);
	static void SendTip();
	static void SendInsult();

	static bool addExtraLoadTime;
	static int tipServerIndex;
	static float tipTime;
	static bool incrementTipTime;
	static int tipIndex;
	static float requiredPlayersMessageTime;
};