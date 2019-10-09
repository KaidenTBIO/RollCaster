#ifndef CASTER_MAINDAT
#define CASTER_MAINDAT

#if _MSC_VER >= 1300
#	if !defined(_WINDOWS_)
#		if !defined(NO_WIN32_LEAN_AND_MEAN)
#		define WIN32_LEAN_AND_MEAN
#		define  _VCL_LEAN_AND_MEAN
#		endif
#	endif
#	include <windows.h>
#	include <mmsystem.h>
#	include <process.h>
#	include <winsock2.h>
#	include <time.h>
#	include <io.h>

#	include <iostream>
#	include <iomanip>
#	include <fstream>
#else
#	include <windows.h>
#	include <process.h>
#	include <winsock2.h>
#	include <time.h>
#	include <io.h>

#	include <iostream.h>
#	include <iomanip.h>
#	include <fstream.h>
#endif

#include "charDatClass.h"
#include "inputDataClass.h"
#include "syncDataClass.h"
#include "etc.h"
#include "const.h"

class mainDatClass{
	public:

	PROCESS_INFORMATION pi;
	BYTE bodyOldCode[14];
	HANDLE hProcess;
	HANDLE hProcessTh;
	DWORD  processThID;

	LPVOID pStackBase;

	WSADATA wsaData;
	SOCKET s;
	SOCKADDR_IN Here;
	SOCKADDR_IN Away;
	SOCKADDR_IN Root;
	SOCKADDR_IN Branch;
	SOCKADDR_IN subBranch;
	SOCKADDR_IN Leaf[4];
	SOCKADDR_IN Access;
	SOCKADDR_IN Ready;
	SOCKADDR_IN Standby[2];
	SOCKADDR_IN Remote;
	bool hasRemote;

	HANDLE hMutex;
	HANDLE hPrintMutex;
	HANDLE hRecvTh;
	HANDLE hSendTh;
	HANDLE hSendEvent;
	HANDLE hCheckEvent;
	HANDLE hTh075Th;
	HANDLE hManageTh;
	HANDLE hBoosterTh;

	DWORD nowTime;
	lastTimeStruct lastTime;

	sTaskClass sTask[50];

	charDatClass datA;
	charDatClass datB;

	int mainDatInit(int, char**);
	int mainDatEnd();
	int charInit();
	int charEnd();

	int backGroundRoop();
	int mainRoop();
	int ManipMenu();
	int Entrance();
	int WaitForStart();
	int WaitForMenu();
	int mainStep();
	int th075Roop(DWORD*);

	int UnRockTime();
	int RockTime();
	int SetBodyBreakPoint();
	int RemoveBodyBreakPoint();

	int SetCode();
	int RemoveCode();
	int TerminateTh075();
	int th075Quit();

	int WriteCode(void*, BYTE);
	int ReadMemory(void*, void*, DWORD);
	int WriteMemory(void*, void*, DWORD);

	int SendData(int, void*, DWORD);
	int SendData(SOCKADDR_IN*, void*, DWORD);
	int SendDataR(int, void*, DWORD);
	int SendDataR(SOCKADDR_IN*, void*, DWORD);
	int SendDataSub(int, void*, DWORD, WORD);
	int SendDataSub(SOCKADDR_IN*, void*, DWORD, WORD);

	int SendData(int, void*, DWORD, void*, DWORD);
	int SendData(SOCKADDR_IN*, void*, DWORD, void*, DWORD);
	int SendDataR(int, void*, DWORD, void*, DWORD);
	int SendDataR(SOCKADDR_IN*, void*, DWORD, void*, DWORD);
	int SendDataSub(int, void*, DWORD, void*, DWORD, WORD);
	int SendDataSub(SOCKADDR_IN*, void*, DWORD, void*, DWORD, WORD);

	int SendArea(int, void*, DWORD);
	int SendArea(SOCKADDR_IN*, void*, DWORD);
	int SendAreaR(int, void*, DWORD);
	int SendAreaR(SOCKADDR_IN*, void*, DWORD);
	int SendAreaSub(int, void*, DWORD, WORD);
	int SendAreaSub(SOCKADDR_IN*, void*, DWORD, WORD);

