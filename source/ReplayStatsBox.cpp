#include "ReplayStatsBox.h"
#include "bakkesmod\wrappers\includes.h"
#include "bakkesmod\wrappers\CanvasWrapper.h"
#include "bakkesmod\wrappers\StructArrayWrapper.h"
#include "bakkesmod\wrappers\ReplayServerWrapper.h"
#include <string>

using namespace std;


/*
TO-DO
	- Fix matching local username to playerNames, but that would probably skip the player if it actually shows them :shrug:
*/

BAKKESMOD_PLUGIN(ReplayStatsBox, "Replay Stats Box", "0.9", PLUGINTYPE_REPLAY)

string playerStats[6] = {"Unknown", "0", "0", "0", "0", "0"};
string playerNames[8];
LinearColor boxColor = {0,0,0,0};

void ReplayStatsBox::onLoad()
{
	cvarManager->registerNotifier("rsbShowStats", [this](std::vector<string> params){rsbShow();}, "Displays stats box during replays", PERMISSION_ALL);
	cvarManager->registerNotifier("rsbHideStats", [this](std::vector<string> params){rsbHide();}, "Displays stats box during replays", PERMISSION_ALL);

	posX = make_shared<float>(2);
	posY = make_shared<float>(85);
	cvarManager->registerCvar("rsbPosX", "2", "Position of stat box X", true, true, 0, true, 100).bindTo(posX);
	cvarManager->registerCvar("rsbPosY", "85", "Position of stat box Y", true, true, 0, true, 100).bindTo(posY);

	rsbShow();
}
void ReplayStatsBox::onUnload(){}






void ReplayStatsBox::rsbShow()
{
	this->showStats = true;
	rsbUpdateVals();
	gameWrapper->UnregisterDrawables();
	gameWrapper->RegisterDrawable(bind(&ReplayStatsBox::rsbRender, this, std::placeholders::_1));
	this->drawablesAreRegistered = true;
}

void ReplayStatsBox::rsbHide()
{
	this->showStats = false;
}



void ReplayStatsBox::rsbUpdateVals()
{
	if(showStats)
	{
		if(gameWrapper->IsInReplay())
		{
			if(!drawablesAreRegistered)
			{
				gameWrapper->UnregisterDrawables();
				gameWrapper->RegisterDrawable(bind(&ReplayStatsBox::rsbRender, this, std::placeholders::_1));
				this->drawablesAreRegistered = true;
			}

			ReplayServerWrapper replayGame = gameWrapper->GetGameEventAsReplay();
			if(!replayGame.IsNull())
			{
				ArrayWrapper<CarWrapper> cars = replayGame.GetCars();
				if(cars.Count() > 0)
				{
					for(int i=0;i<cars.Count();i++)
						playerNames[i] = cars.Get(i).GetPRI().IsNull() ? 0 : cars.Get(i).GetPRI().GetPlayerName().ToString();
				}
			}

			PriWrapper specPRI = PriWrapper(reinterpret_cast<std::uintptr_t>(gameWrapper->GetCamera().GetViewTarget().PRI));
			this->isSpectating = false;
			if(!specPRI.IsNull())
			{
				for(int i=0;i<8;i++)
				{
					
					if(specPRI.GetPlayerName().ToString() == playerNames[i] && !specPRI.GetCar().IsNull())
					{
						this->isSpectating = true;

						int playerTeamNum = (int)(specPRI.GetTeamNum());
						if(playerTeamNum <= 1)
							boxColor = replayGame.GetTeams().Get(playerTeamNum).GetFontColor();

						playerStats[0] = specPRI.GetPlayerName().ToString();
						playerStats[1] = to_string(specPRI.GetMatchScore());
						playerStats[2] = to_string(specPRI.GetMatchGoals());
						playerStats[3] = to_string(specPRI.GetMatchAssists());
						playerStats[4] = to_string(specPRI.GetMatchSaves());
						playerStats[5] = to_string(specPRI.GetMatchShots());
					}
				}
			}
		}
		else
		{
			if(drawablesAreRegistered)
			{
				gameWrapper->UnregisterDrawables();
				this->drawablesAreRegistered = false;
			}
		}
	}
	else
	{
		gameWrapper->UnregisterDrawables();
		this->drawablesAreRegistered = false;
		return;
	}

	gameWrapper->SetTimeout(bind(&ReplayStatsBox::rsbUpdateVals, this), 0.0001f);
}




