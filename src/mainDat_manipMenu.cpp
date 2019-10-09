#include "mainDatClass.h"
using namespace std;

BYTE ManipMenuSub( charInfoStruct* now, charInfoStruct* dest){
	if(now->phase == 4) { now->placeTime = (now->placeTime % 50); }

	if(dest->phase > now->phase){
		switch( now->phase ){
		case 0 :
			//まずキャラの位置を合わせる
			if( dest->ID != now->ID ){
				if(dest->ID > now->ID){
					if( dest->ID > now->ID + 5 ){
						return key_left;
					}else{
						return key_right;
					}
				}else{
					if( dest->ID + 5 < now->ID ){
						return key_right;
					}else{
						return key_left;
					}
				}
			}else{
				//決定
				if (dest->color & 1) {
					return key_D;
				} else {
					return key_A;
				}
			}
		case 1 :
			if( dest->ID != now->ID ){
				return key_B;
			}
			if(dest->firstSpell != now->spellPlace){
				switch( dest->firstSpell ){
				case 0 :
					return key_up;
				case 1 :
					return key_left;
				case 2 :
					return key_down;
				}
			}else{
				//決定
				return key_A;
			}
		case 2 :
			if( dest->ID != now->ID ){
				return key_B;
			}
			if( dest->firstSpell != now->firstSpell ){
				return key_B;
			}
			if(dest->secondSpell != now->spellPlace){
				// Only change if not confirmed
				if( abs(dest->placeTime-now->placeTime)>10){
					return key_D;
				}

				switch( dest->secondSpell ){
				case 0 :
					return key_up;
				case 1 :
					return key_left;
				case 2 :
					return key_down;
				}
			}else{
				//決定
				return key_A;
			}
		case 4 :
			//場所
			if( dest->ID != now->ID ){
				return key_B;
			}
			if( dest->firstSpell != now->firstSpell ){
				return key_B;
			}
			if(dest->secondSpell != now->secondSpell ){
				return key_B;
			}
			if(dest->place != now->place){
//				return key_right;

				if(dest->place > now->place){
					return key_right;
				}else{
					return key_left;
				}
			}else{
				if(dest->placeTime != now->placeTime){
					return key_D;
				}else{
					return key_A;
				}
			}
		}
	}

	if(dest->phase < now->phase){
		//キャンセル
		return key_B;
	}

	if(dest->phase == now->phase){
		//カーソル移動だけ
		switch( now->phase ){
		case 0 :
			//キャラ横方向
			if(dest->ID != now->ID){
				if(dest->ID > now->ID){
					if( dest->ID > now->ID + 5 ){
						return key_left;
					}else{
						return key_right;
					}
				}else{
					if( dest->ID + 5 < now->ID ){
						return key_right;
					}else{
						return key_left;
					}
				}
			}
			break;
		case 1 :
			if(dest->ID != now->ID){
				return key_B;
			}
			//スペル縦方向
				if( abs(dest->placeTime-now->placeTime)>10){
					return key_D;
				}
			if(dest->spellPlace != now->spellPlace){
				switch( dest->spellPlace ){
				case 0 :
					return key_up;
				case 1 :
					return key_left;
				case 2 :
					return key_down;
				}
			}
			break;
		case 2 :
			if( dest->ID != now->ID ){
				return key_B;
			}
			if( dest->firstSpell != now->firstSpell ){
				return key_B;
			}
				if( abs(dest->placeTime-now->placeTime)>10){
					return key_D;
				}
			//スペル縦方向
			if(dest->spellPlace != now->spellPlace){
				switch( dest->spellPlace ){
				case 0 :
					return key_up;
				case 1 :
					return key_left;
				case 2 :
					return key_down;
				}
			}
			break;
		case 4 :
			if( dest->ID != now->ID ){
				return key_B;
			}
			if( dest->firstSpell != now->firstSpell ){
				return key_B;
			}
			if(dest->secondSpell != now->secondSpell ){
				return key_B;
			}
			//場所
			if(dest->place != now->place){
				if(dest->place > now->place){
					if(dest->place > now->place + 4){
						return key_left;
					}else{
						return key_right;
					}
				}else{
					if(dest->place + 4 < now->place){
						return key_right;
					}else{
						return key_left;
					}
				}
			}else{
				if(dest->placeTime != now->placeTime){
					return key_D;
				}
			}
			break;
		}
	}

	return 0;
}