	int SendCmd( int, BYTE );
	int SendCmd( SOCKADDR_IN*, BYTE );
	int SendCmdR( int, BYTE );
	int SendCmdR( SOCKADDR_IN*, BYTE );

	int SendCmd( int, BYTE, void*, DWORD );
	int SendCmd( SOCKADDR_IN*, BYTE, void*, DWORD );
	int SendCmdR( int, BYTE, void*, DWORD );
	int SendCmdR( SOCKADDR_IN*, BYTE, void*, DWORD );
	int SendCmdR( int, BYTE, void*, DWORD, void*, DWORD );
	int SendCmdR( SOCKADDR_IN*, BYTE, void*, DWORD, void*, DWORD );
	int SendDataR(int, void*, DWORD, void*, DWORD, void*, DWORD);
	int SendDataR(SOCKADDR_IN*, void*, DWORD, void*, DWORD, void*, DWORD);
	int SendDataSub(int, void*, DWORD, void*, DWORD, void*, DWORD, WORD);
	int SendDataSub(SOCKADDR_IN*, void*, DWORD, void*, DWORD, void*, DWORD, WORD);

	int SendCmdM( int, BYTE );
	int SendCmdM( SOCKADDR_IN*, BYTE );

	WORD roopFlg;
	WORD th075Flg;
	WORD continueFlg;
	WORD rockFlg;

	BYTE Ahidden;
	BYTE Bhidden;

	gameInfoStruct myInfo;
	gameInfoStruct enInfo;
	gameInfoStruct dataInfo;

	int GetEsc();
	int GetMyPort();
	int GetPlayerSide();
	int GetDelay( BYTE, float* );
	int Communicate();

	WORD delayTime;
	WORD lastDelayTime;
	WORD myPort;
	WORD accessFlg;

	BYTE enRandNo;
	BYTE enRand;
	BYTE myRandNo;
	BYTE myRand;
	WORD delayTimeObsNo;
	float delayTimeObs[5];
	WORD playerSideRes;
	BYTE myInitFlg;
	BYTE enInitFlg;
	BYTE joinRes;
	echoFlgStruct echoFlg;

	int enCowCaster;

	int WaitForIni();
	int WaitForRoopFlg();

	inputDataClass inputData;
	float delay;
	time_t startTime;

	int SetRandBreakPoint();
	int RemoveRandBreakPoint();

	int SetWindowResize();

	int SetCharBreakPoint();
	int RemoveCharBreakPoint();
	BYTE charOldCode;
	WORD toughModeFlg;

	int SetRoopBreakPoint();
	int RemoveRoopBreakPoint();

	int SetAutoSave();
	int RemoveAutoSave();

	WORD autoNextFlg;

	WORD deInitFlg;
	DEBUG_EVENT de;
	DWORD ContinueStatus;

	WORD delayTimeA;
	WORD delayTimeB;

	int TestPort( SOCKADDR_IN* );
	WORD testPortFlg;

	WORD stdbyPort[2];
	WORD readyPort;

	WORD mainFlg;
	WCHAR iniPath[200];
	WORD enPort;
	WORD beepFlg;
	WORD waveFlg;
	unsigned long waitTargetIP;
	WORD endTimeFlg;
	WORD escSelectFlg;
	WORD lessCastFlg;
	WORD th075QuitFlg;
	WORD zlibFlg;
	WORD priorityFlg;
	WORD windowModeFlg;
	WORD stageLimitCancelFlg;
	WORD perfectFreezeFlg;
	WORD boosterOnFlg;
	WORD boosterFlg;
	WORD practiceNoSpecFlg;

	WORD practiceLastMode;
	WORD practiceMode;
	WORD practiceJump;
	WORD practiceSetMove;
	WORD practiceTiming;
	WORD practiceTeching;
	WORD practiceGuard;

	BYTE lastCharacterA;
	BYTE lastCharacterB;

