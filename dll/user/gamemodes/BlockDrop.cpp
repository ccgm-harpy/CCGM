#include "pch-il2cpp.h"
#include "BlockDrop.h"
#include "Server.h"
#include "Chat.h"
#include <nlohmann/json.hpp>
#include "Mod.h"
#include <string>
#include "Win.h"
#include "HTTP.h"

using namespace nlohmann;

// The coords interally for blockdrop are strange
// This mapping is used to map 1 - 16 to these coords so we can drop blocks easier
std::map<int, int>blockDropCoords = {
		{1, 14},
		{2, 3},
		{3, 13},
		{4, 2},

		{5, 16},
		{6, 5},
		{7, 15},
		{8, 4},

		{9, 18},
		{10, 7},
		{11, 19},
		{12, 8},

		{13, 1},
		{14, 11},
		{15, 0},
		{16, 10},
};

bool BlockDrop::stopAtEnd = false;
bool BlockDrop::enabled = false;
bool BlockDrop::sendingBlock = false;
int BlockDrop::index = 0;
int BlockDrop::frameIndex = 0;
float BlockDrop::sendTime = 1;
json animation = json::parse("{}");

void BlockDrop::Stop() {
	BlockDrop::enabled = false;

	if (BlockDrop::stopAtEnd) {
		BlockDrop::stopAtEnd = false;
		Mod::SendModeState(2);
	}

	BlockDrop::sendingBlock = false;
	BlockDrop::index = 0;
	BlockDrop::frameIndex = 0;
	BlockDrop::sendTime = 1;
}

void BlockDrop::ChanceStart(int percentChance) {
	if (rand() % 100 + 1 <= percentChance) {
		BlockDrop::Start();
	}
}

void BlockDrop::Start() {
	BlockDrop::RequestBlockDropAnimation();
}

void BlockDrop::RequestBlockDropAnimation() {
	animation["animation"] = json::array();

	std::cout << "REQUESTION BLOCK DROP ANIMATION" << std::endl;

	try {
		json _animation = HTTP::LoadBlockDropAnimation();

		for (auto& _frame : _animation["animation"].items()) {
			auto frame = _frame.value();
			animation["animation"].push_back(frame);
		}
		BlockDrop::enabled = true;

		Chat::SendServerMessage("!!! YOU MUST SURVIVE THE ENTIRE SEQUENCE !!!");
	}
	catch (...) {
		Chat::SendServerMessage("Tried to start sequence but no sequence matched player count!");
		BlockDrop::enabled = false;
	}
}

void BlockDrop::IncrementSendTime(float dt) {
	if (enabled) {
		sendTime += dt;

		float animationTime = 500;

		if (index <= animation["animation"].size() - 1) {
			if (frameIndex <= animation["animation"][index]["coords"].size() - 1) {
				animationTime = animation["animation"][index]["time"].get<float>();
				float iterationTime = animation["animation"][index]["iteration_time"].get<float>();

				if (sendTime >= animationTime + iterationTime*(frameIndex+1)) {
					auto speed = animation["animation"][index]["speed"].get<float>();
					auto coords = animation["animation"][index]["coords"][frameIndex];

					for (int i = 0; i < animation["animation"][index]["coords"][frameIndex].size(); i++) {
						auto coord = animation["animation"][index]["coords"][frameIndex][i].get<int>();
						Mod::SendFallingBlocks(speed, blockDropCoords[coord]);
					}

					frameIndex += 1;

					if (index+1 > animation["animation"].size()-1 && frameIndex > animation["animation"][index]["coords"].size()-1) {
						sendingBlock = true;
						sendTime = 1;
					}
				}
			}
			else {
				frameIndex = 0;
				index += 1;
			}
		}
		else {
			BlockDrop::Stop();
		}
	}
	else {
		sendTime = 1;
	}
}