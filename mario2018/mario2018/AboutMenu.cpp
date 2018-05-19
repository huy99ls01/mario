#include "AboutMenu.h"
#include "CFG.h"
#include "Core.h"
#include "stdlib.h"
#include "time.h"

/* ******************************************** */

AboutMenu::AboutMenu(void) {
	lMO.push_back(new MenuOption("MAIN MENU", 150, 340));

	this->numOfMenuOptions = lMO.size();

	this->cR = 93;
	this->cG = 148;
	this->cB = 252;
	this->nR = 0;
	this->nG = 0;
	this->nB = 0;
	this->colorStepID = 1;
	this->iColorID = 0;

	this->moveDirection = true;

	this->iNumOfUnits = 0;

	srand((unsigned)time(NULL));
}


AboutMenu::~AboutMenu(void) {

}

/* ******************************************** */

void AboutMenu::Update() {
	if (SDL_GetTicks() >= iTime + 35) {
		this->cR = getColorStep(cR, nR);
		this->cG = getColorStep(cG, nG);
		this->cB = getColorStep(cB, nB);

		if (colorStepID >= 15 || (cR == nR && cG == nG && cB == nB)) {
			nextColor();
			colorStepID = 1;
		}
		else {
			++colorStepID;
		}

		CCore::getMap()->setLevelType(rand() % 4);

		if (rand() % 10 < 6) {
			CCore::getMap()->addGoombas(-(int)CCore::getMap()->getXPos() + rand() % (CCFG::GAME_WIDTH + 128), -32, rand() % 2 == 0);
			CCore::getMap()->addGoombas(-(int)CCore::getMap()->getXPos() + rand() % (CCFG::GAME_WIDTH + 128), -32, rand() % 2 == 0);
		}


		iTime = SDL_GetTicks();
	}

	if (moveDirection && CCFG::GAME_WIDTH - CCore::getMap()->getXPos() >= (CCore::getMap()->getMapWidth() - 20) * 32) {
		moveDirection = !moveDirection;
	}
	else if (!moveDirection && -CCore::getMap()->getXPos() <= 0) {
		moveDirection = !moveDirection;
	}

	CCore::getMap()->setXPos(CCore::getMap()->getXPos() + 4 * (moveDirection ? -1 : 1));
}

void AboutMenu::Draw(SDL_Renderer* rR) {
	CCFG::getText()->DrawWS(rR, "PETER HAO - C++ and SDL2", 150, 128, 0, 0, 0);
	CCFG::getText()->DrawWS(rR, "AUTOR: TRAN QUANG HUY ", 150, 146, 0, 0, 0);

	CCFG::getText()->DrawWS(rR, "SOURCES: SPRITERS-RESOURCE.COM/NES/SUPERMARIOBROS", 150, 188, 0, 0, 0);

	CCFG::getText()->DrawWS(rR, "FB.COM/HUY.TRANHUY.77", 150, 264, 0, 0, 0);



	for (unsigned int i = 0; i < lMO.size(); i++) {
		CCFG::getText()->DrawWS(rR, lMO[i]->getText(), lMO[i]->getXPos(), lMO[i]->getYPos(), 0, 0, 0);
	}

	CCFG::getMM()->getActiveOption()->Draw(rR, lMO[activeMenuOption]->getXPos() - 32, lMO[activeMenuOption]->getYPos());
}

/* ******************************************** */

void AboutMenu::enter() {
	CCFG::getMM()->resetActiveOptionID(CCFG::getMM()->eMainMenu);
	CCFG::getMM()->setViewID(CCFG::getMM()->eMainMenu);
	reset();
	CCFG::getMusic()->StopMusic();
}

/* ******************************************** */

void AboutMenu::launch() {
	this->cR = 93;
	this->cG = 148;
	this->cB = 252;
}

void AboutMenu::reset() {
	CCore::getMap()->setXPos(0);
	CCore::getMap()->loadLVL();
}

/* ******************************************** */

void AboutMenu::nextColor() {
	int iN = iColorID;

	while (iN == iColorID) {
		iColorID = rand() % 16;
	}

	++iColorID;

	switch (iColorID) {
	case 0:
		nR = 73;
		nG = 133;
		nB = 203;
		break;
	case 1:
		nR = 197;
		nG = 197;
		nB = 223;
		break;

	}
}

int AboutMenu::getColorStep(int iOld, int iNew) {
	return iOld + (iOld > iNew ? (iNew - iOld) * colorStepID / 30 : (iNew - iOld) * colorStepID / 30);
}

/* ******************************************** */

void AboutMenu::setBackgroundColor(SDL_Renderer* rR) {
	SDL_SetRenderDrawColor(rR, cR, cG, cB, 255);
}

void AboutMenu::updateTime() {
	this->iTime = SDL_GetTicks();
}