#include "mainDatClass.h"
#include <direct.h>
using namespace std;

void mainDatClass::calcRewind(DWORD delay, DWORD *newDelayTime, DWORD *newRewindAmount) {
	DWORD localDelayTime = delay;
	DWORD rewindAmount = 0;
	
	if (rollbackOk && myInfo.terminalMode == mode_root) {
		DWORD v = delay / 2;
		
		if (rollbackMinInputDelay <= v) {
			rewindAmount = v;
			localDelayTime = 0;
			
			rewindAmount -= rollbackMinInputDelay;
			localDelayTime += rollbackMinInputDelay;
			
			if (rewindAmount > rollbackMaxRewind) {
				DWORD r = rewindAmount - rollbackMaxRewind;
				
				rewindAmount -= r;
				localDelayTime += r;
			}
		} else {
			rewindAmount = 0;
			localDelayTime = v;
		}

		rewindAmount *= 2;
		localDelayTime *= 2;
	}
	
	*newDelayTime = localDelayTime;
	*newRewindAmount = rewindAmount;
}

int mainDatClass::mainStep(){
	#if debug_mode_mainRoop
		WaitForSingleObject( hPrintMutex, INFINITE );
		cout << "debug : mainStep()" << endl;
		ReleaseMutex( hPrintMutex );
	#endif

	UnRockTime();


	DWORD deInfo;
	DWORD gameTime;
	DWORD gameTimeNoCast = 0;
	BYTE InputA;
	BYTE InputB = 0;
	BYTE InputABuf;
	BYTE InputBBuf;
	BYTE pauseFlg;
	DWORD FlgA;
	DWORD FlgB;
	BYTE	menuFlg;
	BYTE  sendBufBody[40];
	BYTE* sendBuf = &sendBufBody[6];
	BYTE  castBufBody[40];
	BYTE* castBufA = &castBufBody[6];
	BYTE* castBufB = &castBufBody[12];
	memset( sendBufBody, 0, sizeof( sendBufBody ) );
	memset( castBufBody, 0, sizeof( castBufBody ) );
	BYTE diInputA;
	BYTE diInputB;
	WORD fastFlg = 0;
	//HWND hWnd = FindWindow( NULL , windowName );

	BYTE HPCountA = 0;
	BYTE HPCountB = 0;
	
	DWORD inputBufCache[2][20][7];
	memset(inputBufCache, 0, sizeof(inputBufCache));
	
	BYTE rollbackInput[2][20];
	memset(rollbackInput, 0xFF, sizeof(rollbackInput));


	myInfo.phase = phase_battle;
	dataInfo.phase = phase_battle;

	BYTE sessionNo;
	BYTE sessionID;

	//要検討
	if( myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug || myInfo.terminalMode == mode_broadcast ){
		sessionNo = myInfo.sessionNo;
		sessionID = myInfo.sessionID;
	}else if( myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch ){
		sessionNo = dataInfo.sessionNo;
		sessionID = dataInfo.sessionID;
	}

	sendBufBody[0] = sessionNo;
	sendBufBody[1] = sessionID;

	castBufBody[0] = sessionNo;
	castBufBody[1] = sessionID;

	DWORD menuAddress;
	BYTE gamePhase;

//	srand( 0 );
	{
		int sRand;
		if ( newCWflg ) {
			sRand = 0;
		} else {
			sRand = 1;
		}
		WriteMemory( (void*)rand_address, &sRand, 4 );
	}

	//緩和
	DWORD roopCounterReq = 0;
	DWORD roopCounterSend = 0;

	//先読み実験
	DWORD preReqTime = 0;

	//sync
	DWORD HPA;
	DWORD HPB;
	DWORD XA;	//* 64 and float->DWORD
	DWORD XB;
	WORD deSyncShowFlg = 1;
	WORD infiniteSpiritCheat = 0;
	
	boosterInput = 0;
	
	// RollCaster data
	DWORD endOfMatch = 0;
	DWORD gracePeriod = 0;
	DWORD graceWait = 0;
	DWORD syncFrame = 204;
	DWORD syncEndFrame = 0;
	
	if (!rollbackOk) {
		localDelayTime = delayTime;
		rewindAmount = 0;
	}

	for(;;){
		if( th075Roop( &deInfo ) ) return 0xF;
		if( !roopFlg ){
			if( fastFlg ) WriteCode( (void *)0x66C23C, 16);
			UnRockTime();
			return 1;
		}
		if( deInfo == de_body ){
			//戦闘中の同期など
			ReadMemory( (void*)((DWORD)pStackBase - 420) , &menuFlg , 1 );	//12FE5C
			ReadMemory( (void*)0x6718B4, &gameTime, 4 );
			ReadMemory( (void*)0x67161C, &pauseFlg, 1 );
			ReadMemory( (void*)0x671418 , &FlgA , 4 );
			ReadMemory( (void*)0x67141C , &FlgB , 4 );
			ReadMemory( (void*)((DWORD)pStackBase - 936) , &menuAddress , 4 );	//12FC58
			if( FlgA && FlgB ){
				ReadMemory( (void*)(FlgA + 0x39D) , &HPCountA , 1 );
				ReadMemory( (void*)(FlgB + 0x39D) , &HPCountB , 1 );
			}else{
				HPCountA = 0;
				HPCountB = 0;
			}
			if( menuAddress ){
				ReadMemory( (void*)(menuAddress + 0x18) , &gamePhase , 1 );
			}else{
				gamePhase = 0;
			}
			
			bool isRewinding = 0;
			
			if (rollbackOk && gameTime > 100 && myInfo.terminalMode == mode_root) {
				int rewind = 0;
				
				int n = ((gameTime)/2)%20;
				
				if (myInfo.playerSide == 0xA) {
					DWORD time = gameTime + 2;
					for (int i = 0; i < 12; ++i) {
						if (time < 198) {
							break;
						}
						BYTE input;
						if (!inputData.GetInputData( sessionNo, time, 0xB, &input )){
							if (rollbackInput[1][n] != input) {
								rewind = i; // + 1;
							}
						}
						time -= 2;
						n = (n + 19) % 20;
					}
				} else if (myInfo.playerSide == 0xB) {
					DWORD time = gameTime + 2;
					for (int i = 0; i < 12; ++i) {
						if (time < 198) {
							break;
						}
						BYTE input;
						if (!inputData.GetInputData( sessionNo, time, 0xA, &input )){
							if (rollbackInput[0][n] != input) {
								rewind = i; // + 1;
							}
						}
						time -= 2;
						n = (n + 19) % 20;
					}
				}
				
				if (rewind > 0) {
					isRewinding = 1;
					
					WriteCode((void *)rollbackRewindAddr, rewind);
				}
			}
			
			//sync data
			if( gameTime > 100 && FlgA && FlgB && !rewindAmount){
				float XATemp = 0;
				float XBTemp = 0;
				HPA = 0;
				HPB = 0;

				ReadMemory( (void*)(FlgA + 0x324) , &HPA , 2 );
				ReadMemory( (void*)(FlgA + 0x44) , &XATemp , 4 );

				ReadMemory( (void*)(FlgB + 0x324) , &HPB , 2 );
				ReadMemory( (void*)(FlgB + 0x44) , &XBTemp , 4 );

				XA = (DWORD)(XATemp * 64);
				XB = (DWORD)(XBTemp * 64);

				syncData.SetSyncDataHereA( gameTime, HPA, XA );
				syncData.SetSyncDataHereB( gameTime, HPB, XB );
			}
			
			if (gameTime > 100) {
				float XATemp = 0;
				float XBTemp = 0;
				HPA = 0;
				HPB = 0;

				ReadMemory( (void*)(FlgA + 0x324) , &HPA , 2 );
				ReadMemory( (void*)(FlgA + 0x44) , &XATemp , 4 );

				ReadMemory( (void*)(FlgB + 0x324) , &HPB , 2 );
				ReadMemory( (void*)(FlgB + 0x44) , &XBTemp , 4 );

				XA = (DWORD)(XATemp * 64);
				XB = (DWORD)(XBTemp * 64);
			}
			
			if (HPCountA && HPCountB && gamePhase != 3) {
				// match ongoing
				endOfMatch = 999999999;
			} else if (!rewindAmount || myInfo.terminalMode != mode_root) {
				// not rewinding or not root
				endOfMatch = 0;
			} else if (endOfMatch == 999999999) {
				// match POSSIBLY ending, wait to see
				endOfMatch = gameTime + 2;

				// In case this is not the first end of match frame,
				// calculate when it REALLY happened.
				if (menuAddress) {
					DWORD animCtr;
					ReadMemory( (void *)(menuAddress + 0x60), &animCtr, 4);
					animCtr = 0x95 - animCtr;
					if (animCtr < 0x95) {
						endOfMatch -= animCtr * 2;
					}
				}
			} else if (!isRewinding) {
				// check for final input
				DWORD enSide;
				if (myInfo.playerSide == 0xA) {
					enSide = 0xB;
				} else {
					enSide = 0xA;
				}
				if( !inputData.GetInputData( sessionNo, endOfMatch, enSide, NULL ) ){
					endOfMatch = 0;
				}
			}
			
			if( !FlgA || !FlgB || menuFlg != 0 || gameTime==0 || !endOfMatch ){
				if( lessCastFlg ){
					if( !endOfMatch && gameTimeNoCast >= 8 ){
						int count = 1;
						if (rollbackOk && rewindAmount > 0) {
							gameTimeNoCast -= rewindAmount + 2;
							count = ((rewindAmount/2) + 4)/2;
						}
						for (int i = 0; i < count; ++i) {
							inputData.GetInputData( sessionNo, gameTimeNoCast - 8, 0xA, &castBufA[5] );
							inputData.GetInputData( sessionNo, gameTimeNoCast - 6, 0xA, &castBufA[4] );
							inputData.GetInputData( sessionNo, gameTimeNoCast - 4, 0xA, &castBufA[3] );
							inputData.GetInputData( sessionNo, gameTimeNoCast - 2, 0xA, &castBufA[2] );
							inputData.GetInputData( sessionNo, gameTimeNoCast, 0xA, &castBufA[1] );
							inputData.GetInputData( sessionNo, gameTimeNoCast + 2, 0xA, &castBufA[0] );
							inputData.GetInputData( sessionNo, gameTimeNoCast - 8, 0xB, &castBufB[5] );
							inputData.GetInputData( sessionNo, gameTimeNoCast - 6, 0xB, &castBufB[4] );
							inputData.GetInputData( sessionNo, gameTimeNoCast - 4, 0xB, &castBufB[3] );
							inputData.GetInputData( sessionNo, gameTimeNoCast - 2, 0xB, &castBufB[2] );
							inputData.GetInputData( sessionNo, gameTimeNoCast, 0xB, &castBufB[1] );
							inputData.GetInputData( sessionNo, gameTimeNoCast + 2, 0xB, &castBufB[0] );
							*(DWORD*)&castBufBody[2] = gameTimeNoCast + 2;
							SendCmd( dest_leaf, cmd_cast, castBufBody, 18 );
							
							gameTimeNoCast += 4;
						}
					}
				}

				if( myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch ){
					//観戦しててタイムアウトしたときはroopFlgが1のままのため
					if( !Root.sin_addr.s_addr ){
						roopFlg = 0;
					}
				}

				lastGameTime = gameTime;
				totalGameTime += gameTime;

				//send message
				if( !HPCountA || !HPCountB ){
					if (!( HPCountA && HPCountB) && !( !HPCountA && !HPCountB ) ) {
						if (HPCountA) {
							winsA++;
							roundsWonA += 2;
							if (HPCountA == 1) {
								roundsWonB += 1;
							}
						} else {
							winsB++;
							roundsWonB += 2;
							if (HPCountB == 1) {
								roundsWonA += 1;
							}
						}

						for (int i = 0; i < 2; ++i) {
							ostream *stream;
							if (i == 0) {
								if (roundShowFlg < 2) {
									continue;
								}
								stream = &cout;
							} else if (i == 1) {
								if (!sessionLogFlg || !sessionLogFile) {
									continue;
								}
								stream = &sessionLogFile;

								printDate(stream);
								*stream << ' ';
							}

							printTime(stream);
							*stream << ' ';

							if (HPCountA) {
								*stream << "Player 1 wins match ";
								if (HPCountA == 1) {
									*stream << "2-1 : " ;
								} else {
									*stream << "2-0 : " ;
								}
							} else {
								*stream << "Player 2 wins match ";
								if (HPCountB == 1) {
									*stream << "1-2 : " ;
								} else {
									*stream << "0-2 : " ;
								}
							}

							for (int i = 2; i <= 3; ++i) {
								const charInfoStruct *c;
								if (i == 2) {
									c = &myInfo.A;
								} else {
									c = &myInfo.B;
								}

								*stream << getCharacterName(c->ID);

								*stream << "(" << (int)(c->firstSpell)+1 << "," << (int)(c->secondSpell)+1 << ")";

								if (i == 2) {
									*stream << " vs ";
								} else {
									if (gameTime > 0) {
										if (humanTimeFlg && totalGameTime >= 120 && i == 0) {
											DWORD seconds = totalGameTime / 120;
											DWORD minutes = seconds / 60;
											seconds -= minutes * 60;
											
											*stream << " (" << dec << minutes << "m " << seconds << "s)" << endl;
										} else {
											*stream << " (" << dec << gameTime/120 << "s)" << endl;
										}
									} else {
										*stream << endl;
									}
								}
							}
						}

						if (sessionLogFile) {
							sessionLogFile.flush();
						}
					}

					if( ( myInfo.playerSide == 0xA || myInfo.playerSide == 0xB )
					    && !( HPCountA && HPCountB )
					    && !( !HPCountA && !HPCountB ) )
					{
						if( ( myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug ) && uMsg ){
							BYTE wParamArray[4];
							*(DWORD*)wParamArray = 0;
							charInfoStruct* mySideInfo;
							charInfoStruct* enSideInfo;

							if( myInfo.playerSide == 0xA ){
								mySideInfo = &myInfo.A;
								enSideInfo = &myInfo.B;
							}else{
								mySideInfo = &myInfo.B;
								enSideInfo = &myInfo.A;
							}

							//wParamArrayの内容はここから
							BYTE mySide;
							BYTE myCount;
							BYTE enCount;
							if( myInfo.playerSide == 0xA ){
								mySide = 1;
								myCount = HPCountA;
								enCount = HPCountB;
							}else{
								mySide = 2;
								myCount = HPCountB;
								enCount = HPCountA;
							}

							//win/lose
							if( myCount ){
								//win
								wParamArray[0] &= 0x7F;

								if( myCount == 2 ){
									//2-0
									wParamArray[0] &= 0xFE;
								}else{
									//2-1
									wParamArray[0] |= 0x01;
								}
							}else{
								//lose
								wParamArray[0] |= 0x80;

								if( enCount == 2 ){
									//0-2
									wParamArray[0] &= 0xFE;
								}else{
									//1-2
									wParamArray[0] |= 0x01;
								}


							}

							//playerSide
							wParamArray[1] = mySide;

							//mySide Info
							wParamArray[2] = mySideInfo->ID;
							wParamArray[2] |= mySideInfo->firstSpell << 4;
							wParamArray[2] |= mySideInfo->secondSpell << 6;

							//enSide Info
							wParamArray[3] = enSideInfo->ID;
							wParamArray[3] |= enSideInfo->firstSpell << 4;
							wParamArray[3] |= enSideInfo->secondSpell << 6;

							if( !( myInfo.terminalMode == mode_debug && boosterFlg ) ){
								PostMessage( HWND_BROADCAST, uMsg, *(WPARAM*)wParamArray, 0 );
							}

							if( myInfo.terminalMode == mode_debug && boosterFlg == 0 ){
								cout  << hex << "th075_result " << (WORD)wParamArray[0] << ","
									<< (WORD)wParamArray[1] << "," << (WORD)wParamArray[2] << "," << (WORD)wParamArray[3] << "," << endl;
							}
						}
					}
				}

				//要検討
				myInfo.phase = phase_default;
				dataInfo.phase = phase_default;

				UnRockTime();
				datA.SetInput( diInputA );
				datB.SetInput( diInputB );
				break;
			}

			if( gameTime > 800 && !(gameTime%20) ){
				int Res;
				Res = syncData.TestSyncData( gameTime - 200 );

				/*
				if( Res == 0 ){
					cout << "debug : Sync Good " << Res << endl;
				}
				if( Res == 1 || Res == 2 || Res == 3 ){
					//相手が同期チェックに対応してないときなど
					cout << "Tested " << (gameTime - 200) << endl;
					cout << "debug : sync error " << Res << endl;
				}
				 */
				if( Res == 0xF ){
					if( deSyncShowFlg ){
						deSyncShowFlg = 0;

						cout << "ERROR : deSync" << endl;
						if( deSyncSoundFlg ){
							if( lstrcmpW( nowDir, L"fail" ) && lstrlenW( nowDir ) < 280 ){
								WCHAR path[300];
								lstrcpyW( path, nowDir );
								lstrcatW( path, L"\\desync.wav\0" );
								if( !_waccess( path, 0 ) ){
									PlaySoundW( path, NULL, SND_ASYNC );
								}else if( deSyncSoundFlg == 2 ){
									lstrcpyW( path, nowDir );
									lstrcatW( path, L"\\sound.wav\0" );
									if( !_waccess( path, 0 ) ){
										PlaySoundW( path, NULL, SND_ASYNC );
									}
								}
							}
						}
//						roopFlg = 0;
					}
				}
			}


			if( (DWORD)( gameTime / 2 ) * 2 != gameTime ) gameTime--;
			if( myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_broadcast || myInfo.terminalMode == mode_debug ){
				dataInfo.gameTime = gameTime;
				inputData.SetTime( sessionNo, gameTime );
			}
			myInfo.gameTime = gameTime;

			//diInput
			InputA = 0;
			InputB = 0;
			diInputA = datA.GetInput();
			diInputB = datB.GetInput();
			if( diInputA == key_P ){
				if( datA.inputDeviceType == 0xFF ){
					HWND hFocus = GetForegroundWindow();
					if( hFocus && hWnd != hFocus ){
						diInputA = 0;
					}
				}
			}
			if( diInputB == key_P ){
				if( datB.inputDeviceType == 0xFF ){
					HWND hFocus = GetForegroundWindow();
					if( hFocus && hWnd != hFocus ){
						diInputB = 0;
					}
				}
			}
			if( diInputA == key_P ){
				if( gameTime <= 400 ){
					diInputA = 0;
				}
			}
			if( diInputB == key_P ){
				if( gameTime <= 400 ){
					diInputB = 0;
				}
			}

			if( myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch ){
				//早送り
				if( rockFlg ){
					if( fastFlg ){
						WriteCode( (void *)0x66C23C, 16);
						fastFlg = 0;
					}
				}else{
					if( fastFlg ){
						if( gameTime + 30 > inputData.GetTime( sessionNo ) ){
							WriteCode( (void *)0x66C23C, 16);
							fastFlg = 0;
						}else if( gameTime + 120 < inputData.GetTime( sessionNo ) ){
							if (!noFastFlg) WriteCode( (void *)0x66C23C, 8);
							fastFlg = 1;
						}
					}else{
						if( gameTime + 120 < inputData.GetTime( sessionNo ) ){
							if (!noFastFlg) WriteCode( (void *)0x66C23C, 8);
							fastFlg = 1;
						}else if( gameTime + 60 < inputData.GetTime( sessionNo ) ){
							if (!noFastFlg) WriteCode( (void *)0x66C23C, 14);
							fastFlg = 1;
						}
					}
				}
			}

			//先読み
			//要検討
			if( myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch ){
				if( gameTime >= preReqTime + 60 ){
					if( inputData.GetTime( sessionNo ) >= gameTime + 300 ){
						if( !inputData.GetInputDataA( sessionNo, gameTime + 2, NULL ) && !inputData.GetInputDataB( sessionNo, gameTime + 2, NULL ) ){
							if( inputData.GetInputDataA( sessionNo, gameTime + 62, NULL ) && inputData.GetInputDataB( sessionNo, gameTime + 62, NULL ) ){
								//送信
								if( zlibFlg ){
									BYTE data[7];
									data[0] = sessionNo;
									data[1] = sessionID;
									*(DWORD*)&data[2] = gameTime + 62;
									data[6] = 0;	//仕様を拡張するため
									SendCmd( dest_root, cmd_inputdata_req, data, 7 );
								}else{
									BYTE data[6];
									data[0] = sessionNo;
									data[1] = sessionID;
									*(DWORD*)&data[2] = gameTime + 62;
									SendCmd( dest_root, cmd_inputdata_req, data, 6 );
								}

								preReqTime = gameTime;

//								cout << "debug : preReqTime " << preReqTime << endl;
							}
						}
					}
				}
			}

			//bodyInput
			if( gameTime <= 200 ){
				diInputA = 0;
				diInputB = 0;
			}

			if( myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug ){
				if( myInfo.terminalMode == mode_debug ){
					if( boosterFlg ){
						inputData.SetInputData( sessionNo, gameTime + 2, 0xB, boosterInput );
					}
				}
				
				DWORD offset = localDelayTime;
				//if (rollbackOk) {
				//	offset = 0;
				//}

				if( inputData.GetInputData( sessionNo, gameTime + offset + 2, myInfo.playerSide, NULL ) ){
					inputData.SetInputData( sessionNo, gameTime + offset + 2, myInfo.playerSide, diInputA );

					//データ送信
					if( gameTime + offset - 8 >= 0 ){
						inputData.GetInputData( sessionNo, gameTime + offset - 8, myInfo.playerSide, &sendBuf[5] );
					}else{
						sendBuf[5] = 0;
					}
					if( gameTime + offset - 6 >= 0 ){
						inputData.GetInputData( sessionNo, gameTime + offset - 6, myInfo.playerSide, &sendBuf[4] );
					}else{
						sendBuf[4] = 0;
					}
					if( gameTime + offset - 4 >= 0 ){
						inputData.GetInputData( sessionNo, gameTime + offset - 4, myInfo.playerSide, &sendBuf[3] );
					}else{
						sendBuf[3] = 0;
					}
					if( gameTime + offset - 2 >= 0 ){
						inputData.GetInputData( sessionNo, gameTime + offset - 2, myInfo.playerSide, &sendBuf[2] );
					}else{
						sendBuf[2] = 0;
					}
					if( gameTime + offset >= 0 ){
						inputData.GetInputData( sessionNo, gameTime + offset , myInfo.playerSide, &sendBuf[1] );
					}else{
						sendBuf[1] = 0;
					}
					inputData.GetInputData( sessionNo, gameTime + offset + 2, myInfo.playerSide, &sendBuf[0] );

					*(DWORD*)&sendBufBody[2] = gameTime + offset + 2;

					if( myInfo.terminalMode == mode_debug && boosterFlg ){
						//none
						//vs th075Booster
					}else{
						if( gameTime%20 ){
							SendCmd( dest_away, cmd_sendinput, sendBufBody, 12 );
						}else if (!rewindAmount) {
							//send with syncData
//							cout << "debug : send syncData" << endl;
							*(DWORD*)&sendBufBody[12] = gameTime;
							*(DWORD*)&sendBufBody[16] = HPA;
							*(DWORD*)&sendBufBody[20] = XA;
							*(DWORD*)&sendBufBody[24] = HPB;
							*(DWORD*)&sendBufBody[28] = XB;

							SendCmd( dest_away, cmd_sendinput, sendBufBody, 32 );
						} else if (!syncData.GetSyncDataHere(gameTime - 20, &HPA, &HPB, &XA, &XB)) {
							*(DWORD*)&sendBufBody[12] = gameTime - 20;
							*(DWORD*)&sendBufBody[16] = HPA;
							*(DWORD*)&sendBufBody[20] = XA;
							*(DWORD*)&sendBufBody[24] = HPB;
							*(DWORD*)&sendBufBody[28] = XB;
							
							SendCmd( dest_away, cmd_sendinput, sendBufBody, 32 );
						}
					}
				}
			}else if( myInfo.terminalMode == mode_broadcast ){
				if( inputData.GetInputData( sessionNo, gameTime + delayTimeA + 2, 0xA, NULL ) ){
					inputData.SetInputData( sessionNo, gameTime + delayTimeA + 2, 0xA, diInputA );
				}
				if( inputData.GetInputData( sessionNo, gameTime + delayTimeB + 2, 0xB, NULL ) ){
					inputData.SetInputData( sessionNo, gameTime + delayTimeB + 2, 0xB, diInputB );
				}
			}

			//戦闘終了後に自動で進む
			if (keystate[KEY_INFINITE_SPIRIT_CHEAT] == 1) {
				infiniteSpiritCheat = 1 - infiniteSpiritCheat;
				if (infiniteSpiritCheat) {
					cout << "setting : infinite spirit cheat ON" << endl;
				} else {
					cout << "setting : infinite spirit cheat OFF" << endl;
				}
			}

			if (keystate[KEY_NOFAST_TOGGLE] == 1) {
				noFastFlg = !noFastFlg;
				if (noFastFlg) {
					cout << "setting : noFast ON" << endl;
				} else {
					cout << "setting : noFast OFF" << endl;
				}

				if (fastFlg) {
					if (noFastFlg) {
						WriteCode( (void *)0x66C23C, 16);
					} else {
						WriteCode( (void *)0x66C23C, 8);
					}
				}
			}

			if ( keystate[KEY_SAVE_PALETTE] == 1 && myInfo.terminalMode != mode_debug && myInfo.terminalMode != mode_broadcast ) {
				// Save palettes, whee.
				for (int i = 0; i < 2; ++i) {
					if (myInfo.terminalMode == mode_root) {
						if (i == 0 && myInfo.playerSide == 0xA) {
							continue;
						} else if (i == 1 && myInfo.playerSide == 0xB) {
							continue;
						}
					}
					if (paletteNo[i] < 0 || palettes[paletteNo[i]].state != PALETTE_LOADED) {
						continue;
					}

					char *name;

					if (i == 0) {
						name = p1PlayerName;
					} else {
						name = p2PlayerName;
					}

					if (!*name) {
						name = "Unknown";
					}

					// process palette into cpe form
					DWORD palette[256];
					DWORD *dpal = palette;
					const WORD *spal = (const WORD *)palettes[paletteNo[i]].data;

					for (int j = 0; j < 256; ++j) {
						*dpal++ = ((*spal&0x1f)<<3) | (((*spal>>5)&0x1f)<<11) | (((*spal>>10)&0x1f)<<19) | 0x80000000;

						spal++;
					}

					// search for free palette file OR existing palette file
					char palfilename[1024];
					int len;
					mkdir("palette");
					sprintf(palfilename,"palette\\%s-%s-",getCharacterName(i==0?myInfo.A.ID:myInfo.B.ID),name);
					len = strlen(palfilename);

					int n = 0;
					while (++n) {
						sprintf(palfilename+len, "%d.pal", n);

						ifstream file(palfilename);
						if (!file) {
							break;
						}

						DWORD srcpal[256];
						file.read((char *)srcpal, 1024);
						if (!memcmp(srcpal, palette, 1024)) {
							n = 0;
							cout << "Already have remote palette for player " << dec << (i+1) << ": " << palfilename << endl;
							break;
						}
					}

					if (!n) {
						continue;
					}

					ofstream file(palfilename, ios::binary);

					file.write((const char *)palette, 1024);

					cout << "Saved palette " << palfilename << endl;
				}
			}

			//delayTime変更
			if( myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug ){
				WORD delayTimeNew = 0;
				if (keystate[KEY_DELAY1] == 1) delayTimeNew = 2;
				if (keystate[KEY_DELAY2] == 1) delayTimeNew = 4;
				if (keystate[KEY_DELAY3] == 1) delayTimeNew = 6;
				if (keystate[KEY_DELAY4] == 1) delayTimeNew = 8;
				if (keystate[KEY_DELAY5] == 1) delayTimeNew = 10;
				if (keystate[KEY_DELAY6] == 1) delayTimeNew = 12;
				if (keystate[KEY_DELAY7] == 1) delayTimeNew = 14;
				if (keystate[KEY_DELAY8] == 1) delayTimeNew = 16;
				if (keystate[KEY_DELAY9] == 1) delayTimeNew = 18;
				if (keystate[KEY_DELAY10] == 1) delayTimeNew = 20;
				
				DWORD tempLocalDelayTime;
				DWORD tempRewindAmount;
				
				if (delayTimeNew > 0) {
					if (keystate[KEY_REWIND_MODIFIER] != 0 && myInfo.terminalMode == mode_root) {
						if (delayTimeNew > delayTime) {
							delayTimeNew = delayTime;
						}
					
						tempRewindAmount = delayTimeNew;
						tempLocalDelayTime = delayTime - delayTimeNew;
					
						delayTimeNew = delayTime;
					} else {
						calcRewind(delayTimeNew, &tempLocalDelayTime, &tempRewindAmount);
					}
				}

				if (delayTimeNew > 0 && (localDelayTime != tempLocalDelayTime || rewindAmount != tempRewindAmount)) {
					WORD delayTimeOld = delayTime;

					
					delayTime = delayTimeNew;
					
					if( localDelayTime != tempLocalDelayTime ){
						if( localDelayTime < tempLocalDelayTime){
							DWORD delayTimeStep;
							for(delayTimeStep = localDelayTime; delayTimeStep <= tempLocalDelayTime; delayTimeStep += 2){
								if( inputData.GetInputData( sessionNo, gameTime + delayTimeStep + 2, myInfo.playerSide, NULL ) ){
									inputData.SetInputData( sessionNo, gameTime + delayTimeStep + 2, myInfo.playerSide, diInputA );
								}
							}
							//delayTime = delayTimeNew;
							localDelayTime = tempLocalDelayTime;

							if( gameTime + localDelayTime - 8 >= 0 ){
								inputData.GetInputData( sessionNo, gameTime + localDelayTime - 8, myInfo.playerSide, &sendBuf[5] );
							}else{
								sendBuf[5] = 0;
							}
							if( gameTime + localDelayTime - 6 >= 0 ){
								inputData.GetInputData( sessionNo, gameTime + localDelayTime - 6, myInfo.playerSide, &sendBuf[4] );
							}else{
								sendBuf[4] = 0;
							}
							if( gameTime + localDelayTime - 4 >= 0 ){
								inputData.GetInputData( sessionNo, gameTime + localDelayTime - 4, myInfo.playerSide, &sendBuf[3] );
							}else{
								sendBuf[3] = 0;
							}
							if( gameTime + localDelayTime - 2 >= 0 ){
								inputData.GetInputData( sessionNo, gameTime + localDelayTime - 2, myInfo.playerSide, &sendBuf[2] );
							}else{
								sendBuf[2] = 0;
							}
							if( gameTime + localDelayTime >= 0 ){
								inputData.GetInputData( sessionNo, gameTime + localDelayTime , myInfo.playerSide, &sendBuf[1] );
							}else{
								sendBuf[1] = 0;
							}
							inputData.GetInputData( sessionNo, gameTime + localDelayTime + 2, myInfo.playerSide, &sendBuf[0] );

							*(DWORD*)&sendBufBody[2] = gameTime + localDelayTime + 2;
							SendCmd( dest_away, cmd_sendinput, sendBufBody, 12 );

						}else{
							//none
							//delayTime = delayTimeNew;
						}
					}
					
					if (delayTime != delayTimeOld) {
						cout << "setting:  Local buffer : " << delayTime / 2 << endl;
					}
					
					localDelayTime = tempLocalDelayTime;
					rewindAmount = tempRewindAmount;
					
					if (rollbackOk) {
						cout << "           Input delay : " << localDelayTime/2 << endl;
						cout << "         Rewind frames : " << rewindAmount/2 << endl;
					}
				}
			}
			
			if( pauseFlg ){
				UnRockTime();
				if( fastFlg ){
					WriteCode( (void *)0x66C23C, 16);
					fastFlg = 0;
				}


				//要検討
				BYTE menuIndex;
				if( menuAddress ){
					ReadMemory( (void*)(menuAddress + 0x118) , &menuIndex , 1 );
				}else{
					menuIndex = 0;
				}

				if( menuIndex == 0 ){
					//ゲーム再開
					//none
				}else if( menuIndex == 1 ){
					//ゲーム終了
					if( diInputA != key_P ){
						if( diInputA & key_A || diInputB & key_A ){
							break;
						}
					}
				}else if( menuIndex == 2 ){
					//リプレイを保存して終了
					if( diInputA != key_P ){
						if( diInputA & key_A || diInputB & key_A ){
							break;
						}
					}
				}
			}else if( isRewinding && endOfMatch != 999999999 && (gameTime + 2 - endOfMatch) >= rewindAmount ) {
				UnRockTime();
			}else if( rockFlg ){
				//ロック中
				if( gameTime <= 200 ){
					UnRockTime();
				}else{
					if( diInputA == 0xFF || diInputB == 0xFF ){	//0xFF == key_P
						//ポーズにする
						//フォアグラウンドのときだけ入力
						//要検討
						//余分な入力が入る余地がある
						UnRockTime();
					}else{
						if( !inputData.GetInputDataA( sessionNo, syncFrame, NULL ) && !inputData.GetInputDataB( sessionNo, syncFrame, NULL ) ){
							//通常へ
							UnRockTime();
						}else{
							//データが届いていないとき
//							if( datA.GetInputBuf( gameTime + 2, &InputA ) ) cout << "error at a." << gameTime + 2 << endl;
//							if( datB.GetInputBuf( gameTime + 2, &InputB ) ) cout << "error at b." << gameTime + 2 << endl;

							if( myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug){
								BYTE mySide = 0;
								BYTE enSide = 0;
								DWORD myTime = syncFrame;
								DWORD enTime = syncFrame;
								if( myInfo.playerSide == 0xA ){
									mySide = 0xA;
									enSide = 0xB;
								}else if( myInfo.playerSide == 0xB ){
									mySide = 0xB;
									enSide = 0xA;
								}
								if( enTime > endOfMatch ) {
									enTime = endOfMatch;
								}
								if( myTime > endOfMatch ) {
									myTime = endOfMatch;
								}
								if( mySide ){
									roopCounterSend += 1;
									if( roopCounterSend > 3 ){
										roopCounterSend = 0;
										while (enTime <= syncEndFrame) { 
											if( inputData.GetInputData( sessionNo, enTime, enSide, NULL ) ){
												BYTE data[6];
												data[0] = sessionNo;
												data[1] = sessionID;
												*(DWORD*)&data[2] = enTime;
												SendCmd( dest_away, cmd_input_req, data, 6 );
											}
											enTime += 2;
										}
									}
									if( inputData.GetInputData( sessionNo, myTime, mySide, NULL ) ){
										//破綻への対処
										cout << "ERROR : recover" << endl;
										inputData.SetInputData( sessionNo, myTime, mySide, 0 );
									}
								}
							}else if( myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch ){

								//観戦しててタイムアウトしたときはroopFlgが1のままのため
								if( !Root.sin_addr.s_addr ){
									roopFlg = 0;
								}else{
									if( roopCounterReq > 15 ){
										if( zlibFlg ){
											BYTE data[7];
											data[0] = sessionNo;
											data[1] = sessionID;
											*(DWORD*)&data[2] = syncFrame;
											data[6] = 0;	//仕様を拡張するため
											SendCmd( dest_root, cmd_inputdata_req, data, 7 );
										}else{
											BYTE data[6];
											data[0] = sessionNo;
											data[1] = sessionID;
											*(DWORD*)&data[2] = syncFrame;
											SendCmd( dest_root, cmd_inputdata_req, data, 6 );
										}

										roopCounterReq = 0;
									}
									roopCounterReq++;
								}
							}else if( myInfo.terminalMode == mode_broadcast ){
								//破綻への対処
								inputData.SetInputDataA( sessionNo, gameTime + 2, diInputA );
								inputData.SetInputDataB( sessionNo, gameTime + 2, diInputB );
							}
						}
					}
				}
			}else{
				if( diInputA == 0xFF || diInputB == 0xFF ){
					//none
				}else{
					DWORD timeA = gameTime + 2;
					DWORD timeB = timeA;
					bool hasFrameData = true;
					
					syncEndFrame = timeA;
					if (syncEndFrame > endOfMatch) {
						syncEndFrame = endOfMatch;
					}
										
					if (rollbackOk) {
						if (myInfo.playerSide == 0xA) {
							timeB -= rewindAmount;
							if (timeB > endOfMatch) {
								timeB = endOfMatch;
							}
							syncEndFrame = timeB;
						} else if (myInfo.playerSide == 0xB) {
							timeA -= rewindAmount;
							if (timeA > endOfMatch) {
								timeA = endOfMatch;
							}
							syncEndFrame = timeA;
						}
					}

					if (((myInfo.playerSide == 0xA) && inputData.GetInputDataA(sessionNo, timeA, NULL))
 					    || ((myInfo.playerSide == 0xB) && inputData.GetInputDataB(sessionNo, timeB, NULL))) {
						hasFrameData = false;
					} else {
						if (myInfo.playerSide == 0xA) {
							while (syncFrame <= syncEndFrame) {
								if (inputData.GetInputDataB(sessionNo, syncFrame, NULL)) {
									hasFrameData = false;
									break;
								}
								syncFrame += 2;
							}
						} else if (myInfo.playerSide == 0xB) {
							while (syncFrame <= syncEndFrame) {
								if (inputData.GetInputDataA(sessionNo, syncFrame, NULL)) {
									hasFrameData = false;
									break;
								}
								syncFrame += 2;
							}
						} else {
							while (syncFrame <= syncEndFrame) {
								if (inputData.GetInputDataA(sessionNo, syncFrame, NULL)
								    || inputData.GetInputDataB(sessionNo, syncFrame, NULL)) {
									hasFrameData = false;
									break;
								}
								syncFrame += 2;
							}
						}
					}

					if (hasFrameData) {
						//none, check for grace period updates
						if (rollbackOk) {
							if (graceWait > 0) {
								graceWait -= 2;
							} else if (gracePeriod != 0) {
								gracePeriod = 0;
								graceWait = 2;
							}
						}
					}else if (rollbackOk && gameTime > 202 && myInfo.terminalMode == mode_root && gracePeriod == 0 && graceWait == 0) {
						gracePeriod = 2;
					}else {
						if( gameTime > 200 ){
							if( perfectFreezeFlg == 2 ) { //|| (perfectFreezeFlg == 1 && (myInfo.A.ID == 8 || myInfo.B.ID == 8) ) ){
//								cout << "debug : PF" << endl;
								int Counter = 0;
								for(;;){
									if( !roopFlg ){
										if( fastFlg ) WriteCode( (void *)0x66C23C, 16);
										UnRockTime();
										return 1;
									}
									if( fastFlg ) {
										WriteCode( (void *)0x66C23C, 16);
										fastFlg = 0;
									}

									if (myInfo.terminalMode == mode_root) {
										WaitForSingleObject( hInputRecvEvent, 16 );
									} else {
										Sleep(20);
									}

									if( !inputData.GetInputDataA( sessionNo, syncFrame, NULL )
									 && !inputData.GetInputDataB( sessionNo, syncFrame, NULL ) ){
										break;
									}
									if( myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug){
										if( Counter > 4 ){
											BYTE mySide = 0;
											BYTE enSide = 0;
											DWORD timeReq = syncFrame;
											DWORD myTime = syncFrame;
											if( myInfo.playerSide == 0xA ){
												mySide = 0xA;
												enSide = 0xB;
											}else if( myInfo.playerSide == 0xB ){
												mySide = 0xB;
												enSide = 0xA;
											}
											if( timeReq > endOfMatch ) {
												timeReq = endOfMatch;
											}
											if( myTime > endOfMatch ) {
												myTime = endOfMatch;
											}
											if( mySide ){
												roopCounterSend++;
												if( roopCounterSend > 3 ){
													roopCounterSend = 0;
													while (timeReq <= syncEndFrame) {
														if( inputData.GetInputData( sessionNo, timeReq, enSide, NULL ) ){
															BYTE data[6];
															data[0] = sessionNo;
															data[1] = sessionID;
															*(DWORD*)&data[2] = timeReq;
															SendCmd( dest_away, cmd_input_req, data, 6 );
															roopCounterSend = 0;
														}
														timeReq += 2;
													}
												}
												if( inputData.GetInputData( sessionNo, myTime, mySide, NULL ) ){
													//破綻への対処
													cout << "ERROR : recover" << endl;
													inputData.SetInputData( sessionNo, myTime, mySide, 0 );
												}
											}
											Counter = 0;
										}
									}else if( myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch ){
										if( Counter > 30 ){
											//観戦しててタイムアウトしたときはroopFlgが1のままのため
											if( !Root.sin_addr.s_addr ){
												roopFlg = 0;
											}else{
												if( zlibFlg ){
													BYTE data[7];
													data[0] = sessionNo;
													data[1] = sessionID;
													*(DWORD*)&data[2] = syncFrame;
													data[6] = 0;	//仕様を拡張するため
													SendCmd( dest_root, cmd_inputdata_req, data, 7 );
												}else{
													BYTE data[6];
													data[0] = sessionNo;
													data[1] = sessionID;
													*(DWORD*)&data[2] = syncFrame;
													SendCmd( dest_root, cmd_inputdata_req, data, 6 );
												}
											}
											Counter = 0;
										}
									}else if( myInfo.terminalMode == mode_broadcast ){
										//破綻への対処
										inputData.SetInputDataA( sessionNo, gameTime + 2, diInputA );
										inputData.SetInputDataB( sessionNo, gameTime + 2, diInputB );
										break;
									}
									Counter++;
								}
							}else{
								RockTime();
							}
						}else{
							//none
						}
					}
				}
			}

			if( rockFlg ){
				diInputA = 0;
				diInputB = 0;
			}

			//bodyの入力
			datA.SetBodyInput( diInputA );
			datB.SetBodyInput( diInputB );

		}else if( deInfo == de_char ){
			//charの入力
			ReadMemory( (void*)0x6718B4, &gameTime, 4 );
			if( (DWORD)( gameTime / 2 ) * 2 != gameTime ) gameTime--;

			//test
			roopCounterSend = 0;
			roopCounterReq = 0;

			/*
			{
				//debug
				BYTE stateA;
				BYTE stateB;
				ReadMemory( (void*)( FlgA + 0x60 ) , &stateA , 1 );
				ReadMemory( (void*)( FlgB + 0x60 ) , &stateB , 1 );
				cout << gameTime << hex << " : " << (WORD)stateA << " " << (WORD)stateB << endl;
			}
			*/
			
			DWORD timeA = gameTime + 2;
			DWORD timeB = timeA;
			if(rollbackOk) {
				if (myInfo.playerSide == 0xA) {
					while( inputData.GetInputData( sessionNo, timeB, 0xB, NULL ) && timeB > 0){
						timeB -= 2;
					}
				} else if (myInfo.playerSide == 0xB) {
					while( inputData.GetInputData( sessionNo, timeA, 0xA, NULL ) && timeA > 0){
						timeA -= 2;
					}
				}
			}
			
			//sync data
			if( gameTime > 100 && !(gameTime % 20) && FlgA && FlgB && rewindAmount){
				float XATemp = 0;
				float XBTemp = 0;
				HPA = 0;
				HPB = 0;

				ReadMemory( (void*)(FlgA + 0x324) , &HPA , 2 );
				ReadMemory( (void*)(FlgA + 0x44) , &XATemp , 4 );

				ReadMemory( (void*)(FlgB + 0x324) , &HPB , 2 );
				ReadMemory( (void*)(FlgB + 0x44) , &XBTemp , 4 );

				XA = (DWORD)(XATemp * 64);
				XB = (DWORD)(XBTemp * 64);

				syncData.SetSyncDataHereA( gameTime, HPA, XA );
				syncData.SetSyncDataHereB( gameTime, HPB, XB );
			}
			
			//対処（ポーズの後など）
			if( gameTime > 200 ){
				if( inputData.GetInputData( sessionNo, timeA, 0xA, NULL ) ){
					BYTE inputTemp;
					inputData.GetInputData( sessionNo, gameTime, 0xA, &inputTemp );
					inputData.SetInputData( sessionNo, gameTime + 2, 0xA, inputTemp );
					cout << "ERROR : inputData check" << endl;
				}
				if( inputData.GetInputData( sessionNo, timeB, 0xB, NULL ) ){
					BYTE inputTemp;
					inputData.GetInputData( sessionNo, gameTime, 0xB, &inputTemp );
					inputData.SetInputData( sessionNo, gameTime + 2, 0xB, inputTemp );
					cout << "ERROR : inputData check" << endl;
				}
			}

			//観戦データ送信頻度減少
			//現状では環境が悪い
			if( gameTime > gameTimeNoCast && (!lessCastFlg || gameTime / 2 & 1) ) {
				DWORD offTime = gameTime;
				if (rollbackOk) {
					offTime -= rewindAmount + 2;
					offTime -= gracePeriod;
				}
				//発信
				//side A
				if( offTime - 8 >= 0 ){
					inputData.GetInputData( sessionNo, offTime - 8, 0xA, &castBufA[5] );
				}else{
					castBufA[5] = 0;
				}

				if( offTime - 6 >= 0 ){
					inputData.GetInputData( sessionNo, offTime - 6, 0xA, &castBufA[4] );
				}else{
					castBufA[4] = 0;
				}
				if( offTime - 4 >= 0 ){
					inputData.GetInputData( sessionNo, offTime - 4, 0xA, &castBufA[3] );
				}else{
					castBufA[3] = 0;
				}
				if( offTime - 2 >= 0 ){
					inputData.GetInputData( sessionNo, offTime - 2, 0xA, &castBufA[2] );
				}else{
					castBufA[2] = 0;
				}
				if( offTime >= 0 ){
					inputData.GetInputData( sessionNo, offTime, 0xA, &castBufA[1] );
				}else{
					castBufA[1] = 0;
				}
				inputData.GetInputData( sessionNo, offTime + 2, 0xA, &castBufA[0] );

				//side B
				if( offTime - 8 >= 0 ){
					inputData.GetInputData( sessionNo, offTime - 8, 0xB, &castBufB[5] );
				}else{
					castBufB[5] = 0;
				}
				if( offTime - 6 >= 0 ){
					inputData.GetInputData( sessionNo, offTime - 6, 0xB, &castBufB[4] );
				}else{
					castBufB[4] = 0;
				}
				if( offTime - 4 >= 0 ){
					inputData.GetInputData( sessionNo, offTime - 4, 0xB, &castBufB[3] );
				}else{
					castBufB[3] = 0;
				}
				if( offTime - 2 >= 0 ){
					inputData.GetInputData( sessionNo, offTime - 2, 0xB, &castBufB[2] );
				}else{
					castBufB[2] = 0;
				}
				if( offTime >= 0 ){
					inputData.GetInputData( sessionNo, offTime, 0xB, &castBufB[1] );
				}else{
					castBufB[1] = 0;
				}
				inputData.GetInputData( sessionNo, offTime + 2, 0xB, &castBufB[0] );
				*(DWORD*)&castBufBody[2] = offTime + 2;
				SendCmd( dest_leaf, cmd_cast, castBufBody, 18 );
				
				gameTimeNoCast = gameTime;
			}else if (gameTime > gameTimeNoCast) {
				gameTimeNoCast = gameTime;
			}
			
			//入力
			if( gameTime <= 200 ){
				InputA = 0;
				InputB = 0;
			}else{
				inputData.GetInputDataA( sessionNo, timeA, &InputA );
				inputData.GetInputDataB( sessionNo, timeB, &InputB );
			}
			
			int n = ((gameTime)/2)%20;
			memcpy(datA.inputBufChar, inputBufCache[0][n], 7*4);
			memcpy(datB.inputBufChar, inputBufCache[1][n], 7*4);
			
			rollbackInput[0][n] = InputA;
			rollbackInput[1][n] = InputB;
			
			datA.SetCharInput( InputA );
			datB.SetCharInput( InputB );
			
			n = (n + 1) % 20;

			memcpy(inputBufCache[0][n], datA.inputBufChar, 7*4);
			memcpy(inputBufCache[1][n], datB.inputBufChar, 7*4);

			if (infiniteSpiritCheat) {
				WORD thousand = 1000;
				WORD zero = 0;

				ReadMemory( (void*)0x671418 , &FlgA , 4 );
				ReadMemory( (void*)0x67141C , &FlgB , 4 );

				if (FlgA && FlgB) {
					WriteMemory( (void*)(FlgA + 0x3B0), &thousand, 2 );
					WriteMemory( (void*)(FlgA + 0x3B2), &thousand, 2 );
					WriteMemory( (void*)(FlgA + 0x3B4), &zero, 2 );
					WriteMemory( (void*)(FlgA + 0x3B6), &zero, 2 );
					WriteMemory( (void*)(FlgB + 0x3B0), &thousand, 2 );
					WriteMemory( (void*)(FlgB + 0x3B2), &thousand, 2 );
					WriteMemory( (void*)(FlgB + 0x3B4), &zero, 2 );
					WriteMemory( (void*)(FlgB + 0x3B6), &zero, 2 );
				}
			}
		}
	}
	
	if( fastFlg ) WriteCode( (void *)0x66C23C, 16);
	UnRockTime();

	myInfo.A.phase = 0;
	myInfo.B.phase = 0;

	dataInfo.A.phase = 0;
	dataInfo.B.phase = 0;

	dataInfo.phase = phase_default;

	return 0;
}

