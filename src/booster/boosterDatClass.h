#ifndef TH075BOOSTER_DAT
#define TH075BOOSTER_DAT

#include <windows.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include "conf.h"
#include "zlib.h"
#include "value.h"
#include "../mainDatClass.h"
#include "scriptClass.h"
using namespace std;

#define _PAGE	5	//myGameInfoÇÃÉyÅ[ÉWêî

class boosterDatClass{
	public:
	DWORD	PID;
	HWND	hWnd;
	HANDLE hProcess;
	INPUT Input;

	DWORD	baseAddress;
	DWORD	myBase;
	DWORD	myName;
	DWORD	myIniOffset;
	DWORD	myID;
	char	Name[16];
	char	NameTemp[16];
	char	enName[16];
	char	enNameTemp[16];
	DWORD	gameTimeAddress;
	DWORD	gameTime;
	DWORD	gameTimeTemp;
	DWORD	gameMode;
	DWORD	AIMode;
	DWORD	playerSide;

	DWORD	battleFlg;
	BOOL	bodyIniFlg;
	DWORD	secondaryModeFlg;
	DWORD	boosterDatInitFlg;
	DWORD	intervalFlg;

	BYTE*	AI;
	char	bodyBuf[bodyBuf_size];
	char	myBuf[myBuf_size];
	DWORD	bufSize;

	DWORD	gameInfoIni[10][6];
	DWORD	gameInfoInput[10][3];
	DWORD	gameInfoPara[20][3];
	DWORD	gameInfoImpress[10][3];

	float	floatArray[10];
	short	shortArray[10];
	BYTE	statusIDArray[800];
	BYTE	statusArray[256][3];
	BYTE	statusID;
	ULONGLONG commandArray[256];
	BYTE	commandInput[20];
	DWORD	inputTime;
	DWORD	decodeTime;
	DWORD	inputResetTime;
	BYTE*	AIbase;
	BYTE*	SNAIbase;
	BYTE*	SWAIbase;
	BYTE*	LNAIbase;
	BYTE*	SpellAIbase;
	BYTE*	RecoverAIbase;
	BYTE*	BackAIbase;
	BYTE*	LocalAIbase;
	DWORD	AIsizeArray[8];

	DWORD	eigenValueSN[10][4];
	DWORD	eigenValueSW[10][4];
	DWORD	eigenValueSpell[10][4];
	DWORD	eigenValueLN[10][4];
	DWORD	eigenValueRecover[10][4];
	DWORD	eigenValueBack[10][4];
	DWORD	eigenValueLocal[10][4];

	DWORD	LNAIBuf[20][10][20];
	DWORD	SNAIBuf[20][10][20];	//é¿å±íÜ
	DWORD	SWAIBuf[20][10][20];	//é¿å±íÜ
	DWORD	SpellAIBuf[10][5][20];	//é¿å±íÜ
	DWORD RecoverAIBuf[5];
	DWORD	BackAIBuf[10][6][20];	//é¿å±íÜ

	DWORD	Flg;
	DWORD	Line;
	DWORD	Index;
	DWORD	Counter;
	DWORD	Counter2;
	DWORD	Counter3;
	DWORD	Counter4;
	BYTE*	Address;
	BYTE*	Address2;
	BYTE*	Address3;
	BYTE*	Address4;
	DWORD Time;
//	DWORD	Temp[10];

	char	baseAIname[16];
	char	spellAIname[16];
	char	backAIname[16];
	char	individualAImyName[16];
	char	individualAIenName[16];
	char	localAIname[16];

	DWORD  myGameInfo[ _PAGE ][10][5];
	DWORD* enGameInfo[ _PAGE ][10][5];

// -- practice stuff begins here
	WORD practiceMode;
	WORD practiceSetMove;
	WORD practiceSetMoveCommand;
	WORD practiceJump;
	WORD practiceTiming;
	WORD practiceTeching;
	WORD practiceGuard;
	WORD practiceState;
	WORD practiceLastState;
	WORD practiceLastState2;
	WORD practiceLastMove;
	WORD practiceLastTech;
	bool practiceDeclareFlag;
	bool practiceUpdateFlag;
	int practiceTimer;
	int practiceTimer2;
// -- practice dummy stuff ends here


	int GetName();
	int OpenAI(char*);
	int OpenSpellAI(char*);
	int OpenBackAI(char*);
	int OpenIndividualAI(char*,char*);
	int OpenLocalAI(char*);
	int CloseAI();
	int CloseSpellAI();
	int CloseBackAI();
	int CloseIndividualAI();
	int CloseLocalAI();
	void CalcAddress();
	int ConvertIni();
	void statusInit();

	void CallSNAI();
	void ReadSNAI();
	void CallLNAI();
	void ReadLNAI();
	void CallSpellAI();
	void ReadSpellAI();
	void CallSWAI();
	void ReadSWAI();
	void CallRecoverAI();
	void ReadRecoverAI();
	void CallBackAI();
	void ReadBackAI();
	void ManageAI();


	boosterDatClass(WORD);
	~boosterDatClass();
	int boosterDatInit();
	int RefleshDat();
	void ConvertDat();
	void mainRoop();
	void InputCommand();

	boosterDatClass* enDat;

	WORD listeningMode;
	DWORD gameTimeTemp2;
	mainDatClass* casterDat;

	scriptClass script;
	WORD scriptFlg;
};

#endif
