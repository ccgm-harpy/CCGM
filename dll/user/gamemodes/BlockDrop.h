#include "pch-il2cpp.h"
#include <string>
#include <map>
#include <nlohmann/json.hpp>

using namespace nlohmann;

class BlockDrop {
public:
	static int index;
	static int frameIndex;
	static float sendTime;
	static bool enabled;
	static bool sendingBlock;
	static bool stopAtEnd;
	
	static void ChanceStart(int percentChance);
	static void Stop();
	static void Start();
	static void RequestBlockDropAnimation();
	static void IncrementSendTime(float dt);
};