void ReplayStatsBox::rsbRender(CanvasWrapper canvas)
{
	int alphaVal;
	if(!isSpectating)//hide if not spectating
		alphaVal = 0;
	else
		alphaVal = 1;

	//Everything was explicitly positioned in 1920x1080 || math below converts from 1080 to other resolutions
	float resX = canvas.GetSize().X;
	float resY = canvas.GetSize().Y;
	float scaleX = resX / 1920;
	float scaleY = resY / 1080;

	float calcPosX = ((float)*posX/100)*(1920.0f-355.0f)+15;//maximum limit is right side of box
	float calcPosY = ((float)*posY/100)*(1080.0f-280.0f)+5;//maximum limit is bottom of box

	//DEFAULT FONT SIZES AND COMPENSATION VALUES AT 1920x1080
	int statNamesFont = 1;
	int watchingFont = 2;
	int statValuesFont = 3;
	int playerNameFont = 4;
	int playerNameCompX = 0;
	int playerNameCompY = 0;
	int watchingSizeComp = 0;
	int statValsSizeComp = 0;
	int charWidths[4] = {8,16,25,32};//widths of characters at different xScale,yScale values (1,2,3,4)
	int statValsCharWidth = charWidths[2];//subtract multiples of this to right-align values
	
	//at 1920x1080, default top left corner of colored box (not shadow): {40,720}

	//draw black "shadow" box
	canvas.SetColor(0, 0, 0, 255 * alphaVal);
	canvas.DrawRect({(int)((calcPosX-5)*scaleX),(int)((calcPosY-5)*scaleY)},{(int)((calcPosX+325)*scaleX),(int)((calcPosY+275)*scaleY)});//full black box ({35,715},{365,995})
	canvas.DrawRect({(int)((calcPosX-15)*scaleX),(int)((calcPosY+35)*scaleY)},{(int)((calcPosX+340)*scaleX),(int)((calcPosY+95)*scaleY)});//banner black box ({25,755},{380,815})

	
	//draw team colored box																														 
	canvas.SetColor(boxColor.R*255, boxColor.G*255, boxColor.B*255, boxColor.A*255 * alphaVal);
	canvas.DrawRect({(int)(calcPosX*scaleX),(int)(calcPosY*scaleY)},{(int)((calcPosX+320)*scaleX),(int)((calcPosY+270)*scaleY)});//({40,720},{360,990})

	
	
	//draw "banner" box and text
	Vector2 bannerTopLeft = {(int)((calcPosX-10)*scaleX),(int)((calcPosY+40)*scaleY)};
	Vector2 bannerBottomRight = {(int)((calcPosX+335)*scaleX),(int)((calcPosY+90)*scaleY)};
	canvas.SetColor(225, 225, 225, 255 * alphaVal);
	canvas.DrawRect(bannerTopLeft,bannerBottomRight);//({30,760},{375,810})
	
	//-------------------------------		DETERMINE TEXT SCALE AND POSITIONING BASED ON NUMBER OF CHARACTERS		--------------------------------
	if(scaleX >= 1)
	{
		if((playerStats[0].size()*charWidths[3]) > (bannerBottomRight.X - bannerTopLeft.X))//(playerStats[0].size() > 10)
		{
			playerNameFont = 3;
			playerNameCompY = 9;
			if((playerStats[0].size()*charWidths[2]) > (bannerBottomRight.X - bannerTopLeft.X))//(playerStats[0].size() > 14)
			{
				playerNameFont = 2;
				playerNameCompX = 3;
				playerNameCompY = 15;
				if((playerStats[0].size()*charWidths[1]) > (bannerBottomRight.X - bannerTopLeft.X))//(playerStats[0].size() > 21)
				{
					playerNameFont = 1;
					playerNameCompX = 3;
					playerNameCompY = 22;
				}
			}
		}
	}
	//COMPENSATIONS BELOW 1920x1080
	if(scaleX < 1)
	{
		watchingFont = 1;
		statValuesFont = 1;
		watchingSizeComp = 5;
		statValsSizeComp = 15;
		statValsCharWidth = charWidths[0];
		playerNameFont = 3;
		playerNameCompY = -3;
		if((playerStats[0].size()*charWidths[2]) > (bannerBottomRight.X - bannerTopLeft.X))//(playerStats[0].size() > 9)
		{
			playerNameFont = 2;
			playerNameCompX = 3;
			playerNameCompY = 9;
			if((playerStats[0].size()*charWidths[1]) > (bannerBottomRight.X - bannerTopLeft.X))//(playerStats[0].size() > 14)
			{
				playerNameFont = 1;
				playerNameCompX = 4;
				playerNameCompY = 20;
				if(((playerStats[0].size()+1)*charWidths[0]) > (bannerBottomRight.X - bannerTopLeft.X))//(playerStats[0].size() > 28)
				{
					//split it and add a hyphen
					int splitLength = 0;
					bool haveSize = false;
					for(int i=0; i<playerStats[0].size();i++)
					{
						if(!haveSize && ((i+1)*charWidths[0]) > (bannerBottomRight.X - bannerTopLeft.X))
						{
							if(i>=4)
								splitLength = i-3;//always == 25 except when it randomly == 1 (happens at 29 characters and I think i == 29 too)
							haveSize = true;
						}
					}
					string playerNameBeginning = playerStats[0];
					string playerNameEnd = playerStats[0];
					int endChars = playerStats[0].size() - splitLength;
					if(splitLength != playerStats[0].size() && splitLength != 0)
					{
						playerNameBeginning.erase(splitLength, endChars);
						playerNameEnd.erase(0, splitLength);
						playerStats[0] = playerNameBeginning + "-\n" + playerNameEnd;
						playerNameCompX = 4;
						playerNameCompY = 8;
					}
				}
			}
		}
	}	
	
	canvas.SetPosition({(int)((calcPosX-5+playerNameCompX)*scaleX),(int)((calcPosY+35+playerNameCompY)*scaleY)});//({55,770})
	canvas.SetColor(0, 0, 0, 255 * alphaVal);
	canvas.DrawString(playerStats[0], playerNameFont, playerNameFont);//playerName
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------


	//draw stats text and WATCHING
	canvas.SetColor(225, 225, 225, 255 * alphaVal);
	canvas.SetPosition({(int)((calcPosX+15)*scaleX),(int)((calcPosY+5+watchingSizeComp)*scaleY)});//({55,735})
	canvas.DrawString("WATCHING", watchingFont, watchingFont);
	canvas.SetPosition({(int)((calcPosX+15)*scaleX),(int)((calcPosY+110)*scaleY)});//({55,830})
	canvas.DrawString("SCORE", statNamesFont, statNamesFont);
	canvas.SetPosition({(int)((calcPosX+15)*scaleX),(int)((calcPosY+140)*scaleY)});//({55,860})
	canvas.DrawString("GOALS", statNamesFont, statNamesFont);
	canvas.SetPosition({(int)((calcPosX+15)*scaleX),(int)((calcPosY+170)*scaleY)});//({55,890})
	canvas.DrawString("SHOTS", statNamesFont, statNamesFont);
	canvas.SetPosition({(int)((calcPosX+15)*scaleX),(int)((calcPosY+200)*scaleY)});//({55,920})
	canvas.DrawString("ASSISTS", statNamesFont, statNamesFont);
	canvas.SetPosition({(int)((calcPosX+15)*scaleX),(int)((calcPosY+230)*scaleY)});//({55,950})
	canvas.DrawString("SAVES", statNamesFont, statNamesFont);


	//Stat values positioning based upon string length to be right-aligned
	canvas.SetPosition({(int)(((calcPosX+290)*scaleX)-(statValsCharWidth*(playerStats[1].size()-1))),(int)((calcPosY+95+statValsSizeComp)*scaleY)});//({330,825})
	canvas.DrawString(playerStats[1], statValuesFont, statValuesFont);//score
	canvas.SetPosition({(int)(((calcPosX+290)*scaleX)-(statValsCharWidth*(playerStats[2].size()-1))),(int)((calcPosY+125+statValsSizeComp)*scaleY)});//({330,855})
	canvas.DrawString(playerStats[2], statValuesFont, statValuesFont);//goals
	canvas.SetPosition({(int)(((calcPosX+290)*scaleX)-(statValsCharWidth*(playerStats[5].size()-1))),(int)((calcPosY+155+statValsSizeComp)*scaleY)});//({330,885})
	canvas.DrawString(playerStats[5], statValuesFont, statValuesFont);//shots
	canvas.SetPosition({(int)(((calcPosX+290)*scaleX)-(statValsCharWidth*(playerStats[3].size()-1))),(int)((calcPosY+185+statValsSizeComp)*scaleY)});//({330,915})
	canvas.DrawString(playerStats[3], statValuesFont, statValuesFont);//assists
	canvas.SetPosition({(int)(((calcPosX+290)*scaleX)-(statValsCharWidth*(playerStats[4].size()-1))),(int)((calcPosY+215+statValsSizeComp)*scaleY)});//({330,945})
	canvas.DrawString(playerStats[4], statValuesFont, statValuesFont);//saves
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	return;
}