	WORD obsCountFlg;
	DWORD obsIP[64];
	UINT uMsg;
	BYTE boosterInput;
	BYTE targetMode;
	WCHAR nowDir[200];
	WORD autoWaitFlg;
	WORD autoWaitOnFlg;
	WORD replaySaveFlg;
	WORD playerSideFlg;
	WORD playerSideHostFlg;
	WORD autoSaveFlg;
	WORD autoSaveReplayNameFlg;
	WORD autoSaveReplayFileRenameFlg;
	WORD replayMetadataSaveFlg;
	WORD unknownNameFlg;
	WORD allowObsFlg;
	WORD windowTopFlg;
	WORD deSyncSoundFlg;
	WORD roundShowFlg;
	WORD sessionLogFlg;
	WORD dontReportRemoteFlg;
	WORD boosterWarningsFlg;
	WORD systemTimeFlg;
	WORD processRemotePalettesFlg;
	WORD noFastFlg;
	WORD bgmOffFlg;
	WORD anonymousObsFlg;
	WORD disableScreensaverFlg;
	WORD windowResizeFlg;
	WORD windowDisableTransitionsFlg;
	WORD windowFilterState;
	DWORD windowWidth;
	DWORD windowHeight;

	float windowMultiplier;

	WORD winsA;
	WORD roundsWonA;
	WORD winsB;
	WORD roundsWonB;

	WORD lastMode;

	WORD toggleWindowTopFlg;

	WORD keybinds[KEY_COUNT];
	BYTE keystate[KEY_COUNT];

	void runKeybind(int key);
	void updateKeybinds();

	argDataStruct argData;
	WORD accessPort;
	char accessIP[80];
	char standbyIP[80];
	syncDataClass syncData;

	ofstream sessionLogFile;

	char windowName[160];

	HANDLE hInputRecvEvent;
	WORD portSeekFlg;
	WORD floatControlFlg;
	WORD newCWflg;

	WORD myVisiblePort;

	char myPlayerName[16];
	char myShortName[5];

	char p1PlayerName[16];
	char p1ShortName[5];
	bool p1TempName;
	char p2PlayerName[16];
	char p2ShortName[5];
	bool p2TempName;
	int nPlayers;
	bool namesLocked;

	DWORD lastGameTime;
	DWORD totalGameTime;

	bool replaySavedFlg;
	char replayFilename[80];
	WCHAR replayFilenameFormat[200];

	paletteEntryStruct palettes[(11*4*2) + 2];
	int paletteCounter;
	int paletteNo[2];

	WORD bgmToggleMode;
	WORD bgmOrigVolume;

	HWND hWnd;
	
	void runReplayMetadata();
	void runAutoSaveRename();
	void cleanString(char *string, bool spaces_okay);
	const char *getCharacterName(int ch);
	const char *getCharacterShortName(int ch);
	void loadPalette(int ch, int n, int player);
	void requestPaletteInfo(int color);
	void runPaletteUpdate(DWORD addr);
	bool getPaletteData(int player_no, unsigned int *dest);

	void printDate(ostream *stream);
	void printTime(ostream *stream);

	bool useth075eDLL;
	
	bool wineFlg;
	
	int dllHookState;
	
	int hookDLLCode(const WCHAR *name, bool useNowDir);
	int hookEnglishDLL();
	int hookStateDLL();

	void iniReadStringNonUnicode(const WCHAR *section, const WCHAR *name,
				const WCHAR *default_value, char *dest, int len,
				const WCHAR *path);
	
	WORD rollbackFlg;
	WORD rollbackOk;
	DWORD rollbackRewindAddr;
	DWORD rollbackLockModeAddr;
	
	DWORD rollbackMinInputDelay;
	DWORD rollbackMaxRewind;
	
	void getHookAddresses();
	void calcRewind(DWORD delay, DWORD *newDelayTime, DWORD *newRewindAmount);
	
	WORD humanTimeFlg;
	WORD origMenuFlg;
	WORD acceptRecommendedDelayFlg;
	
	DWORD localDelayTime;
	DWORD rewindAmount;
	
	bool isRollIni;
};

#endif
