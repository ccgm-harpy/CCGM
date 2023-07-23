#include "pch-il2cpp.h"
#include "LightsOut.h"
#include "Server.h"
#include "Mod.h"

bool LightsOut::lightState = false;
float LightsOut::flashTime = 0;
float LightsOut::flashOnTime = Server::second * 5;
float LightsOut::flashDuration = Server::second * 2;

void LightsOut::IncrementFlashTime(float dt) {
	if (Server::GameMode == 7) {
		LightsOut::flashTime += dt;

		if (LightsOut::flashTime >= LightsOut::flashOnTime) {
			if (LightsOut::flashTime < LightsOut::flashOnTime + LightsOut::flashDuration) {
				if (!LightsOut::lightState) {
					LightsOut::lightState = true;
					Mod::ToggleLights(LightsOut::lightState);
				}
			}
			else {
				LightsOut::flashTime = 0;
				if (LightsOut::lightState) {
					LightsOut::lightState = false;
					Mod::ToggleLights(LightsOut::lightState);
				}
			}
		}
	}
}