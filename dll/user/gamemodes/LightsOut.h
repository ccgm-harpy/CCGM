#include "pch-il2cpp.h"
#include <string>
#include <map>

class LightsOut {
public:
	static void IncrementFlashTime(float dt);
	static bool lightState;
	static float flashTime;
	static float flashOnTime;
	static float flashDuration;
};