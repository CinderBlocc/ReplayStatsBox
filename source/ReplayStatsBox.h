#pragma once
#pragma comment(lib, "BakkesMod.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"




class ReplayStatsBox : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	std::shared_ptr<float> posX;
	std::shared_ptr<float> posY;
public:
	virtual void onLoad();
	virtual void onUnload();

	void rsbShow();
	void rsbHide();
	void rsbUpdateVals();
	void rsbRender(CanvasWrapper canvas);

	bool showStats = false;
	bool isSpectating = false;
	bool drawablesAreRegistered = false;
};