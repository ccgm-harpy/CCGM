#include "pch-il2cpp.h"

class ReadyUp {
public:
	static bool sendRequiredPlayersMessage;
	static bool sentRequiredPlayersMessage;
	static void IncrementAutoStart(float dt);
	static void StartGame();
	static void ResetTimers();
};