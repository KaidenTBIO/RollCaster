#include "mainDatClass.h"
#include <shlwapi.h>
using namespace std;

int mainDatClass::mainDatInit(int argc, char** argv){
	cout << "RollCaster version " << cowcaster_version_string << endl;
	//cout << ", protocol version " << (int)cowcaster_protocol_version << endl;
	//cout << "Command version : " << cmd_version << endl;
	startTime = time( NULL );

	//nowDir
	memset( nowDir, 0, sizeof(nowDir) );
	lstrcpyW( nowDir, L"fail\0" );
	{
		WCHAR dir[200];

		if( GetModuleFileNameW( NULL, dir, 200 ) ){
			if( PathRemoveFileSpecW( dir ) ){
				dir[199] = 0;
				lstrcpyW( nowDir, dir );

				//作業フォルダ設定
				SetCurrentDirectoryW( nowDir );
			}
		}

		dir[199] = 0;
	}

	//iniPath
	isRollIni = 1;
	lstrcpyW( iniPath, L"fail\0" );
	{
		WCHAR dir[200];
		if( lstrcmpW( nowDir, L"fail" ) && lstrlenW( nowDir ) < 180 ){
			//ok
			lstrcpyW( iniPath ,nowDir );
			lstrcatW( iniPath, L"\\config_rollcaster.ini\0" );
			if( GetFileAttributesW( iniPath ) == 0xffffffff ){
				lstrcpyW( iniPath ,nowDir );
				lstrcatW( iniPath, L"\\config_caster.ini\0" );
				isRollIni = 0;
			}
		}
	}

	//arg
	mainFlg = main_default;
	memset( &argData, 0, sizeof( argData ) );
	strcpy( argData.targetIP, "default\0" );
	{
		int Counter;
		char* arg;

		for(Counter = 0; Counter<argc; Counter++){
			arg = argv[Counter];
			if( *arg=='-' ){
				arg++;
				switch( *arg ){
				case 'i':
					if( lstrcmpW( iniPath, L"fail" ) ){
						if( GetPrivateProfileIntW( L"MAIN", L"allowArgMode", 1, iniPath ) ){;

							mainFlg = main_arg;
							argData.argMode = 0;

							//IP
							strncpy( argData.targetIP, arg + 1, sizeof( argData.targetIP ) );
						}
					}else{
						mainFlg = main_arg;
						argData.argMode = 0;

						//IP
						strncpy( argData.targetIP, arg + 1, sizeof( argData.targetIP ) );
					}
					break;
				case 'p':
					//port
					argData.targetPort = (WORD)atoi( arg + 1 );
					break;
				}
			}
		}
		for(Counter = 0; Counter<argc; Counter++){
			arg = argv[Counter];
			if( *arg=='-' ){
				arg++;
				switch( *arg ){
				case 'w' :
					mainFlg = main_arg;
					argData.argMode = 2;
					break;
				}
			}
		}
		for(Counter = 0; Counter<argc; Counter++){
			arg = argv[Counter];
			if( *arg=='-' ){
				arg++;
				switch( *arg ){
				case 'r':
					if( lstrcmpW( iniPath, L"fail" ) ){
						if( GetPrivateProfileIntW( L"MAIN", L"allowArgMode", 1, iniPath ) ){;

							mainFlg = main_arg;
							argData.argMode = 4;

							//IP
							strncpy( argData.targetIP, arg + 1, sizeof( argData.targetIP ) );
						}
					}else{
						mainFlg = main_arg;
						argData.argMode = 4;

						//IP
						strncpy( argData.targetIP, arg + 1, sizeof( argData.targetIP ) );
					}
					break;
				}
			}
		}
		for(Counter = 0; Counter<argc; Counter++){
			arg = argv[Counter];
			if( *arg=='-' ){
				arg++;
				switch( *arg ){
				case 's':
					if( lstrcmpW( iniPath, L"fail" ) ){
						if( GetPrivateProfileIntW( L"MAIN", L"allowArgMode", 1, iniPath ) ){;

							mainFlg = main_arg;
							argData.argMode = 3;

							//IP
							strncpy( argData.targetIP, arg + 1, sizeof( argData.targetIP ) );
						}
					}else{
						mainFlg = main_arg;
						argData.argMode = 3;

						//IP
						strncpy( argData.targetIP, arg + 1, sizeof( argData.targetIP ) );
					}
					break;
				}
			}
		}
		for(Counter = 0; Counter<argc; Counter++){
			arg = argv[Counter];
			if( *arg=='-' ){
				arg++;
				switch( *arg ){
				case 'f':
					if( lstrcmpW( iniPath, L"fail" ) ){
						if( GetPrivateProfileIntW( L"MAIN", L"allowFileMode", 1, iniPath ) ){;
							if( mainFlg == main_default ){
								mainFlg = main_file;
							}
						}
					}
					break;
				}
			}
		}
	}
	if( mainFlg == main_arg ){
		//arg mode
		cout << "debug : arg Mode" << endl;

		if( strcmp( argData.targetIP, "default" ) && argData.argMode == 2 ){
			//特定相手待ち
			argData.argMode = 1;
		}

	}else if( mainFlg == main_file ){
		//file mode
		cout << "debug : file Mode" << endl;
	}else if( mainFlg != main_file ){
		if( lstrcmpW( iniPath, L"fail" ) ){
			if( GetPrivateProfileIntW( L"MAIN", L"fileModeStart", 0, iniPath ) ){
				mainFlg = main_file;
				cout << "debug : file Mode" << endl;
			}
		}
	}

	datA.playerSide = 0xA;
	datB.playerSide = 0xB;
	myInfo.terminalMode = 0;
	continueFlg = 1;
	th075Flg = 0;
	roopFlg = 0;
	rockFlg = 0;
	toughModeFlg = 0;
	boosterFlg = 0;
	portSeekFlg = 0;
	newCWflg = 0;
	replaySavedFlg = 0;

	delayTime = 0;
	lastDelayTime = 0;

	practiceLastMode = 1;
	practiceMode = 1;
	practiceJump = 1;
	practiceGuard = 1;
	practiceSetMove = 1;
	practiceTiming = 1;
	practiceTeching = 0;

	hRecvTh = NULL;
	hSendTh = NULL;
	hTh075Th = NULL;
	hBoosterTh = NULL;

	memset( &myInfo, 0, sizeof(myInfo) );
	memset( &enInfo, 0, sizeof(enInfo) );
	memset( &dataInfo, 0, sizeof(dataInfo) );

	memset( &Here , 0, sizeof(SOCKADDR_IN) );
	memset( &Away , 0, sizeof(SOCKADDR_IN) );
	memset( &Root , 0, sizeof(SOCKADDR_IN) );
	memset( &Branch , 0, sizeof(SOCKADDR_IN) );
	memset( &subBranch , 0, sizeof(SOCKADDR_IN) );
	memset( &Access , 0, sizeof(SOCKADDR_IN) );

	memset( &Leaf[0] , 0, sizeof(SOCKADDR_IN) );
	memset( &Leaf[1] , 0, sizeof(SOCKADDR_IN) );
	memset( &Leaf[2] , 0, sizeof(SOCKADDR_IN) );
	memset( &Leaf[3] , 0, sizeof(SOCKADDR_IN) );

	memset( &Ready , 0, sizeof(SOCKADDR_IN) );
	memset( &Standby[0] , 0, sizeof(SOCKADDR_IN) );
	memset( &Standby[1] , 0, sizeof(SOCKADDR_IN) );

	memset( windowName, 0, sizeof(windowName) );
	strcpy( windowName, "東方萃夢想 ～ Immaterial and Missing Power. ver1.11" );

	memset( myPlayerName, 0, sizeof(myPlayerName) );
	memset( myShortName, 0, sizeof(myShortName) );
	memset( p1PlayerName, 0, sizeof(p1PlayerName) );
	memset( p1ShortName, 0, sizeof(p1ShortName) );
	memset( p2PlayerName, 0, sizeof(p2PlayerName) );
	memset( p2ShortName, 0, sizeof(p2ShortName) );

	memset( replayFilename, 0, sizeof(replayFilename) );

	lstrcpyW( replayFilenameFormat, L"%A vs %B %n" );

	ZeroMemory(&keybinds, sizeof(keybinds));
	ZeroMemory(&keystate, sizeof(keystate));

	keybinds[KEY_AUTOSAVE_OFF] = 189;               // '-'
	keybinds[KEY_AUTOSAVE_ON] = 187;                // '='
	keybinds[KEY_AUTONEXT_OFF] = 219;               // '['
	keybinds[KEY_AUTONEXT_ON] = 221;                // ']'
	keybinds[KEY_ROUNDCOUNT_CYCLE] = 220;           // '\'
	keybinds[KEY_BGM_TOGGLE] = 192;                 // '`'
	keybinds[KEY_NOFAST_TOGGLE] = 8;                // backspace

	keybinds[KEY_SAVE_PALETTE] = 222;               // '''

	keybinds[KEY_INFINITE_SPIRIT_CHEAT] = VK_F8;    // F8

	keybinds[KEY_REMOTE_PALETTE_CYCLE] = 222;       // '''

	keybinds[KEY_STAGE_FREE] = 8;                   // backspace
	keybinds[KEY_STAGE_MANUAL] = 8;                 // backspace

	keybinds[KEY_ALWAYSONTOP_TOGGLE] = 191;         // '/'

	keybinds[KEY_DELAY1] = '1';
	keybinds[KEY_DELAY2] = '2';
	keybinds[KEY_DELAY3] = '3';
	keybinds[KEY_DELAY4] = '4';
	keybinds[KEY_DELAY5] = '5';
	keybinds[KEY_DELAY6] = '6';
	keybinds[KEY_DELAY7] = '7';
	keybinds[KEY_DELAY8] = '8';
	keybinds[KEY_DELAY9] = '9';
	keybinds[KEY_DELAY10] = '0';
	
	keybinds[KEY_REWIND_MODIFIER] = 16;		// shift

	ZeroMemory(&pi, sizeof(pi));

	accessPort = 7500;
	strcpy( accessIP, "0.0.0.0" );
	strcpy( standbyIP, "mizuumi.net" );
	
	paletteNo[0] = -1;
	paletteNo[1] = -1;

	//uMsg
	uMsg = RegisterWindowMessage( umsg_string );

	// wine test
	HKEY hKey;
	
	wineFlg = 0;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Wine", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		wineFlg = 1;
		RegCloseKey(hKey);
	}

	//read ini
	myPort = 0;
	enPort = 7500;
	beepFlg = 0;
	waveFlg = 1;
	endTimeFlg = 0;
	escSelectFlg = 0;
	lessCastFlg = 1;
	th075QuitFlg = 0;
	zlibFlg = 1;
	perfectFreezeFlg = 0;
	obsCountFlg = 0;
	autoNextFlg = 0;
	boosterOnFlg = 0;
	autoWaitOnFlg = 0;
	autoWaitFlg = 0;
	replaySaveFlg = 0;
	playerSideFlg = 0;
	autoSaveFlg = 0;
	autoSaveReplayNameFlg = 0;
	autoSaveReplayFileRenameFlg = 0;
	replayMetadataSaveFlg = 0;
	allowObsFlg = 1;
	anonymousObsFlg = 0;
	windowTopFlg = 0;
	deSyncSoundFlg = 0;
	roundShowFlg = 0;
	sessionLogFlg = 0;
	practiceNoSpecFlg = 0;
	dontReportRemoteFlg = 0;
	boosterWarningsFlg = 0;
	unknownNameFlg = 0;
	systemTimeFlg = 0;
	processRemotePalettesFlg = 0;
	noFastFlg = 0;
	bgmOffFlg = 0;
	lastMode = 0;
	floatControlFlg = 1;
	windowWidth = 640;
	windowHeight = 480;
	windowFilterState = 2;
	windowDisableTransitionsFlg = 0;
	windowResizeFlg = 0;
	disableScreensaverFlg = 1;
	useth075eDLL = 1;
	
	rollbackFlg = 1;
	rollbackMinInputDelay = 0;
	rollbackMaxRewind = 3;
	
	humanTimeFlg = 0;
	origMenuFlg = 0;
	acceptRecommendedDelayFlg = 0;

	if( lstrcmpW( iniPath, L"fail" ) ){
		int Res;

		if( GetPrivateProfileIntW( L"PORT", L"onoff", 0, iniPath ) ){
			//myPort
			Res = GetPrivateProfileIntW( L"PORT", L"myPort", 0, iniPath );
			if( Res ){
				myPort = Res;
			}

			//enPort
			Res = GetPrivateProfileIntW( L"PORT", L"enPort", 0, iniPath );
			if( Res ){
				enPort = Res;
				accessPort = enPort;
			}
		}

		//beep
		Res = GetPrivateProfileIntW( L"SOUND", L"beep", 0, iniPath );
		if( Res ){
			beepFlg = 1;
		}

		//sound
		Res = GetPrivateProfileIntW( L"SOUND", L"wave", 0, iniPath );
		if( Res ){
			waveFlg = 1;
		}else{
			waveFlg = 0;
		}
		
		// ***** MAIN

		//th075Quit
		if( GetPrivateProfileIntW( L"MAIN", L"th075Quit", 0, iniPath ) ){
			th075QuitFlg = 1;
		}

		//autoNext
		if( GetPrivateProfileIntW( L"MAIN", L"autoNext", 0, iniPath ) ){
			autoNextFlg = 1;
		} else if (GetPrivateProfileIntW( L"MAIN", L"autoNextOn", 0, iniPath) ){
			// legacy
			autoNextFlg = 1;
		}

 		//roundShow
 		if( GetPrivateProfileIntW( L"MAIN", L"roundShow", 0, iniPath ) ){
 			roundShowFlg = GetPrivateProfileIntW( L"MAIN", L"roundShow", 0, iniPath );
 		}

 		//sessionLog
 		if( GetPrivateProfileIntW( L"MAIN", L"sessionLog", 0, iniPath ) ){
 			sessionLogFlg = GetPrivateProfileIntW( L"MAIN", L"sessionLog", 0, iniPath );
 		}

 		// bgmOffFlg
 		if( GetPrivateProfileIntW( L"MAIN", L"bgmOff", 0, iniPath ) ){
 			bgmOffFlg = GetPrivateProfileIntW( L"MAIN", L"bgmOff", 0, iniPath );
 		}

 		// disableScreensaver
 		disableScreensaverFlg = GetPrivateProfileIntW( L"MAIN", L"disableScreensaver", 1, iniPath );

		//acceptRecommendedDelay
		if( GetPrivateProfileIntW( L"MAIN", L"acceptRecommendedDelay", 0, iniPath ) ){
			acceptRecommendedDelayFlg = 1;
		}

		//allowObs
		if( GetPrivateProfileIntW( L"MAIN", L"allowObs", 1, iniPath ) ){
			allowObsFlg = 1;
		}else{
			allowObsFlg = 0;
		}


		// ***** replay stuff
		const WCHAR *ini_block = isRollIni?L"REPLAY":L"MAIN";
		//replaySave
		if( GetPrivateProfileIntW( ini_block, L"replaySave", 0, iniPath ) ){
			replaySaveFlg = 1;
		}

 		//autoSave
 		if( GetPrivateProfileIntW( ini_block, L"autoSave", 0, iniPath ) ){
			autoSaveFlg = 1;
 		}

 		//autoSaveReplayFileRenameFlg
 		if( GetPrivateProfileIntW( ini_block, L"autoSaveReplayFileRename", 0, iniPath ) ){
			autoSaveReplayFileRenameFlg = 1;
 		}
 		
 		//replayMetadataSaveFlg
 		if( GetPrivateProfileIntW( ini_block, L"replayMetadataSave", 0, iniPath ) ){
			replayMetadataSaveFlg = 1;
 		}
 		
		GetPrivateProfileStringW( ini_block, L"replayFilenameFormat", replayFilenameFormat, replayFilenameFormat, 200, iniPath );

 		// ***** advanced
 		ini_block = isRollIni?L"ADVANCED":L"MAIN";

		//th075booster
		boosterOnFlg = GetPrivateProfileIntW( ini_block, L"th075Booster", 0, iniPath );


 		//autoSaveReplayNameFlg
 		if( GetPrivateProfileIntW( ini_block, L"autoSaveReplayName", 0, iniPath ) ){
			autoSaveReplayNameFlg = 1;
 		}

		//autoWaitOn
		if( GetPrivateProfileIntW( ini_block, L"autoWaitOn", 0, iniPath ) ){
			autoWaitOnFlg = 1;
//			autoWaitFlg = 1;
		}

		//endTime
		if( GetPrivateProfileIntW( ini_block, L"time", 0, iniPath ) ){
			endTimeFlg = 1;
		}

		//escSelect
		if( GetPrivateProfileIntW( ini_block, L"escSelect", 0, iniPath ) ){
			escSelectFlg = 1;
			}

		//lessCast
		lessCastFlg = GetPrivateProfileIntW( ini_block, L"lessCast", 1, iniPath );

		//zlib
		zlibFlg = GetPrivateProfileIntW(ini_block, L"zlib", 1, iniPath);

		//perfectFreeze
		perfectFreezeFlg = GetPrivateProfileIntW( ini_block, L"perfectFreeze", 0, iniPath );

		//obsCount
		if( GetPrivateProfileIntW( ini_block, L"obsCount", 0, iniPath ) ){
			obsCountFlg = 1;
		}

		//playerSide
		playerSideFlg = GetPrivateProfileIntW( ini_block, L"playerSide", 0, iniPath );
		if( playerSideFlg > 3 ) playerSideFlg = 0;

		// anonymousObs
 		if( GetPrivateProfileIntW( ini_block, L"anonymousObs", 0, iniPath ) ){
 			anonymousObsFlg = GetPrivateProfileIntW( ini_block, L"anonymousObs", 0, iniPath );
 		}

		// defaultMode
 		if( GetPrivateProfileIntW( ini_block, L"defaultMode", 0, iniPath ) ){
 			lastMode = GetPrivateProfileIntW( ini_block, L"defaultMode", 0, iniPath );
 		}

		// practiceNoSpecFlg
 		if( GetPrivateProfileIntW( ini_block, L"practiceNoSpec", 0, iniPath ) ){
 			practiceNoSpecFlg = GetPrivateProfileIntW( ini_block, L"practiceNoSpec", 0, iniPath );
 		}

		// dontReportRemoteFlg
 		if( GetPrivateProfileIntW( ini_block, L"dontReportRemote", 0, iniPath ) ){
 			dontReportRemoteFlg = GetPrivateProfileIntW( ini_block, L"dontReportRemote", 0, iniPath );
 		}

 		// unknownNameFlg
 		if( GetPrivateProfileIntW( ini_block, L"unknownName", 0, iniPath ) ){
 			unknownNameFlg = GetPrivateProfileIntW( ini_block, L"unknownName", 0, iniPath );
 		}

 		// boosterWarningsFlg
 		if( GetPrivateProfileIntW( ini_block, L"boosterWarnings", 0, iniPath ) ){
 			boosterWarningsFlg = GetPrivateProfileIntW( ini_block, L"boosterWarnings", 0, iniPath );
 		}


		//floatControl
		floatControlFlg = (WORD)GetPrivateProfileIntW( ini_block, L"floatControl", floatControlFlg, iniPath );

		// defaultIP
		iniReadStringNonUnicode( ini_block, L"defaultIP", L"0.0.0.0", accessIP, 79, iniPath );

		// defaultStandbyIP
		iniReadStringNonUnicode( ini_block, L"defaultStandbyIP", L"mizuumi.net", standbyIP, 79, iniPath );

		//top
		windowTopFlg = (WORD)GetPrivateProfileIntW( L"POSITION", L"setWindowTop", 0, iniPath );

		//deSyncSound
		deSyncSoundFlg = (WORD)GetPrivateProfileIntW( L"SOUND", L"deSyncSound", 0, iniPath );

		iniReadStringNonUnicode( L"PLAYER", L"name", L"", myPlayerName, 16, iniPath );
		cleanString(myPlayerName, 1);

		iniReadStringNonUnicode( L"PLAYER", L"shortName", L"", myShortName, 5, iniPath );
		cleanString(myShortName, 1);
		
		useth075eDLL = GetPrivateProfileIntW( ini_block, L"useth075eDLL", useth075eDLL, iniPath);

 		// systemTimeFlg
 		if( GetPrivateProfileIntW( ini_block, L"systemTime", 0, iniPath ) ){
 			systemTimeFlg = GetPrivateProfileIntW( ini_block, L"systemTime", 0, iniPath );
 		}

 		// humanTimeFlg
 		if( GetPrivateProfileIntW( ini_block, L"humanTime", 0, iniPath ) ){
 			humanTimeFlg = GetPrivateProfileIntW( ini_block, L"humanTime", 0, iniPath );
 		}

 		// origMenuFlg
 		if( GetPrivateProfileIntW( ini_block, L"origMenu", 0, iniPath ) ){
 			origMenuFlg = GetPrivateProfileIntW( ini_block, L"origMenu", 0, iniPath );
 		}

 		// processRemotePalettesFlg
 		if( GetPrivateProfileIntW( ini_block, L"processRemotePalettes", 0, iniPath ) ){
 			processRemotePalettesFlg = GetPrivateProfileIntW( ini_block, L"processRemotePalettes", 0, iniPath );
 		}

 		// noFastFlg
 		if( GetPrivateProfileIntW( ini_block, L"noFast", 0, iniPath ) ){
 			noFastFlg = GetPrivateProfileIntW( ini_block, L"noFast", 0, iniPath );
 		}

 		windowWidth = GetPrivateProfileIntW( L"WINDOW", L"windowWidth", 640, iniPath );
 		if (windowWidth < 320) {
			windowWidth = 320;
			cout << "Error reading config file: POSITION::width must be at least 320" << endl;
 		}
 		if (windowWidth != 640) {
 			windowResizeFlg = 1;
 		}

 		windowFilterState = GetPrivateProfileIntW( L"WINDOW", L"bilinearFilterState", 2, iniPath );
 		if (windowFilterState > 3) {
			windowFilterState = 2;
 		}

 		windowDisableTransitionsFlg = GetPrivateProfileIntW( L"WINDOW", L"disableMenuTransitions", 0, iniPath );

 		windowHeight = (int)(((float)windowWidth/(float)640.0)*480.0);
 		
		rollbackFlg = GetPrivateProfileIntW( L"ROLLBACK", L"enable", 1, iniPath );
		rollbackMinInputDelay = GetPrivateProfileIntW( L"ROLLBACK", L"minInputDelay", rollbackMinInputDelay, iniPath );
		rollbackMaxRewind = GetPrivateProfileIntW( L"ROLLBACK", L"maxRewind", rollbackMaxRewind, iniPath );
		
 		const WCHAR *keybind_strings[KEY_COUNT];
 		ZeroMemory(keybind_strings, sizeof(keybind_strings));
		keybind_strings[KEY_AUTOSAVE_ON] = L"autoSaveOn";
		keybind_strings[KEY_AUTOSAVE_OFF] = L"autoSaveOff";
		keybind_strings[KEY_AUTOSAVE_TOGGLE] = L"autoSaveToggle";
		keybind_strings[KEY_AUTONEXT_ON] = L"autoNextOn";
		keybind_strings[KEY_AUTONEXT_OFF] = L"autoNextOff";
		keybind_strings[KEY_AUTONEXT_TOGGLE] = L"autoNextToggle";
		keybind_strings[KEY_ROUNDCOUNT_CYCLE] = L"roundShowCycle";
		keybind_strings[KEY_BGM_TOGGLE] = L"bgmToggle";
		keybind_strings[KEY_NOFAST_TOGGLE] = L"noFastToggle";
		keybind_strings[KEY_SAVE_PALETTE] = L"saveRemotePalette";
		keybind_strings[KEY_INFINITE_SPIRIT_CHEAT] = L"infiniteSpiritCheat";
		keybind_strings[KEY_REMOTE_PALETTE_CYCLE] = L"processRemotePalettesCycle";
		keybind_strings[KEY_STAGE_FREE] = L"stageFree";
		keybind_strings[KEY_STAGE_MANUAL] = L"stageManual";
		keybind_strings[KEY_ALWAYSONTOP_TOGGLE] = L"alwaysOnTopToggle";
		keybind_strings[KEY_DELAY1] = L"delay1";
		keybind_strings[KEY_DELAY2] = L"delay2";
		keybind_strings[KEY_DELAY3] = L"delay3";
		keybind_strings[KEY_DELAY4] = L"delay4";
		keybind_strings[KEY_DELAY5] = L"delay5";
		keybind_strings[KEY_DELAY6] = L"delay6";
		keybind_strings[KEY_DELAY7] = L"delay7";
		keybind_strings[KEY_DELAY8] = L"delay8";
		keybind_strings[KEY_DELAY9] = L"delay9";
		keybind_strings[KEY_DELAY10] = L"delay10";
		keybind_strings[KEY_REWIND_MODIFIER] = L"rewindModifier";

		for (int i = 0; i < KEY_COUNT; ++i) {
			WORD bind = GetPrivateProfileIntW( L"KEYBIND", keybind_strings[i], 0xffff, iniPath );
			if (bind != 0xffff) {
				keybinds[i] = bind;
			}
		}
	}
	
	s = INVALID_SOCKET;

	Here.sin_family = AF_INET;
	Here.sin_addr.s_addr = htonl( INADDR_ANY );


	if( !myPort ){
		WORD Temp;
		myPort = 7500;
		cout	<< endl
			<< "<StartUp>" << endl
			<< "0 : Exit" << endl
			<< "1 : UDP.7500" << endl
			<< "2 : UDP.0" << endl
			<< "3 : Specific Port" << endl
			<< "Input >";
		cin >> Temp;
		if( cin.fail() ){
			//cin.clear();
			//cin.ignore(1024,'\n');
			Temp = 0;
		}
		switch( Temp ){
		case 0 :
			return 1;
		case 1 :
			break;
		case 2 :
			myPort = 0;
			break;
		case 3 :
			cin.clear();
			cin.ignore(1024,'\n');
			cout << "Input Port Number >";
			cin >> myPort;
			if( cin.fail() ){
				myPort = 7500;
			}
			break;
		default :
			return 1;
		}

		cin.clear();
		cin.ignore(1024,'\n');
	}

	/*
	if( myPort == 7501 || myPort > 10000 ){
		myPort = 7500;
	}
	 */

	Here.sin_port = htons( myPort );

	if (!myPlayerName[0] ) {
		if (myShortName[0]) {
			strcpy(myPlayerName, myShortName);
		}
		/*
		if (!GetPrivateProfileIntW( L"PLAYER", L"dontNagForName", 0, iniPath)) {
			cout << endl;
			cout << "You have not configured your player information." << endl;
			cout << "Please edit config_caster.ini to fix this." << endl;
			cout << "Enter your name (15 letters max): ";
			char buf[80];
			cin.getline(buf, 79);
			memcpy(myPlayerName, buf, 15);
			myPlayerName[15] = '\0';
			cin.clear();
			cout << "Enter your short name (4 letters max): ";
			cin.getline(buf, 79);
			memcpy(myShortName, buf, 4);
			myShortName[4] = '\0';
			cout << endl;
		}
		 */
	}

	if (myPlayerName[0]) {
		if (!myShortName[0]) {
			memcpy(myShortName, myPlayerName, 4);
			myShortName[4] = '\0';
		}
		cout << "You are identified as: [" << myShortName << "] " << myPlayerName << endl;
	}

	WORD wVersion = MAKEWORD( 2, 0);
	if( !WSAStartup(wVersion , &wsaData) ){
		if( wVersion == wsaData.wVersion) {
			hPrintMutex = CreateMutex(NULL, FALSE, NULL);
			if( hPrintMutex ){
				hMutex = CreateMutex(NULL, FALSE, NULL);
				if( hMutex ){
					hInputRecvEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
					if( hInputRecvEvent ){
						hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
						if( hSendEvent ){
							hManageTh = (HANDLE)_beginthreadex(NULL, 0, manageThread, this, 0, NULL);
							if( hManageTh ){
								SetThreadPriority( hManageTh, THREAD_PRIORITY_TIME_CRITICAL );
								s = socket(AF_INET , SOCK_DGRAM , 0);
								if( s != INVALID_SOCKET ){
									if( bind( s, (SOCKADDR*)&Here, sizeof(Here)) >= 0 ){
										//要検討
										GetMyPort();
										hSendTh = (HANDLE)_beginthreadex(NULL, 0, sendThread, this, 0, NULL);
										if( hSendTh ){
										 	if( SetThreadPriority( hSendTh, THREAD_PRIORITY_TIME_CRITICAL ) ){
												hRecvTh = (HANDLE)_beginthreadex(NULL, 0, recvThread, this, 0, NULL);
												if( hRecvTh ){
												 	SetThreadPriority( hRecvTh, THREAD_PRIORITY_TIME_CRITICAL );

												 	hBoosterTh = (HANDLE)_beginthreadex(NULL, 0, boosterThread, this, 0, NULL);
												 	if( hBoosterTh ){
												 		SetThreadPriority( hBoosterTh, THREAD_PRIORITY_LOWEST );
												 	}

												 	goto noError;
												}
												continueFlg = 0;
												SetEvent(hSendEvent);
												WaitForSingleObject(hSendTh, 1000);
												CloseHandle(hSendTh);
											}
										}
									}else{
										cout << "debug : Bind error." << endl;
										cout << "        You probably have another Caster running on the same port." << endl;
										cout << "        Close it out." << endl;
										Sleep( 2000 );
									}
									closesocket(s);
								}
								continueFlg = 0;
								WaitForSingleObject(hManageTh, 1000);
								CloseHandle(hManageTh);
							}
							CloseHandle( hSendEvent );
						}
						CloseHandle( hInputRecvEvent );
					}
					CloseHandle( hMutex );
				}
				CloseHandle( hPrintMutex );
			}
		}
		WSACleanup();
	}
	return 1;

