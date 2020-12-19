#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"

class ReplayStatsBox : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	std::shared_ptr<bool> enabled;
	std::shared_ptr<float> posX;
	std::shared_ptr<float> posY;
public:
	void onLoad() override;
	void onUnload() override;

	void rsbRender(CanvasWrapper canvas);
	bool rsbUpdateVals();

	bool isSpectating = false;
};