int mainDatClass::ManipMenu(){
	#if debug_mode_mainRoop
		WaitForSingleObject( hPrintMutex, INFINITE );
		cout << "debug : ManipMenu()" << endl;
		ReleaseMutex( hPrintMutex );
	#endif

	memset( &enInfo, 0, sizeof(enInfo));

	DWORD menuAddress = 0;	//0x12FD14			//DWORD
	DWORD menuFlg = 0;	//0x12FE5C			//BYTE
	DWORD gameMode = 0;	//0x671418 + 0x218	//BYTE
	DWORD battleFlgA = 0;	//0x671418			//DWORD
	DWORD battleFlgB = 0;	//0x67141C			//DWORD

	WORD pushFlg = 0;
	BYTE InputA;
	BYTE InputB;

	myInfo.A.phase = 0;
	myInfo.B.phase = 0;

	enInfo.A.phase = 0;
	enInfo.B.phase = 0;

	dataInfo.A.phase = 0;
	dataInfo.B.phase = 0;

	myInfo.A.color = 0;
	myInfo.B.color = 0;

	datA.SetBodyInput( 0 );
	datB.SetBodyInput( 0 );

	myInfo.gameTime = 0;
	enInfo.gameTime = 0;
	dataInfo.gameTime = 0;

	DWORD paletteReqCounter = 0;

	//test
	DWORD menuAddressTemp = 0;

	//緩和
	DWORD roopCounter = 0;
	DWORD stageRandCounter = 0;


	//HWND hWnd = FindWindow( NULL , windowName );

	//ステージ選択の異常に応急措置
	WORD placeRecoverFlg = 0;

	//ステージ選択を自由に
	WORD placeFreeFlg = 0;

	DWORD keyCancelCounter = 0;

	DWORD deInfo;
	for(;;){
		if( th075Roop( &deInfo ) ) return 0xF;
		if( !roopFlg ){
			UnRockTime();
			return 1;
		}
		if( deInfo == de_body ){
			myInfo.phase = phase_menu;
			InputA = 0;
			InputB = 0;
			DWORD lastMenuFlg = menuFlg;
//			ReadProcessMemory( hProcess, (void*)(0x12FE5C) , &menuFlg , 1 , NULL );
			ReadProcessMemory( hProcess, (void *)((DWORD)pStackBase - 420) , &menuFlg , 1 , NULL );
			ReadProcessMemory( hProcess, (void*)(0x671418 + 0x218) , &gameMode , 1 , NULL );
			ReadProcessMemory( hProcess, (void*)(0x671418) , &battleFlgA , 4 , NULL );
			ReadProcessMemory( hProcess, (void*)(0x67141C) , &battleFlgB , 4 , NULL );
			
			if(menuFlg != 3 && menuFlg != 1 && battleFlgA==0 && battleFlgB==0){
				datA.SetInput( 0 );
				datB.SetInput( 0 );
				return 1;
			}else{
				if( menuFlg == 3 ){
					ReadProcessMemory( hProcess, (void *)((DWORD)pStackBase - 748) , &menuAddress , 4 , NULL );
					if (lastMenuFlg != menuFlg) {
						WriteProcessMemory( hProcess, (void*)(menuAddress + 0xE0), &lastCharacterA, 1, NULL);
						WriteProcessMemory( hProcess, (void*)(menuAddress + 0xE1), &lastCharacterB, 1, NULL);
					}
//					ReadProcessMemory( hProcess, (void*)(0x12FD14) , &menuAddress , 4 , NULL );
					ReadProcessMemory( hProcess, (void*)(0x671418 + 0x1F8) , &myInfo.A.firstSpell  ,1 ,NULL );
					ReadProcessMemory( hProcess, (void*)(0x671418 + 0x1F9) , &myInfo.B.firstSpell  ,1 ,NULL );
					ReadProcessMemory( hProcess, (void*)(0x671418 + 0xB4)  , &myInfo.A.secondSpell ,1 ,NULL );
					ReadProcessMemory( hProcess, (void*)(0x671418 + 0xB5)  , &myInfo.B.secondSpell ,1 ,NULL );

					if(menuAddress){
						if( !menuAddressTemp ) menuAddressTemp = menuAddress;
						if( menuAddress == menuAddressTemp ){
							ReadProcessMemory( hProcess, (void*)(menuAddress + 0x96), &myInfo.A.phase , 1 , NULL );
							ReadProcessMemory( hProcess, (void*)(menuAddress + 0x97), &myInfo.B.phase , 1 , NULL );
							ReadProcessMemory( hProcess, (void*)(menuAddress + 0xE0), &myInfo.A.ID , 1 , NULL );
							ReadProcessMemory( hProcess, (void*)(menuAddress + 0xE1), &myInfo.B.ID , 1 , NULL );
							ReadProcessMemory( hProcess, (void*)(menuAddress + 0xE4), &Ahidden , 1 , NULL );
							ReadProcessMemory( hProcess, (void*)(menuAddress + 0xE5), &Bhidden , 1 , NULL );
							ReadProcessMemory( hProcess, (void*)(menuAddress + 0xE6), &myInfo.A.spellPlace , 1 , NULL );
							ReadProcessMemory( hProcess, (void*)(menuAddress + 0xE7), &myInfo.B.spellPlace , 1 , NULL );

							BYTE Acolor,Bcolor;
							ReadProcessMemory( hProcess, (void*)(0x6714AC), &Acolor , 1 , NULL );
							ReadProcessMemory( hProcess, (void*)(0x6714AD), &Bcolor , 1 , NULL );

							if (myInfo.A.phase > 0) {
								myInfo.A.color = (myInfo.A.color&~1) | (Acolor&1);
							} else {
								myInfo.A.color = Acolor;
							}
							if (myInfo.B.phase > 0) {
								myInfo.B.color = (myInfo.B.color&~1) | (Bcolor&1);
							} else {
								myInfo.B.color = Bcolor;
							}

							if( placeRecoverFlg ){
								//manual
								myInfo.place = enInfo.place;
								myInfo.placeTime = enInfo.placeTime;
							}else{
								ReadProcessMemory( hProcess, (void*)(menuAddress + 0xF1), &myInfo.place , 1 , NULL );
								ReadProcessMemory( hProcess, (void*)(menuAddress + 0xF0), &myInfo.placeTime , 1 , NULL );
							}
							myInfo.A.place = myInfo.place;
							myInfo.B.place = myInfo.place;
							myInfo.A.placeTime = myInfo.placeTime;
							myInfo.B.placeTime = myInfo.placeTime;

							lastCharacterA = myInfo.A.ID;
							lastCharacterB = myInfo.B.ID;

							//Should this go here?
							if(myInfo.A.phase!=4) {
								if(myInfo.A.placeTime>40 && myInfo.A.placeTime<60) {
									if(Ahidden==1) { myInfo.A.placeTime-=50; }
								}
								if(myInfo.A.placeTime<10) { myInfo.A.placeTime+=(1-Ahidden)*50; }
							} else {
								myInfo.A.placeTime = myInfo.A.placeTime % 50;
							}
							if(myInfo.B.phase!=4) {
								if(myInfo.B.placeTime>40 && myInfo.B.placeTime<60) {
									if(Bhidden==1) { myInfo.B.placeTime-=50; }
								}
								if(myInfo.B.placeTime<10) { myInfo.B.placeTime+=(1-Bhidden)*50; }
							} else {
								myInfo.B.placeTime = myInfo.B.placeTime % 50;
							}

							//debug
//							cout << (WORD)myInfo.place << " " << (WORD)myInfo.placeTime << endl;

						}else{
//							cout << "debug : menuAddress != menuAddressTemp" << endl;
						}
					}
				}

				if( myInfo.terminalMode == mode_broadcast || myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug ){
					dataInfo = myInfo;
				}

				if (keystate[KEY_REMOTE_PALETTE_CYCLE] == 1) {
					processRemotePalettesFlg = (processRemotePalettesFlg+1)%3;
					cout << "setting : processRemotePalettes " << (int)processRemotePalettesFlg << " : ";
					if (processRemotePalettesFlg == 1) {
						cout << "Use local palettes" << endl;
					} else if (processRemotePalettesFlg == 2) {
						cout << "Strip custom palette choices" << endl;
					} else {
						cout << "Use remote palettes" << endl;
					}
				}

				if( myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug ){
					//ルートの場合
					if( enInfo.phase == phase_battle || enInfo.phase == phase_read ){
						enInfo.A.phase = 5;
						enInfo.B.phase = 5;
					}

					//データを要求
					if (myInfo.terminalMode == mode_debug) {
						enInfo = myInfo;
					} else {
						SendCmd( dest_away, cmd_gameInfo, (void *)cowcaster_id, 5 );
					}

					//sessionNoで対応
//					if( enInfo.gameTime > 2 ) return 1;

					//要検討
//					cout << menuFlg << " " << battleFlgA << " " << battleFlgB << endl;

					//ステージランダム後はしばらく決定できないようにする
					if( stageRandCounter ) stageRandCounter++;
					if( stageRandCounter > 36 ) stageRandCounter = 0;

					//stageRecover
					if( myInfo.playerSide == 0xB ){
						if( !placeRecoverFlg && keystate[KEY_STAGE_MANUAL] == 1 ){
							cout << "setting : Stage Manual - 2P can now choose its own stage" << endl;
							placeRecoverFlg = 1;
						}
					}

					//free
					if( myInfo.playerSide == 0xA ){
						if( !placeFreeFlg && keystate[KEY_STAGE_FREE] == 1 ){
							cout << "setting : Stage Free - banned stages now available" << endl;
							placeFreeFlg = 1;
						}
					}

					//delay
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

					if( delayTimeNew ){
						//calcRewind(delayTimeNew, &localDelayTime, &rewindAmount);
						HWND hFocus = GetForegroundWindow();
						if( !hFocus || hWnd == hFocus ){
							bool print = 0;
							if( keystate[KEY_REWIND_MODIFIER] != 0 && myInfo.terminalMode == mode_root) {
								double newRewind = delayTimeNew;
								if (newRewind > delayTime) {
									cout << "Limiting rewind value to buffer." << endl;
									newRewind = delayTime;
								}
								
								localDelayTime = delayTime - newRewind;
								rewindAmount = newRewind;
								
								print = 1;
							} else if( delayTime != delayTimeNew ){
								calcRewind(delayTimeNew, &localDelayTime, &rewindAmount);
								
								delayTime = delayTimeNew;
								cout << "setting:  Local buffer : " << delayTime / 2 << endl;
								print = 1;
							}
							
							if (print) {
								cout << "           Input delay : " << localDelayTime/2 << endl;
								cout << "         Rewind frames : " << rewindAmount/2 << endl;
							}
						}
					}

					if(menuFlg == 3 && battleFlgA == 0 && battleFlgB == 0 ){
						if( myInfo.playerSide == 0xA ){
							InputA = datA.GetInput();

							if( myInfo.terminalMode == mode_debug ){
								if( myInfo.A.phase > 3 && myInfo.B.phase > 3 ){
									if( keyCancelCounter > 40 ){
										//none
									}else{
										InputA = 0;
										keyCancelCounter++;
									}
								}
							}


							//前面かどうか
							if( datA.inputDeviceType == 0xFF ){
								HWND hFocus = GetForegroundWindow();
								if( hFocus && hWnd != hFocus ){
									InputA = 0;
								}
							}

							InputB = ManipMenuSub( &myInfo.B, &enInfo.B );
							if( myInfo.B.phase == 4 && InputB != key_B ){
								InputB = 0;
							}

							//stageRand
							if( myInfo.A.phase == 4 && myInfo.B.phase > 3 && InputA & key_C ){
								stageRandCounter = 1;
							}
							if( stageRandCounter && InputA & key_A && InputA != key_P ){
								InputA = 0;
							}

							if( !placeFreeFlg ){
								//いくつかの場所を選択不可にする
								if( myInfo.A.phase == 4 && InputA & key_A ){
									if( myInfo.place == 9	//幻想郷
									|| (myInfo.place == 5 && myInfo.placeTime == 0)		//大木のある墓地昼
									|| (myInfo.place == 2 && myInfo.placeTime == 0)	)	//時計台昼
									{
										InputA = 0;
									}
								}
							}

							if( myInfo.A.ID != enInfo.B.ID ){
								if( myInfo.B.color&1 != enInfo.B.color&1 ){
									BYTE dataTemp = enInfo.B.color & 1;
									WriteProcessMemory( hProcess, (void*)0x6714AD, &dataTemp, 1 , NULL );
								}
							}

							if (myInfo.terminalMode == mode_root) {
								if (enInfo.B.phase > 0 && (enInfo.B.color & 2)) {
									myInfo.B.color = (myInfo.B.color&1) | (enInfo.B.color&~1);
								} else {
									myInfo.B.color &= 1;
								}
							}
						}
						if( myInfo.playerSide == 0xB ){
							//テスト
							if( enInfo.A.place > 10 ){
								enInfo.A.place = 0;
								enInfo.A.placeTime = 0;
							}

							InputA = ManipMenuSub( &myInfo.A, &enInfo.A );

							InputB = datA.GetInput();

							//前面かどうか
							if( datA.inputDeviceType == 0xFF ){
								HWND hFocus = GetForegroundWindow();
								if( hFocus && hWnd != hFocus ){
									InputB = 0;
								}
							}
							if( myInfo.B.phase == 4 && InputB != key_B ){
								if( !placeRecoverFlg ){
									InputB = 0;
								}else{
									if( InputB & key_A ){
										InputB = 0;
									}
								}
							}

							if (myInfo.terminalMode == mode_root) {
								if (enInfo.A.phase > 0 && (enInfo.A.color & 2)) {
									myInfo.A.color = (myInfo.A.color&1) | (enInfo.A.color&~1);
								} else {
									myInfo.A.color &= 1;
								}
							}
						}

						if( ( enInfo.phase == phase_battle || enInfo.phase == phase_read && myInfo.A.phase == 4 && myInfo.B.phase == 4 ) && myInfo.sessionNo == enInfo.sessionNo ){
							//検証する
							if( myInfo.A.ID == enInfo.A.ID && myInfo.A.firstSpell == enInfo.A.firstSpell && myInfo.A.secondSpell == enInfo.A.secondSpell
							 && myInfo.B.ID == enInfo.B.ID && myInfo.B.firstSpell == enInfo.B.firstSpell && myInfo.B.secondSpell == enInfo.B.secondSpell
							 && myInfo.place == enInfo.place && myInfo.placeTime == enInfo.placeTime )
							{
								if( myInfo.playerSide == 0xB ){
									//Color
									//できれば入力で対応したいところ
									BYTE dataTemp;
									if ( !(enInfo.A.color & 2) && !(enInfo.B.color & 2) ) {
										if( enInfo.A.color & 1 ){
											//2P
											dataTemp = 1;
											WriteProcessMemory( hProcess, (void*)0x6714AC, &dataTemp, 1 , NULL );
										}else{
											dataTemp = 0;
											WriteProcessMemory( hProcess, (void*)0x6714AC, &dataTemp, 1 , NULL );
										}
										if( enInfo.B.color & 1 ){
											dataTemp = 1;
											WriteProcessMemory( hProcess, (void*)0x6714AD, &dataTemp, 1 , NULL );
										}else{
											dataTemp = 0;
											WriteProcessMemory( hProcess, (void*)0x6714AD, &dataTemp, 1 , NULL );
										}
									}
								}
								InputA = key_A;
							}
						}
					}else{
						datA.SetInput( 0 );
						datB.SetInput( 0 );

						if( myInfo.playerSide == 0xA ){
							break;
						}else{
							//検証する
							if( myInfo.A.ID == enInfo.A.ID && myInfo.A.firstSpell == enInfo.A.firstSpell && myInfo.A.secondSpell == enInfo.A.secondSpell
							 && myInfo.B.ID == enInfo.B.ID && myInfo.B.firstSpell == enInfo.B.firstSpell && myInfo.B.secondSpell == enInfo.B.secondSpell
							 && myInfo.place == enInfo.place && myInfo.placeTime == enInfo.placeTime )
							{
								//次のステップへ
								break;
							}else{
								return 1;
							}
						}
					}


					//連打修正
					if( pushFlg ){
						if( myInfo.playerSide == 0xA ) InputB = 0;
						if( myInfo.playerSide == 0xB ) InputA = 0;
					}

					//sessionNo
					if( myInfo.sessionNo != enInfo.sessionNo ){
						if( myInfo.playerSide == 0xA ){
							InputB = 0;
						}
						if( myInfo.playerSide == 0xB ){
							InputA = 0;
						}
					}

				}else if( myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch ){
					//転送の場合
					if( dataInfo.phase == phase_battle || dataInfo.phase == phase_read ){
						dataInfo.A.phase = 5;
						dataInfo.B.phase = 5;
					}

					//データを要求
					if( !roopCounter ){
//						SendCmd( dest_branch, cmd_dataInfo );
						SendCmd( dest_root, cmd_dataInfo, (void *)cowcaster_id, 5);
					}
					roopCounter++;
					if( roopCounter > 6 ) roopCounter = 0;

					if( menuFlg == 3 && !battleFlgA && !battleFlgB ){
						InputA = ManipMenuSub( &myInfo.A, &dataInfo.A );
						InputB = ManipMenuSub( &myInfo.B, &dataInfo.B );

						if( myInfo.B.phase == 4 && InputB != key_B ){
							InputB = 0;
						}
						if (myInfo.A.phase > 0) {
							myInfo.A.color = dataInfo.A.color;
						}
						if (myInfo.B.phase > 0) {
							myInfo.B.color = dataInfo.B.color;
						}
						//検証する
						if( ( dataInfo.phase == phase_battle || dataInfo.phase == phase_read ) && ( myInfo.A.phase == 4 && myInfo.B.phase == 4 ) ){
							if( myInfo.A.ID == dataInfo.A.ID && myInfo.A.firstSpell == dataInfo.A.firstSpell && myInfo.A.secondSpell == dataInfo.A.secondSpell
							 && myInfo.B.ID == dataInfo.B.ID && myInfo.B.firstSpell == dataInfo.B.firstSpell && myInfo.B.secondSpell == dataInfo.B.secondSpell
							 && myInfo.place == dataInfo.place && myInfo.placeTime == dataInfo.placeTime )
							{
								BYTE dataTemp;
								if( (!dataInfo.A.color & 2) && (!dataInfo.B.color & 2) ) {
									if( dataInfo.A.color & 1 ){
										dataTemp = 1;
										WriteProcessMemory( hProcess, (void*)0x6714AC, &dataTemp, 1 , NULL );
									}else{
										dataTemp = 0;
										WriteProcessMemory( hProcess, (void*)0x6714AC, &dataTemp, 1 , NULL );
									}
									if( dataInfo.B.color & 1 ){
										dataTemp = 1;
										WriteProcessMemory( hProcess, (void*)0x6714AD, &dataTemp, 1 , NULL );
									}else{
										dataTemp = 0;
										WriteProcessMemory( hProcess, (void*)0x6714AD, &dataTemp, 1 , NULL );
									}
								}
								InputA = key_A;
							}
						}
					}else{
						datA.SetInput( 0 );
						datB.SetInput( 0 );

						myInfo.A.color = dataInfo.A.color;
						myInfo.B.color = dataInfo.B.color;

						//検証する
						if( myInfo.A.ID == dataInfo.A.ID && myInfo.A.firstSpell == dataInfo.A.firstSpell && myInfo.A.secondSpell == dataInfo.A.secondSpell
						 && myInfo.B.ID == dataInfo.B.ID && myInfo.B.firstSpell == dataInfo.B.firstSpell && myInfo.B.secondSpell == dataInfo.B.secondSpell
						 && myInfo.place == dataInfo.place && myInfo.placeTime == dataInfo.placeTime )
						{
							//次のステップへ
							break;
						}else{
							return 1;
						}
					}

					//連打修正
					if( pushFlg ){
						InputA = 0;
						InputB = 0;
					}
				}else if( myInfo.terminalMode == mode_broadcast ){
					//ブロードキャストの場合
					if( menuFlg == 3 && !battleFlgA && !battleFlgB ){
						HWND hFocus = GetForegroundWindow();
						InputA = datA.GetInput();
						if( datA.inputDeviceType == 0xFF ){
							if( hFocus && hWnd != hFocus ){
								InputA = 0;
							}
						}
						InputB = datB.GetInput();
						if( datB.inputDeviceType == 0xFF ){
							if( hFocus && hWnd != hFocus ){
								InputB = 0;
							}
						}
					}else{
						//次のステップへ
						break;
					}
				}

				//debug
				if( myInfo.terminalMode == mode_debug){
					if( myInfo.A.phase > 2 && myInfo.B.phase < 3 ){
						if( keyCancelCounter > 20 ){
							InputB = datB.GetInput() | InputA;
							if( myInfo.B.phase == 0 && InputA & key_B ){
								//none
							}else{
								InputA = 0;
							}
						}else{
							keyCancelCounter++;
						}
					}else{
						if( myInfo.A.phase > 3 && myInfo.B.phase > 3 ){
							//none
							InputB = (datB.GetInput() | (InputA & key_B)) &key_B;
							//InputB = datB.GetInput() | InputA;
						}else{
							InputB = datB.GetInput();
							keyCancelCounter = 0;
						}
					}
					if( !(myInfo.playerSide==0xB && myInfo.B.phase == 4) ){
						HWND hFocus = GetForegroundWindow();
						if( !pushFlg && (!hFocus || hWnd == hFocus) ){
							if( GetKeyState(VK_RETURN) < 0 ){
								InputA = key_A;
								InputB = key_A;
							}else{
								BYTE* myInput = 0;
								if( myInfo.playerSide == 0xA ){
									myInput = &InputA;
								}else if( myInfo.playerSide == 0xB ){
									myInput = &InputB;
								}
								if( myInput ){
									if( GetKeyState(VK_LEFT) < 0 ){
										*myInput = key_left;
									}else if( GetKeyState(VK_RIGHT) < 0 ){
										*myInput = key_right;
									}else if( GetKeyState(VK_UP) < 0 ){
										*myInput = key_up;
									}else if( GetKeyState(VK_DOWN) < 0 ){
										*myInput = key_down;
									}else if( GetKeyState( 90 ) < 0 ){
										*myInput = key_A;
									}else if( GetKeyState( 88 ) < 0 ){
										*myInput = key_B;
									}
								}
							}
						}
					}
				}

				if( pushFlg ){
					pushFlg--;
				}else{
					pushFlg = 5;
				}

			}

			// custom color palette stuff here!
			if (myInfo.A.phase == 0 && (InputA&(key_A|key_D))) {
				if (InputA&(key_up|key_down)) {
					int color = 0;
					if (InputA&key_D) {
						color |= 1;
					}
					if (InputA&key_down) {
						color |= 2;
					}
					myInfo.A.color = (myInfo.A.color&0x1) | 2 | (color<<2);

					if (myInfo.terminalMode == mode_debug || myInfo.terminalMode == mode_broadcast || myInfo.playerSide == 0xA) {
						loadPalette(myInfo.A.ID, color&3, 0);
					}

					pushFlg = 15;
				} else {
					if (myInfo.B.phase > 0 && (myInfo.B.color & 2) && myInfo.A.ID == myInfo.B.ID) {
						BYTE Acolor = 0;
						BYTE Bcolor = (InputA&key_A)?1:0;
						WriteProcessMemory( hProcess, (void*)0x6714AC, &Acolor, 1 , NULL );
						WriteProcessMemory( hProcess, (void*)0x6714AD, &Bcolor, 1 , NULL );
					}
				}
			}
			if (myInfo.B.phase == 0 && (InputB&(key_A|key_D))) {
				if (InputB&(key_up|key_down)) {
					int color = 0;
					if (InputB&key_D) {
						color |= 1;
					}
					if (InputB&key_down) {
						color |= 2;
					}
					myInfo.B.color = (myInfo.B.color&0x1) | 2 | (color<<2);

					if (myInfo.terminalMode == mode_debug || myInfo.terminalMode == mode_broadcast || myInfo.playerSide == 0xB) {
						loadPalette(myInfo.B.ID, color&3, 1);
					}

					pushFlg = 15;
				} else {
					if (myInfo.A.phase > 0 && (myInfo.A.color & 2) && myInfo.A.ID == myInfo.B.ID ) {
						BYTE Acolor = (InputB&key_A)?1:0;
						BYTE Bcolor = 0;
						WriteProcessMemory( hProcess, (void*)0x6714AC, &Acolor, 1 , NULL );
						WriteProcessMemory( hProcess, (void*)0x6714AD, &Bcolor, 1 , NULL );
					}
				}
			}

			paletteReqCounter++;
			if (paletteReqCounter == 30) {
				paletteReqCounter = 0;

				if (myInfo.terminalMode == mode_root) {
					if (myInfo.playerSide == 0xA && (myInfo.B.color & 2)) {
						BYTE Bcolor = (((myInfo.B.color>>2)&3) | (myInfo.B.ID<<2)) + (11*4);
						if (palettes[Bcolor].state == PALETTE_EMPTY) {
							SendCmd(dest_away, cmd_req_palette, &Bcolor, 1);
						}
					} else if (myInfo.playerSide == 0xB && (myInfo.A.color & 2)) {
						BYTE Acolor = (((myInfo.A.color>>2)&3) | (myInfo.A.ID<<2));
						if (palettes[Acolor].state == PALETTE_EMPTY) {
							SendCmd(dest_away, cmd_req_palette, &Acolor, 1);
						}
					}
				} else if (myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch) {
					if (myInfo.A.color & 2) {
						BYTE Acolor = (((myInfo.A.color>>2)&3) | (myInfo.A.ID<<2));
						if (palettes[Acolor].state == PALETTE_EMPTY) {
							SendCmd(dest_root, cmd_req_palette, &Acolor, 1);
						}
					}
					if (myInfo.B.color & 2) {
						BYTE Bcolor = (((myInfo.B.color>>2)&3) | (myInfo.B.ID<<2)) + (11*4);
						if (palettes[Bcolor].state == PALETTE_EMPTY) {
							SendCmd(dest_root, cmd_req_palette, &Bcolor, 1);
						}
					}
				}
			}

			datA.SetBodyInput( InputA );
			datB.SetBodyInput( InputB );
		}
	}

	return 0;
}