noError :
	return 0;
}



int mainDatClass::TerminateTh075(){
	if( hTh075Th ){
		if( WaitForSingleObject( hTh075Th, 0) ){
			HWND hWnd = FindWindow( NULL , windowName );
			if( hWnd ){
				for(;;){
					hWnd = FindWindow( NULL , windowName );
					if( !hWnd ){
						if( WaitForSingleObject(hProcess, 10000) ){
							CloseHandle( pi.hThread );
							pi.hThread = NULL;

							CloseHandle( pi.hProcess );
							pi.hProcess = NULL;
						}
						break;
					}else{
						PostMessage(hWnd, WM_CLOSE, 0, 0);
						if( !WaitForSingleObject(hProcess, 10000) ) break;
					}
				}
			}else{
				if( WaitForSingleObject(hProcess, 10000) ){
					CloseHandle( pi.hThread );
					pi.hThread = NULL;

					CloseHandle( pi.hProcess );
					pi.hProcess = NULL;
				}
			}
			WaitForSingleObject( hTh075Th, 10000);
		}
		CloseHandle( hTh075Th );
		hTh075Th = NULL;
	}
	return 0;
}


int mainDatClass::mainDatEnd(){
	//スレッドを閉じる
	continueFlg = 0;
	
	if( hSendTh ){
		SetEvent(hSendEvent);
		WaitForSingleObject(hSendTh, 100);
		CloseHandle(hSendTh);
		hSendTh = NULL;
	}
	CloseHandle(hSendEvent);
	hSendEvent = NULL;
	
	if( s != INVALID_SOCKET ) closesocket(s);

	if( hRecvTh ){
		DWORD Flg = WAIT_TIMEOUT;
		while(Flg != WAIT_OBJECT_0 && Flg != WAIT_FAILED){
			Flg = WaitForSingleObject(hRecvTh, 100);
		}
		CloseHandle(hRecvTh);
		hRecvTh = NULL;
	}
	
	WSACleanup();
	
	//萃夢想のスレッドが開いていたら閉じる
	TerminateTh075();

	WaitForSingleObject(hManageTh, 1000);
	CloseHandle(hManageTh);
	hManageTh = NULL;

	if( hBoosterTh ){
		WaitForSingleObject(hBoosterTh, 1000);
		CloseHandle(hBoosterTh);
		hBoosterTh = NULL;
	}

	CloseHandle( hMutex );
	hMutex = NULL;

	CloseHandle( hPrintMutex );
	hPrintMutex = NULL;

	return 0;
}

sTaskClass::sTaskClass(){
	//debug
//	cout << "debug : sTask " << hex << (DWORD)&data[0] << " : " << dec << (DWORD)&data[0] << endl;

	Flg = 0;
}

int mainDatClass::charInit(){
	datA.hProcess = &hProcess;
	datB.hProcess = &hProcess;

	datA.th075Flg = &th075Flg;
	datB.th075Flg = &th075Flg;

	datA.windowName = windowName;
	datB.windowName = windowName;

	if (myInfo.terminalMode == mode_broadcast) {
		if( datA.init() || datB.init() ) return 1;
	} else if (myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug) {
		if( datA.init() ) return 1;
		datB.init();
	} else {
		if ( datA.init() ) {
			if (datA.init_simple_kbd()) return 1;
			cout << "Spectating and player 1 pad not found, using default keyboard inputs." << endl;
		}
	}
	return 0;
}

int mainDatClass::charEnd(){
	datA.end();
	datB.end();
	return 0;
}
