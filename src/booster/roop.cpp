#include "conf.h"
#include "boosterDatClass.h"
#include "../mainDatClass.h"

#include <stdlib.h>
#include <time.h>

void boosterDatClass::mainRoop(){
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".mainRoop() " << endl;
	#endif

// /*
	if(playerSide == 0xA){
//		cout << hex << enDat->gameInfoPara[15][1] << endl;
//		cout << hex << gameInfoPara[15][1] << endl;
//		cout << myGameInfo[ _para ][1][1] << endl;
//		cout << hex << (WORD)statusID << endl;
	}
// */
	if(gameMode != 4){
		if(intervalFlg==0 && myGameInfo[ _info ][7][0]==1){
			intervalFlg = gameTime + 0x360;
		}
	}

// -- practice stuff begins here
	// This is a gigantic hack.
	if (practiceMode != 0) {
		practiceLastState2 = practiceLastState;
		practiceLastState = practiceState;

		WORD practiceNewState;

		switch (statusID) {
		case 1: // Standing
		case 2: // Crouching
		case 3: // Walk towards
		case 4: // Walk away
			practiceNewState = 0;
			break;
		case 5: // 8
		case 6: // 9
		case 7: // 7
		case 8: // falling back down?
			practiceNewState = 1;
			break;
		case 24: // HJ8
		case 25: // HJ9
		case 26: // HJ7
			practiceNewState = 2;
			break;
		case 32: // Being hit
			practiceNewState = 3;
			break;
		case 33: // On the ground
			practiceNewState = 4;
			break;
		case 34: // Stagger
			practiceNewState = 3;
			break;
		case 38: // Ground block
		case 40: // Air block
			practiceNewState = 5;
			break;
		default:
			// undefined: probably in an action of some sort.
			//cout << "undef: " << (int)statusID << endl;
			practiceNewState = 6;
			break;
		case 146: // fwd ground tech
		case 147: // bkwd ground tech
			practiceNewState = 7;
			break;
		}

		bool skip = 0;

		commandInput[0] = 0;
		commandInput[3] = 0;

		// If we're currently doing an action, skip all other updates until that action is complete.
		if (practiceState == 8) {
			if (practiceNewState == 6 || practiceNewState == 3 || practiceNewState == 4) {
				practiceState = practiceNewState;
			} else {
				// sometimes it bugs out, so we do this
				practiceTimer2--;
				if (practiceTimer2 < 0) {
					practiceState = practiceNewState;
				}
			}
		} else {
			if ((practiceState == 3 || practiceState == 4) && (practiceNewState != 3 && practiceNewState != 4)) {
				commandInput[0] = __5;
			}
			practiceState = practiceNewState;
		}

		practiceTimer = practiceTimer - 1;

		if (practiceTimer < 0) {
			practiceUpdateFlag = 1;
			switch(practiceTiming) {
			case 1:
				practiceTimer = 20;
				break;
			case 2:
				practiceTimer = 30;
				break;
			case 3:
				practiceTimer = 60;
				break;
			case 4:
				practiceTimer = 60*5;
				break;
			case 5:
				practiceTimer = 60*10;
				break;
			case 6:
				practiceTimer = rand()%60;
				break;
			case 7:
				practiceTimer = rand()%120;
				break;
			case 8:
				practiceTimer = 60+rand()%60;
				break;
			case 9:
				practiceTimer = 60+rand()%(60*9);
				break;
			case 10:
				break;
			}
		}

		int n;

		if (practiceState != practiceLastState) {
			//cout << "state change: " << (int)practiceLastState << " - " << (int)practiceState << endl;
			if (practiceState == 7) {
				commandInput[0] = __5;
				commandInput[2] = 0;
				skip = 1;
			} else if ((practiceLastState == 4 || practiceLastState == 7)&& practiceState == 0 && practiceDeclareFlag) {
				//cout << "DOING DECLARE" << endl;
				// Declare.
				commandInput[0] = __22D;
				commandInput[2] = 0;

				practiceState = 8;
				skip = 1;
			} else if (  (!(practiceTeching & 0x40) && practiceMode >= 8 && practiceMode <= 13 && practiceGuard > 1 && practiceLastState == 5 && practiceState < 3)
				    ||((practiceTeching & 0x10) && practiceGuard > 1 && practiceLastState == 3 && practiceState < 3)
				    ||((practiceTeching & 0x20) && practiceGuard > 1 && (practiceLastState == 4 || practiceLastState == 7) && practiceState < 3)) {
				// Post-guarding action.
				int n = practiceGuard;
				if (n == 11) {
					n = (rand()%6)+2;
				} else if (n == 12) {
					n = (rand()%5);
					switch (n) {
					case 0:
						n == 3;
						break;
					case 1:
						n == 5;
						break;
					default:
						n += 5;
						break;
					}
				} else if (n == 13) {
					n = (rand()%12)+2;

					if (n > 9) {
						n += 7;
					}
				} else if (n == 14) {
					n = (rand()%4)+17;
				} else if (n == 15) {
					n = rand()%4;
					if (n < 2) {
						n += 17;
					} else {
						n += 19;
					}
				}

				switch (n) {
				case 1: // Do nothing
					break;
				case 2: // 8
					commandInput[0] = __8;
					break;
				case 3: // HJ8
					commandInput[0] = __D8;
					break;
				case 4: // 7
					commandInput[0] = __7;
					break;
				case 5: // HJ7
					commandInput[0] = __D7;
					break;
				case 6: // 9
					commandInput[0] = __9;
					break;
				case 7: // HJ9
					commandInput[0] = __D9;
					break;
				case 8: // D4
					commandInput[0] = __D4;
					break;
				case 9: // D6
					commandInput[0] = __D6;
					break;
				case 10: // Command move.
					commandInput[0] = practiceSetMoveCommand;
					practiceState = 8;
					break;
				case 17: // 5A
					commandInput[0] = __A;
					practiceState = 8;
					break;
				case 18: // 5B
					commandInput[0] = __B;
					practiceState = 8;
					break;
				case 19: // 66A
					commandInput[0] = __D6A;
					practiceState = 8;
					break;
				case 20: // 66B
					commandInput[0] = __D6B;
					practiceState = 8;
					break;
				case 21: // 2A
					commandInput[0] = __2A;
					practiceState = 8;
					break;
				case 22: // 2B
					commandInput[0] = __2B;
					practiceState = 8;
					break;
				default:
					break;
				}
				commandInput[2] = 0;

				skip = 1;
			} else if (practiceLastState == 3 || practiceLastState == 4) {
				// Return input to a neutral state.
				commandInput[0] = __5;
			}
		}

		if (practiceState == 8 || practiceState == 6) {
			// Don't run anything while waiting for a move to finish.
			skip = 1;
		}

		// Run practice Mode.
		if (!skip) switch (practiceMode) {
		case 1: // Random guard crushes
			if (practiceUpdateFlag && (statusID == 3 || statusID == 4)) {
				if (myID == 8) {
					// yukari tombstone - hits high, weight for high because no one uses low crush!
					n = rand()%3;
					if (n == 2) {
						commandInput[0] = __236A;
					}
				} else if (myID == 5) {
					// youmu ground dash thingie - hits low
					n = rand()%4;
					if (n == 3) {
						commandInput[0] = __214B;
					} else if (n > 0) {
						n--;
					}
				} else {
					n = rand()%2;
				}

				if (n == 0) {
					commandInput[0] = __22A;
				} else if (n == 1) {
					commandInput[0] = __22B;
				}
				commandInput[2] = 0;
				practiceState = 8;
				practiceUpdateFlag = 0;
			} else {
				commandInput[0] = __6;
			}
			break;
		case 2: // Repeat move
			if (practiceUpdateFlag) {
				if (practiceState == 0) {
					commandInput[0] = practiceSetMoveCommand;
					practiceState = 8;
				}
				practiceUpdateFlag = 0;
			}
			break;
		case 3: // Jump, repeat move.
			if (practiceState == 0) {
				switch (practiceJump) {
				case 1: commandInput[0] = __8; break;
				case 2: commandInput[0] = __9; break;
				case 3: commandInput[0] = __7; break;
				case 4: commandInput[0] = __D8; break;
				case 5: commandInput[0] = __D9; break;
				case 6: commandInput[0] = __D7; break;
				}
			} else if (practiceUpdateFlag) {
				if (practiceState == 1 || practiceState == 2) {
					commandInput[0] = practiceSetMoveCommand;
					practiceState = 8;
				}
				practiceUpdateFlag = 0;
			}
			break;
		case 4: // Random attacks
			if (practiceUpdateFlag && practiceState == 0) {
				n = rand()%12;
				switch(n) {
				case 0: commandInput[0] = __A; break;
				case 1: commandInput[0] = __B; break;
				case 2: commandInput[0] = __2A; break;
				case 3: commandInput[0] = __2B; break;
				case 4: commandInput[0] = __6A; break;
				case 5: commandInput[0] = __6B; break;
				case 6: commandInput[0] = __22A; break;
				case 7: commandInput[0] = __22B; break;
				case 8: commandInput[0] = __D6A; break;
				case 9: commandInput[0] = __D6B; break;
				case 10: commandInput[0] = __D63A; break;
				case 11: commandInput[0] = __D63B; break;
				}

				practiceState = 8;
				practiceUpdateFlag = 0;
			}
			break;
		case 5: // Random move
		case 6: {
			static bool justOnce = 0;
			if (practiceUpdateFlag) {
				if (practiceMode == 5) {
					n = rand()%11;
				} else { // 8
					n = rand()%16;
				}
				switch (n) {
				case 0: practiceLastMove = __5; break;
				case 1: practiceLastMove = __4; break;
				case 2: practiceLastMove = __6; break;
				case 3: practiceLastMove = __7; break;
				case 4: practiceLastMove = __8; break;
				case 5: practiceLastMove = __9; break;
				case 6: practiceLastMove = __D4; justOnce = 1; break;
				case 7: practiceLastMove = __D6; justOnce = 1; break;
				case 8: practiceLastMove = __D7; justOnce = 1; break;
				case 9: practiceLastMove = __D8; justOnce = 1; break;
				case 10: practiceLastMove = __D9; justOnce = 1; break;
				default: practiceLastMove = practiceSetMoveCommand; practiceState = 8; break;
				}
				practiceUpdateFlag = 0;
			}
			commandInput[0] = practiceLastMove;
			if (justOnce) {
				practiceLastMove = __5;
				justOnce = 0;
			}
			break;
			}
		case 7: // jump!
			if (practiceUpdateFlag && practiceState == 0) {
				switch (practiceJump) {
				case 1: commandInput[0] = __8; break;
				case 2: commandInput[0] = __9; break;
				case 3: commandInput[0] = __7; break;
				case 4: commandInput[0] = __D8; break;
				case 5: commandInput[0] = __D9; break;
				case 6: commandInput[0] = __D7; break;
				}
			}
			practiceUpdateFlag = 0;
			break;
		case 8: // Random guard.
		case 9:
			if (practiceUpdateFlag) {
				if (!(practiceMode & 1)) {
					n = rand()%3;
				} else {
					n = rand()%2;
				}
				switch(n) {
				case 0: practiceLastMove = __1; break;
				case 1: practiceLastMove = __4; break;
				case 2: practiceLastMove = __5; break;
				default: break;
				}
				practiceUpdateFlag = 0;
			}

			commandInput[0] = practiceLastMove;
			break;
		case 10: // Standing guard
			if (practiceState == 0 || practiceState == 5) {
				commandInput[0] = __4;
			}
			break;
		case 11: // Crouching guard
			if (practiceState == 0 || practiceState == 5) {
				commandInput[0] = __1;
			}
			break;
		case 12: // jump guard!
			if (practiceState == 0) {
				switch (practiceJump) {
				case 1: commandInput[0] = __8; break;
				case 2: commandInput[0] = __9; break;
				case 3: commandInput[0] = __7; break;
				case 4: commandInput[0] = __D8; break;
				case 5: commandInput[0] = __D9; break;
				case 6: commandInput[0] = __D7; break;
				}
			} else {
				commandInput[0] = __4;
			}
			break;
		case 13: // one-hit guard
			if (practiceState == 3 || practiceState == 5) {
				practiceTimer2 = 30;
			}
			practiceTimer2 = practiceTimer2 - 1;
			if (practiceTimer2 > 0) {
				commandInput[0] = __1;
			}

			break;
		case 14: // do nothing
			break;
		default:
			break;
		}

		if (practiceState != 8 && (practiceState == 3 || practiceState == 4)) {
			if (practiceState != practiceLastState) {
				if (statusID == 33) { // ground
					switch (practiceTeching&12) {
					case 4:
						practiceLastTech = __D6;
						break;
					case 8:
						practiceLastTech = __D4;
						break;
					case 12:
						n = rand()%3;
						switch (n) {
						case 0: practiceLastTech = __D6; break;
						case 1: practiceLastTech = __D4; break;
						default: practiceLastTech = __5; break;
						}
						break;
					default:
						practiceLastTech = __5;
						break;
					}
				} else if (statusID == 32) { // air
					switch (practiceTeching&3) {
					case 1:
						practiceLastTech = __D6;
						break;
					case 2:
						practiceLastTech = __D4;
						break;
					case 3:
						n = rand()%3;
						switch (n) {
						case 0: practiceLastTech = __D6; break;
						case 1: practiceLastTech = __D4; break;
						default: practiceLastTech = __5; break;
						}
						break;
					default:
						practiceLastTech = __5;
						break;
					}
				}
				commandInput[2] = 0;
			}
			commandInput[0] = practiceLastTech;
		}

		if (practiceState == 8 && practiceLastState != 8) {
			practiceTimer2 = 10;
		}

		// do not fall through, we want to ignore any and all AI stuff
		return;
	}
// -- practice stuff ends here

	if(AIMode==1 || AIMode==2){
		//リスニング・アクティブ共通ここから

		CallSNAI();
		CallSWAI();
		CallBackAI();
		CallSpellAI();
		CallLNAI();
		CallRecoverAI();
	}
	if(AIMode==2){
		//アクティブここから

		commandInput[0] = 0;
		commandInput[3] = 0;
		if(!(myGameInfo[ _para ][5][0]==0 || *enGameInfo[ _para ][5][0]==0)){
			if(myID==9 && (statusID >= 0xC0 && statusID <= 0xC7)){
				//ミッシング系発動中
				ReadSpellAI();
			}else{
				//ミッシング系発動中でない
				if( !( gameTime < intervalFlg || gameTime < enDat->intervalFlg ) ){
					if(myGameInfo[ _info ][0][1]){
						//後ろ向き
						ReadBackAI();
					}else{
						//後ろ向きでない
						if(myGameInfo[ _info ][1][0] == 2 && myGameInfo[ _para ][1][0] < 3){
							//敵が壁際
							ReadSWAI();
						}else{
							//敵が壁際でない
							ReadSNAI();
						}
					}
					if(myGameInfo[ _para ][9][0]==0){
						//時符発動中でない
						ReadSpellAI();
					}
				}
				ReadLNAI();		//CallLNAIのLine情報を使用
				ReadRecoverAI();
				ManageAI();
			}
		}

		//コピー動作	//テスト用
		//要検討
		#if debug_mode_copy
			if(*enGameInfo[ _status ][2][2]){
				commandInput[0] = *enGameInfo[ _status ][2][2];
				cout << dec << (DWORD)commandInput[0] << endl;
				decodeTime = gameTime + 0x00;
			}else{
				commandInput[0] = 0;
			}
		#endif
	}
}


void boosterDatClass::ConvertDat(){
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".ConvertDat() " << endl;
	#endif
	#if debug_mode_time_show
		if(playerSide == 0xA){
			cout << "time : " << dec << gameTime << endl;
		}
	#endif

	#if debug_mode_ConvertDat
		cout << "debug : " << hex << playerSide << ".ConvertDat().1";
	#endif
	DWORD Temp;



	//float格納
	floatArray[0] = *(float*)&gameInfoPara[0][1];	//自分横軸
	floatArray[1] = *(float*)&gameInfoPara[2][1];	//自分縦軸
	floatArray[2] = *(float*)&(enDat->gameInfoPara[0][1]);	//相手横軸
	floatArray[3] = *(float*)&(enDat->gameInfoPara[2][1]);	//相手縦軸

	//距離
	//相手が右にいるとき正
	floatArray[4] = floatArray[2] - floatArray[0];
	if(floatArray[4] < 0){
		//距離(絶対値)
		floatArray[5] = floatArray[4] * (-1);
		myGameInfo[ _para ][0][0] = 0;
	}else{
		floatArray[5] = floatArray[4];
		myGameInfo[ _para ][0][0] = 1;
	}
//	floatArray[6] = *(float*)&gameInfoPara[4][1];	//自分符力
//	floatArray[7] = *(float*)&(enDat->gameInfoPara[4][1]);	//相手符力
//	floatArray[8] = *(float*)&gameInfoImpress[0][1];	//ハンデ　自分攻撃力倍率
//	floatArray[9] = *(float*)&(enDat->gameInfoImpress[0][1]);	//ハンデ　相手攻撃力倍率

	//short格納
	shortArray[0] = *(short*)&gameInfoInput[0][1];
	shortArray[2] = *(short*)&gameInfoInput[2][1];
	shortArray[4] = *(short*)&gameInfoInput[4][1];
	shortArray[5] = *(short*)&gameInfoInput[5][1];
	shortArray[6] = *(short*)&gameInfoInput[6][1];
	shortArray[7] = *(short*)&gameInfoInput[7][1];


	#if debug_mode_ConvertDat
		cout << ".2";
	#endif


	//ここから各種計算
	//myGameInfoは必要に応じてその都度初期化する


	//上下左右入力バッファ
	myGameInfo[ _input ][4][1] = myGameInfo[ _input ][4][0];
	myGameInfo[ _input ][5][1] = myGameInfo[ _input ][5][0];
	myGameInfo[ _input ][6][1] = myGameInfo[ _input ][6][0];
	myGameInfo[ _input ][7][1] = myGameInfo[ _input ][7][0];

	//下入力
	myGameInfo[ _input ][1][0] = 0;
	if(shortArray[0] > 0){
		if(shortArray[0] > 0x15){
			myGameInfo[ _input ][1][0]=2;
		}else{
			myGameInfo[ _input ][1][0]=1;
		}
		if(shortArray[0] > 0x25){
			myGameInfo[ _input ][1][0]=3;
		}
	}

	//上入力
	myGameInfo[ _input ][0][0] = 0;
	if(shortArray[0] < 0){
		if(shortArray[0] < -0x15){
			myGameInfo[ _input ][0][0]=2;
		}else{
			myGameInfo[ _input ][0][0]=1;
		}
		if(shortArray[0] < -0x25){
			myGameInfo[ _input ][0][0]=3;
		}
	}

	#if debug_mode_ConvertDat
		cout << ".3";
	#endif

	//左右入力（正方向、逆方向）
	myGameInfo[ _input ][2][0]=0;
	myGameInfo[ _input ][3][0]=0;
	if(gameInfoInput[2][1] != 0){
		if(floatArray[4] > 0){
			//相手が右
			if(shortArray[2] > 0){
				if(shortArray[2] >= 0x10){
					myGameInfo[ _input ][2][0]=2;
				}else{
					myGameInfo[ _input ][2][0]=1;
				}
				if(shortArray[2] > 0x20){
					myGameInfo[ _input ][2][0]=3;
				}
			}
			if(shortArray[2] < 0){
				if(shortArray[2] <= -0x10){
					myGameInfo[ _input ][3][0]=2;
				}else{
					myGameInfo[ _input ][3][0]=1;
				}
				if(shortArray[2] < -0x20){
					myGameInfo[ _input ][3][0]=3;
				}
			}
		}else{
			//相手が左
			if(shortArray[2] < 0){
				if(shortArray[2] <= -0x10){
					myGameInfo[ _input ][2][0]=2;
				}else{
					myGameInfo[ _input ][2][0]=1;
				}
				if(shortArray[2] < -0x20){
					myGameInfo[ _input ][2][0]=3;
				}
			}
			if(shortArray[2] > 0){
				if(shortArray[2] >= 0x10){
					myGameInfo[ _input ][3][0]=2;
				}else{
					myGameInfo[ _input ][3][0]=1;
				}
				if(shortArray[2] > 0x20){
					myGameInfo[ _input ][3][0]=3;
				}
			}
		}
	}

	#if debug_mode_ConvertDat
		cout << ".4";
	#endif

	//A入力
	myGameInfo[ _input ][4][0] = 0;
	if(gameInfoInput[4][1] > 0){
		if(gameInfoInput[4][1] > 0x15){
			myGameInfo[ _input ][4][0]=2;
		}else{
			myGameInfo[ _input ][4][0]=1;
		}
		if(gameInfoInput[4][1] > 0x25){
			myGameInfo[ _input ][4][0]=3;
		}
	}

	//B入力
	myGameInfo[ _input ][5][0] = 0;
	if(gameInfoInput[5][1] > 0){
		if(gameInfoInput[5][1] > 0x15){
			myGameInfo[ _input ][5][0]=2;
		}else{
			myGameInfo[ _input ][5][0]=1;
		}
		if(gameInfoInput[5][1] > 0x25){
			myGameInfo[ _input ][5][0]=3;
		}
	}

	//C入力
	myGameInfo[ _input ][6][0] = 0;
	if(gameInfoInput[6][1] > 0){
		if(gameInfoInput[6][1] > 0x15){
			myGameInfo[ _input ][6][0]=2;
		}else{
			myGameInfo[ _input ][6][0]=1;
		}
		if(gameInfoInput[6][1] > 0x25){
			myGameInfo[ _input ][6][0]=3;
		}
	}

	//D入力
	myGameInfo[ _input ][7][0] = 0;
	if(gameInfoInput[7][1] > 0){
		if(gameInfoInput[7][1] > 0x15){
			myGameInfo[ _input ][7][0]=2;
		}else{
			myGameInfo[ _input ][7][0]=1;
		}
		if(gameInfoInput[7][1] > 0x25){
			myGameInfo[ _input ][7][0]=3;
		}
	}

	//入力差分検出
	myGameInfo[ _input ][4][2] = 0;
	myGameInfo[ _input ][5][2] = 0;
	myGameInfo[ _input ][6][2] = 0;
	myGameInfo[ _input ][7][2] = 0;

	if(myGameInfo[ _input ][4][1] != myGameInfo[ _input ][4][0]){
		myGameInfo[ _input ][4][2] = myGameInfo[ _input ][4][0];
	}
	if(myGameInfo[ _input ][5][1] != myGameInfo[ _input ][5][0]){
		myGameInfo[ _input ][5][2] = myGameInfo[ _input ][4][0];
	}
	if(myGameInfo[ _input ][6][1] != myGameInfo[ _input ][6][0]){
		myGameInfo[ _input ][6][2] = myGameInfo[ _input ][4][0];
	}
	if(myGameInfo[ _input ][7][1] != myGameInfo[ _input ][7][0]){
		myGameInfo[ _input ][7][2] = myGameInfo[ _input ][4][0];
	}


	//壁際判定フラグ作成
	myGameInfo[ _info ][1][0] = 0;
	if(floatArray[4] > 0){
		//相手が右
		if(floatArray[0] < 180){
			//自分が壁際
			myGameInfo[ _info ][1][0] = 1;
		}
		if(floatArray[2] > 1100){
			//相手が壁際
			myGameInfo[ _info ][1][0] = 2;
		}
	}
	if(floatArray[4] < 0){
		//相手が左
		if(floatArray[0] > 1100){
			//自分が壁際
			myGameInfo[ _info ][1][0] = 1;
		}
		if(floatArray[2] < 180){
			//相手が壁際
			myGameInfo[ _info ][1][0] = 2;
		}
	}

	#if debug_mode_ConvertDat
		cout << ".5";
	#endif

	//距離S
	myGameInfo[ _para ][1][0] = 7;
	if(floatArray[5] < 600){myGameInfo[ _para ][1][0] = 6;}
	if(floatArray[5] < 420){myGameInfo[ _para ][1][0] = 5;}
	if(floatArray[5] < 300){myGameInfo[ _para ][1][0] = 4;}
	if(floatArray[5] < 220){myGameInfo[ _para ][1][0] = 3;}
	if(floatArray[5] < 150){myGameInfo[ _para ][1][0] = 2;}
	if(floatArray[5] < 110){myGameInfo[ _para ][1][0] = 1;}
	if(floatArray[5] < 70){myGameInfo[ _para ][1][0] = 0;}

	//距離L
	myGameInfo[ _para ][1][1] = 4;
	if(floatArray[5] < 600){myGameInfo[ _para ][1][1] = 3;}
	if(floatArray[5] < 400){myGameInfo[ _para ][1][1] = 2;}
	if(floatArray[5] < 200){myGameInfo[ _para ][1][1] = 1;}
	if(floatArray[5] < 100){myGameInfo[ _para ][1][1] = 0;}


	//ダウンのバウンド時は高さを0とみなす
	if( floatArray[1] < 30 && gameInfoPara[15][1] == 0x61 ){
		floatArray[1] = 0;
	}

	//高さS
	myGameInfo[ _para ][2][0] = 4;
	if(floatArray[1] > 25){	//地上ダッシュ排除のため
		if(floatArray[1] < 190){myGameInfo[ _para ][2][0] = 3;}
		if(floatArray[1] < 130){myGameInfo[ _para ][2][0] = 2;}
		if(floatArray[1] < 70){myGameInfo[ _para ][2][0] = 1;}
	}else{
		myGameInfo[ _para ][2][0] = 0;
	}

	//高さL
	myGameInfo[ _para ][2][1] = 2;
	if(floatArray[1] > 25){	//地上ダッシュ排除のため
		if(floatArray[1] < 190){myGameInfo[ _para ][2][1] = 1;}
	}else{
		myGameInfo[ _para ][2][1] = 0;
	}

	//高さM
	myGameInfo[ _para ][2][2] = 3;
	if(floatArray[1] > 25){	//地上ダッシュ排除のため
		if(floatArray[1] < 170){myGameInfo[ _para ][2][2] = 2;}
		if(floatArray[1] < 100){myGameInfo[ _para ][2][2] = 1;}
	}else{
		myGameInfo[ _para ][2][2] = 0;
	}

	//相対高さ
	//floatArray[1] //自分縦軸
	//floatArray[3] //相手縦軸
	//相対高さ	//地上	//相手が自分よりどのくらい高いか
	if(floatArray[3] > floatArray[1]){
		Temp = (DWORD)((floatArray[3] - floatArray[1]) / 10);
		if(Temp > 0xF){
			myGameInfo[ _para ][3][0] = 0xF;
		}else{
			myGameInfo[ _para ][3][0] = Temp;
		}
	}else{
		myGameInfo[ _para ][3][0] = 0;
	}
	//if(playerSide==0xA){cout << "相対高さテスト(地上) : " << myGameInfo[ _para ][3][0] << endl;}

	//相対高さ	//空中
	if(floatArray[3] > floatArray[1]){
		Temp = (DWORD)((floatArray[3] - floatArray[1]) / 10) + 5;
		if(Temp > 0xF){
			myGameInfo[ _para ][3][1] = 0xF;
		}else{
			myGameInfo[ _para ][3][1] = Temp;
		}
	}else{
		if((DWORD)((floatArray[1] - floatArray[3]) / 10) <= 5){
			Temp = 5 - (DWORD)((floatArray[1] - floatArray[3]) / 10);
			myGameInfo[ _para ][3][1] = Temp;
		}else{
			myGameInfo[ _para ][3][1] = 0;
		}
	}
	//if(playerSide==0xA){cout << "相対高さテスト(空中) : " << myGameInfo[ _para ][3][1] << endl;}

	//相対高さ	//まとめ
	if(myGameInfo[ _para ][2][0]==0){
		//地上
		myGameInfo[ _para ][3][2] = myGameInfo[ _para ][3][0];
	}else{
		//空中
		myGameInfo[ _para ][3][2] = myGameInfo[ _para ][3][1];
	}


	#if debug_mode_ConvertDat
		cout << ".6";
	#endif

	//後ろ向き判定
	if(gameInfoPara[13][1]==0xFF){
		//左向き
		myGameInfo[ _info ][0][0] = 0;
	}else{
		if(gameInfoPara[13][1]==1){
			//右向き
			myGameInfo[ _info ][0][0] = 1;
		}else{
			//開始時に不定のときがある
			myGameInfo[ _info ][0][0] = myGameInfo[ _para ][0][0];
		}
	}

	//後ろ向きの判定	//キャラの向いている方向と座標で判断
	myGameInfo[ _info ][0][1] = 0;
	if(myGameInfo[ _info ][0][0] != myGameInfo[ _para ][0][0]){
		//後ろ向き
		myGameInfo[ _info ][0][1] = 1;
	}


	//霊力のフラグ
	myGameInfo[ _info ][2][2] = 0;
	myGameInfo[ _info ][2][1] = myGameInfo[ _info ][2][0];

	if(gameInfoPara[6][1]){
		//霊力ブレイク
		myGameInfo[ _info ][2][0] = 2;
	}else{
		if(gameInfoPara[3][1] >= 300){
			//霊力通常
			myGameInfo[ _info ][2][0] = 0;
		}else{
			//霊力少ない
			myGameInfo[ _info ][2][0] = 1;
		}
	}
	//ブレイクしたときのフラグ
	if(myGameInfo[ _info ][2][1] != myGameInfo[ _info ][2][0] && myGameInfo[ _info ][2][0]==2){
		myGameInfo[ _info ][2][2] = 2;
	}


	//符	//100以上かどうか
	myGameInfo[ _info ][3][0] = 0;
	if(gameInfoPara[4][1] >= 0x42C80000){myGameInfo[ _info ][3][0] = 1;}

	//霊撃の個数
	myGameInfo[ _info ][4][0] = (DWORD)*(BYTE*)&gameInfoPara[5][1];

	//スペルカード宣言中かどうか
	myGameInfo[ _info ][5][0] = 0;
	if(gameInfoPara[7][1]){myGameInfo[ _info ][5][0] = 1;}


	#if debug_mode_ConvertDat
		cout << ".7";
	#endif


	//ダッシュ回数	//必要ないかも
	myGameInfo[ _info ][6][0] = 0;
	if(gameInfoPara[8][1] == 0x0100){myGameInfo[ _info ][6][0] = 1;}
	if(gameInfoPara[8][1] == 0x0200){myGameInfo[ _info ][6][0] = 2;}
	if(gameInfoPara[8][1] == 0x0300){myGameInfo[ _info ][6][0] = 3;}
	if(gameInfoPara[15][1] == 0xC8){myGameInfo[ _info ][6][0] = 1;}	//地上ダッシュ
	if(gameInfoPara[15][1] == 0xC9){myGameInfo[ _info ][6][0] = 1;}	//地上後ろダッシュ
	if((myGameInfo[ _info ][6][0] == 0) && (gameInfoPara[9][1] !=0)){
		//ハイジャンプ
		myGameInfo[ _info ][6][0] = 1;
	}

	//次もダッシュできるか
	myGameInfo[ _info ][6][1] = 1;
	if(myID == 6){
		if(gameInfoPara[8][1] == 0x0300){
			myGameInfo[ _info ][6][1] = 0;
		}
	}else{
		if(gameInfoPara[8][1] == 0x0200){
			myGameInfo[ _info ][6][1] = 0;
		}
	}

	//壱符か弐符か
	myGameInfo[ _info ][7][0] = (DWORD)*(BYTE*)&gameInfoPara[12][1];

	//符種類
	if(myGameInfo[ _info ][7][0] == 0x02){myGameInfo[ _info ][8][0] = gameInfoPara[10][1];}
	if(myGameInfo[ _info ][7][0] == 0x01){myGameInfo[ _info ][8][0] = gameInfoPara[11][1] + 0x3;}

	//符種類2
	if(myGameInfo[ _info ][7][0] == 0x02){myGameInfo[ _info ][8][1] = gameInfoPara[10][1] * 2;}
	if(myGameInfo[ _info ][7][0] == 0x01){myGameInfo[ _info ][8][1] = gameInfoPara[11][1] * 2 + 1;}

	//符種類3	// statusID
	myGameInfo[ _info ][8][2] = 0xB0 + myGameInfo[ _info ][8][1] * 3;


	#if debug_mode_ConvertDat
		cout << ".8" << endl;
	#endif


	//gameInfoImpress
	//HP	//ChainDMG	//ChainHIT
	if(myGameInfo[ _para ][5][0] >= gameInfoImpress[1][1]){
		//HPが減っていたとき
		myGameInfo[ _para ][5][1] = myGameInfo[ _para ][5][0] - gameInfoImpress[1][1];
	}

	if(myGameInfo[ _para ][6][0] > gameInfoImpress[2][1]){
		//chainDMGが減っていたとき
		myGameInfo[ _para ][6][1] = myGameInfo[ _para ][6][0];
	}else{
		//chainDMGが増えていたとき
		if(myGameInfo[ _para ][7][0] == gameInfoImpress[3][1]){
			if(myGameInfo[ _para ][6][0] != gameInfoImpress[2][1]){
				myGameInfo[ _para ][6][1] = gameInfoImpress[2][1];
			}else{
				myGameInfo[ _para ][6][1] = 0;
			}
		}else{
			//変わっていないときは0になる
			myGameInfo[ _para ][6][1] = gameInfoImpress[2][1] - myGameInfo[ _para ][6][0];
		}
	}

	if(myGameInfo[ _para ][7][0] > gameInfoImpress[3][1]){
		//chainHITが減っていたとき
		myGameInfo[ _para ][7][1] = myGameInfo[ _para ][7][0];
	}else{
		//chainHITが増えていたとき
		//変わっていないときは0になる
		myGameInfo[ _para ][7][1] = gameInfoImpress[3][1] - myGameInfo[ _para ][7][0];
	}

	myGameInfo[ _para ][5][0] = gameInfoImpress[1][1];
	myGameInfo[ _para ][6][0] = gameInfoImpress[2][1];
	myGameInfo[ _para ][7][0] = gameInfoImpress[3][1];


	//時符
	myGameInfo[ _para ][9][0] = 0;
	if(gameInfoPara[16][1] || gameInfoPara[17][1]){
		//時符のアクション中か、もしくは時間操作発動中
		if(gameInfoPara[16][1] > 2){
			//時間停止の溜めアクション中
			myGameInfo[ _para ][9][0] = 3;
		}else{
			if(gameInfoPara[16][1]==2){
				//ストップ
				myGameInfo[ _para ][9][0] = 2;
			}else{
				//スロウ
				myGameInfo[ _para ][9][0] = 1;
			}
		}
	}



	//ここでstatusID更新
	if(gameInfoPara[15][1] >= 800){
		//エラー
		gameInfoPara[15][1] = 799;
	}

	//statusID
	statusID = statusIDArray[ gameInfoPara[15][1] ];

	//BYTE型だから0xFFより大きいことはない
	//if(statusID > 0xFF){statusID = 0xFF;}

	//空中ダッシュ回数でID分ける
	if(gameInfoPara[8][1] == 0x0200 && statusID==0x12){statusID = 0x13;}
	if(gameInfoPara[8][1] == 0x0300 && statusID==0x12){statusID = 0x14;}
	if(gameInfoPara[8][1] == 0x0200 && statusID==0x15){statusID = 0x16;}
	if(gameInfoPara[8][1] == 0x0300 && statusID==0x15){statusID = 0x17;}

	myGameInfo[ _status ][8][4] = myGameInfo[ _status ][8][3];
	myGameInfo[ _status ][8][3] = myGameInfo[ _status ][8][2];
	myGameInfo[ _status ][8][2] = myGameInfo[ _status ][8][1];
	myGameInfo[ _status ][8][1] = myGameInfo[ _status ][8][0];
	myGameInfo[ _status ][8][0] = statusID;

	//ガードの間に立ち状態が挟まるのに対処
	if(statusID == 1 && statusArray[ (BYTE)myGameInfo[ _status ][8][1] ][1] == 0xA){
		statusID = (BYTE)myGameInfo[ _status ][8][1];
	}

	//長押し暫定フィルタ
	if(statusArray[ statusID ][2] == __1 || statusArray[ statusID ][2] == __2 || statusArray[ statusID ][2] == __3){
		if(myGameInfo[ _input ][1][0] < 1){
			statusID = 1;
		}
	}

	if(statusArray[ statusID ][2] == __4 || statusArray[ statusID ][2] == __6){
		if(myGameInfo[ _input ][2][0] < 1 && myGameInfo[ _input ][3][0] < 1){
			statusID = 1;
		}
	}

	myGameInfo[ _status ][0][1] = myGameInfo[ _status ][0][0];
	myGameInfo[ _status ][1][1] = myGameInfo[ _status ][1][0];
	myGameInfo[ _status ][2][1] = myGameInfo[ _status ][2][0];
	myGameInfo[ _status ][9][1] = 0;
	myGameInfo[ _status ][0][2] = 0;
	myGameInfo[ _status ][1][2] = 0;
	myGameInfo[ _status ][2][2] = 0;

	//差分検出
	if((BYTE)myGameInfo[ _status ][9][0] != statusID){
		//ガードの入力が多重になるのを防ぐ
		if(!((myGameInfo[ _status ][0][2] == __1 && statusArray[ statusID ][2] == __1)
		  || (myGameInfo[ _status ][0][2] == __4 && statusArray[ statusID ][2] == __4))
		 && statusArray[ statusID ][2] != __5
		){
			myGameInfo[ _status ][9][1] = statusID;
			myGameInfo[ _status ][0][2] = statusArray[ statusID ][0];
			myGameInfo[ _status ][1][2] = statusArray[ statusID ][1];
			myGameInfo[ _status ][2][2] = statusArray[ statusID ][2];
		}
	}

	//長押し→連打変換判定（相手方の回避のため）
	if(statusArray[ statusID ][0] == 0x4){
		if( ((statusArray[ statusID ][2] == __A || myGameInfo[ _status ][2][0] == __2A || myGameInfo[ _status ][2][0] == __6A) && myGameInfo[ _input ][4][2])
		 || ((statusArray[ statusID ][2] == __B || myGameInfo[ _status ][2][0] == __2B || myGameInfo[ _status ][2][0] == __6B) && myGameInfo[ _input ][5][2])
		 || ((statusArray[ statusID ][2] == __C || myGameInfo[ _status ][2][0] == __2C || myGameInfo[ _status ][2][0] == __6C) && myGameInfo[ _input ][6][2])
		){
			myGameInfo[ _input ][8][0] = statusID;
			myGameInfo[ _input ][8][1] = statusArray[ statusID ][0];
			myGameInfo[ _input ][8][2] = statusArray[ statusID ][1];
			myGameInfo[ _input ][8][3] = statusArray[ statusID ][2];
			myGameInfo[ _input ][8][4] = gameTime;
		}
	}

	/*
	shortArray[0] //上下	//↓が正
	shortArray[2] //左右	//→が正
	shortArray[4] //A
	shortArray[5] //B
	shortArray[6] //C
	shortArray[7] //D
	*/
	//長押し
	myGameInfo[ _input ][9][2] = myGameInfo[ _input ][9][1];
	myGameInfo[ _input ][9][0] = 0;
	myGameInfo[ _input ][9][1] = 0;
	if(statusArray[ statusID ][0]==0){
		//移動
		if(shortArray[2]){
			if(shortArray[2] > 0){
				myGameInfo[ _input ][9][0] = (DWORD)shortArray[2];
			}else{
				myGameInfo[ _input ][9][0] = (DWORD)shortArray[2] * -1;
			}
		}
		if( (DWORD)shortArray[0] > myGameInfo[ _input ][9][0] ){
			//しゃがみ
			myGameInfo[ _input ][9][0] = (DWORD)shortArray[0];
		}
	}
	if(statusArray[ statusID ][1] == 3){
		//攻撃
		if(gameInfoInput[4][1]){
			myGameInfo[ _input ][9][0] = (DWORD)shortArray[4];
		}
		if(gameInfoInput[5][1] > myGameInfo[ _input ][9][0]){
			myGameInfo[ _input ][9][0] = (DWORD)shortArray[5];
		}
		if(gameInfoInput[6][1] > myGameInfo[ _input ][9][0]){
			myGameInfo[ _input ][9][0] = (DWORD)shortArray[6];
		}
	}
	if(statusArray[ statusID ][0] == 8){
		//スペル
		if(gameInfoInput[7][1]){
			myGameInfo[ _input ][9][0] = (DWORD)shortArray[7];
		}
	}
	if(myGameInfo[ _input ][9][0]){
		myGameInfo[ _input ][9][1] = myGameInfo[ _input ][9][0] / 5;
		if(myGameInfo[ _input ][9][1] > 0xF){
			myGameInfo[ _input ][9][1] = 0xF;
		}
	}
	/*
	if(playerSide==0xA){
		cout << myGameInfo[ _input ][9][0] << endl;
		cout << myGameInfo[ _input ][9][1] << endl;
	}
	*/


	//入力があったか、起き上がり直後に更新
	if(myGameInfo[ _status ][2][2] || (myGameInfo[ _status ][0][0]==9 && statusID==1)){
		myGameInfo[ _status ][5][0] = statusID;
		myGameInfo[ _status ][5][1] = gameTime;
		myGameInfo[ _status ][5][2] = myGameInfo[ _status ][2][2];

		myGameInfo[ _status ][6][0] = statusID;
		myGameInfo[ _status ][6][1] = gameTime;
		myGameInfo[ _status ][6][2] = myGameInfo[ _status ][2][2];
	}

	//定期的に0xFF導入
	if( statusID==1 && enDat->statusID != 20 ){
		if(gameTime - myGameInfo[ _status ][6][1] > 150){
			myGameInfo[ _status ][6][0] = 0xFF;
			myGameInfo[ _status ][6][1] = gameTime;
			myGameInfo[ _status ][6][2] = __5;
		}
	}

	//更新
	myGameInfo[ _status ][0][0] = statusArray[ statusID ][0];
	myGameInfo[ _status ][1][0] = statusArray[ statusID ][1];
	myGameInfo[ _status ][2][0] = statusArray[ statusID ][2];

	myGameInfo[ _status ][9][0] = statusID;





	#if debug_mode_show
		if(playerSide==0xA){
			cout << "debug : debug_mode_show" << endl;
			for(Counter=0; Counter< _PAGE; Counter++){
				for(Counter2=0;Counter2<10;Counter2++){
					cout << myGameInfo[Counter][Counter2][0] << " : ";
				}
				cout << endl;
			}
		}
	#endif

	#if debug_mode_show2
		if(playerSide==0xA){
			cout << "debug : ";
			cout	<< hex
				<< gameInfoPara[15][1] << " : "
				<< (WORD)statusID << ","
				<< (WORD)statusArray[ statusID ][0] << ","
				<< (WORD)statusArray[ statusID ][1] << " : ";
			/*
			if(statusID == 0x01){cout << "立ち";}
			if(statusID == 0x02){cout << "しゃがみ";}
			if(statusID == 0x03){cout << "前進";}
			if(statusID == 0x04){cout << "後退";}
			if(statusID == 0x05){cout << "ジャンプ";}
			if(statusID == 0x06){cout << "前ジャンプ";}
			if(statusID == 0x07){cout << "後ジャンプ";}
			if(statusID == 0x10){cout << "前ダッシュ";}
			if(statusID == 0x11){cout << "後ダッシュ";}
			if(statusID == 0x12){cout << "空中前ダッシュ1";}
			if(statusID == 0x13){cout << "空中前ダッシュ2";}
			if(statusID == 0x14){cout << "空中前ダッシュ3";}
			if(statusID == 0x15){cout << "空中後ダッシュ1";}
			if(statusID == 0x16){cout << "空中後ダッシュ2";}
			if(statusID == 0x17){cout << "空中後ダッシュ3";}
			if(statusID == 0x18){cout << "ハイジャンプ";}
			if(statusID == 0x19){cout << "前ジャンプ";}
			if(statusID == 0x1A){cout << "後ジャンプ";}
			if(statusID == 0x20){cout << "食らい";}
			if(statusID == 0x21){cout << "ダウン";}
			if(statusID == 0x26){cout << "ガード";}
			if(statusID == 0x27){cout << "ガード";}
			if(statusID == 0x28){cout << "しゃがみガード";}
			if(statusID == 0x29){cout << "しゃがみガード";}
			if(statusID == 0x22){cout << "ひるみ";}
			if(statusID == 0x24){cout << "霊撃";}
			if(statusID == 0x25){cout << "スペルカード宣言";}
			cout << endl;
			*/
			if(statusArray[ statusID ][0] == 0x0){cout << "ノーマル";}
			if(statusArray[ statusID ][0] == 0x1){cout << "ダッシュ";}
			if(statusArray[ statusID ][0] == 0x2){cout << "食らい";}
			if(statusArray[ statusID ][0] == 0x3){cout << "攻撃";}
			if(statusArray[ statusID ][0] == 0x4){cout << "射撃";}
			if(statusArray[ statusID ][0] == 0x5){cout << "遅延射撃";}
			if(statusArray[ statusID ][0] == 0x6){cout << "移動コマンド";}
			if(statusArray[ statusID ][0] == 0x7){cout << "特殊";}
			if(statusArray[ statusID ][0] == 0x8){cout << "スペル実行中";}
			if(statusArray[ statusID ][0] == 0x9){cout << "ダウン";}
			if(statusArray[ statusID ][0] == 0xA){cout << "ガード";}
			cout << endl;
		}
	#endif
}



void boosterDatClass::InputCommand(){
	hWnd = FindWindow( NULL , casterDat->windowName );
	if( hWnd ){
		GetWindowThreadProcessId( hWnd , &PID );
		if( PID != 0 ){
			hProcess  = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
			ReadProcessMemory( hProcess , (void *)(gameTimeAddress) , &gameTime , 4, NULL );	//gameTime取得
			CloseHandle( hProcess );
		}
		if( !( gameTime > gameTimeTemp2 ) ) return;

		gameTimeTemp2 = gameTime;
		if(commandInput[0] && gameTime >= decodeTime){
			inputTime = 0;
			if(commandInput[2]==0){
				#if debug_mode_command
					cout << "commandInput[0] : " << (DWORD)commandInput[0] << endl;
				#endif



				commandInput[2] = 1 + 3;	//工程番号初期化

				//波動拳コマンドが昇竜拳になるのを防止する	//もっと簡単にできないものか
				if(commandInput[0]==__236A || commandInput[0]==__236B || commandInput[0]==__236C || commandInput[0]==__236D){
					if(gameTime < inputResetTime + 0x20){
						gameInfoIni[1][4] = 0;
						gameInfoIni[2][4] = 0;
						inputResetTime = inputResetTime + 0x20;
					}
				}
				if(commandInput[0]==__214A || commandInput[0]==__214B || commandInput[0]==__214C || commandInput[0]==__214D){
					if(gameTime < inputResetTime + 0x20){
						gameInfoIni[1][4] = 0;
						gameInfoIni[2][4] = 0;
						inputResetTime = inputResetTime + 0x20;
					}
				}

				commandInput[4] = *(BYTE*)&commandArray[ commandInput[0] ];
				commandInput[5] = *((BYTE*)&commandArray[ commandInput[0] ] +0x1);
				commandInput[6] = *((BYTE*)&commandArray[ commandInput[0] ] +0x2);
				commandInput[7] = *((BYTE*)&commandArray[ commandInput[0] ] +0x3);
				commandInput[8] = *((BYTE*)&commandArray[ commandInput[0] ] +0x4);
				commandInput[9] = *((BYTE*)&commandArray[ commandInput[0] ] +0x5);
				commandInput[10] = *((BYTE*)&commandArray[ commandInput[0] ] +0x6);
				commandInput[11] = *((BYTE*)&commandArray[ commandInput[0] ] +0x7);
				commandInput[1] = commandInput[0];	//バッファ
				commandInput[0] = 0;
			}
		}
		if(commandInput[2]){
			if(gameTime > inputTime && gameTime >= inputResetTime ){

				Index = commandInput[2];	//Index導入

				if(commandInput[ Index ] < 0x10){
					if(Index == 4){
						for(Counter=1;Counter<9;Counter++){
							gameInfoIni[Counter][4]=0;
						}
						#if debug_mode_command
							cout << "gameInfoIni[Counter][4] init (first command)" << endl;
						#endif
					}else{
						if(commandInput[ Index -1 ] != 0x1E){
							for(Counter=1;Counter<9;Counter++){
								gameInfoIni[Counter][4]=0;
							}
							#if debug_mode_command
								cout << "gameInfoIni[Counter][4] init" << endl;
							#endif
						}else{
							#if debug_mode_command
								cout << "gameInfoIni[Counter][4] init cancel" << endl;
							#endif
						}
					}
				}else{
					commandInput[ Index ] = commandInput[ Index ] - 0x10;
				}

				#if debug_mode_command
					cout << "input show(start)";
					for(Counter=1;Counter<9;Counter++){
						cout << " : " << gameInfoIni[Counter][5];
					}
					cout << endl;
					cout << "commandInput : " << (DWORD)commandInput[ Index ] << endl;
				#endif


				if(commandInput[ Index ] == 0x1){
					if(commandInput[1]==__1){
						if(myGameInfo[ _para ][0][0]){	//敵が右
							gameInfoIni[1][4] = 1;		//逆方向
						}else{
							gameInfoIni[2][4] = 1;
						}
					}else{
						if(myGameInfo[ _info ][0][0]){	//キャラが右向き
							gameInfoIni[1][4] = 1;		//逆方向
						}else{
							gameInfoIni[2][4] = 1;
						}
					}
					gameInfoIni[4][4] = 1;
				}

				if(commandInput[ Index ] == 0x2){
					gameInfoIni[4][4] = 1;
				}

				if(commandInput[ Index ] == 0x3){
//					if(myGameInfo[ _para ][0][0]){	//敵が右
					if(myGameInfo[ _info ][0][0]){	//キャラが右向き
						gameInfoIni[2][4] = 1;		//正方向
					}else{
						gameInfoIni[1][4] = 1;
					}
					gameInfoIni[4][4] = 1;
				}

				if(commandInput[ Index ] == 0x4){
					if(commandInput[1]==__4){
						if(myGameInfo[ _para ][0][0]){	//敵が右
							gameInfoIni[1][4] = 1;		//逆方向
						}else{
							gameInfoIni[2][4] = 1;
						}
					}else{
						if(myGameInfo[ _info ][0][0]){	//キャラが右向き
							gameInfoIni[1][4] = 1;		//逆方向
						}else{
							gameInfoIni[2][4] = 1;
						}
					}
				}

				if(commandInput[ Index ] == 0x6){
//					if(myGameInfo[ _para ][0][0]){	//敵が右
					if(myGameInfo[ _info ][0][0]){	//キャラが右向き
						gameInfoIni[2][4] = 1;		//正方向
					}else{
						gameInfoIni[1][4] = 1;
					}
				}

				if(commandInput[ Index ] == 0x7){
//					if(myGameInfo[ _para ][0][0]){	//敵が右
					if(myGameInfo[ _info ][0][0]){	//キャラが右向き
						gameInfoIni[1][4] = 1;		//逆方向
					}else{
						gameInfoIni[2][4] = 1;
					}
					gameInfoIni[3][4] = 1;
				}

				if(commandInput[ Index ] == 0x8){
					gameInfoIni[3][4] = 1;
				}

				if(commandInput[ Index ] == 0x9){
//					if(myGameInfo[ _para ][0][0]){	//敵が右
					if(myGameInfo[ _info ][0][0]){	//キャラが右向き
						gameInfoIni[2][4] = 1;		//正方向
					}else{
						gameInfoIni[1][4] = 1;
					}
					gameInfoIni[3][4] = 1;
				}

				if(commandInput[ Index ] == 0xA){
					gameInfoIni[5][4] = 1;
				}
				if(commandInput[ Index ] == 0xB){
					gameInfoIni[6][4] = 1;
				}
				if(commandInput[ Index ] == 0xC){
					gameInfoIni[7][4] = 1;
				}
				if(commandInput[ Index ] == 0xD){
					gameInfoIni[8][4] = 1;
				}

				if(commandInput[ Index ] == 0xE){
					inputTime = gameTime + 0x30;
				}

				if(commandInput[ Index ] == 0x1E){
					inputTime = gameTime + 0x60;
				}else{
					inputTime = gameTime;
				}

				if(commandInput[ Index + 1 ] == 0){
					if(commandInput[3]){
						inputTime = inputTime + 0x10 * commandInput[3];
						commandInput[3] = 0;
					}
				}

				if(gameInfoIni[1][4] || gameInfoIni[2][4] || gameInfoIni[8][4]){
					inputResetTime = gameTime;
				}else{
					if(commandInput[ Index ] != 0x1E && commandInput[ Index ] != 0xE && commandInput[ Index ] != 0x0F && commandInput[ Index ] != 0){
						inputResetTime = 0;
					}
				}


				if(commandInput[ Index ] == 0 || Index == 12){	//とりあえず8まで。コマンドが9以上なら初期化も
//					if(commandInput[0] == (BYTE)myGameInfo[ _status ][2][0]){
//					if(commandInput[0] == (BYTE)myGameInfo[ _status ][2][0] || commandInput[0]==commandInput[1]){
					if(commandInput[0]==commandInput[1]){
						if(commandInput[0] != __1 && commandInput[0] != __4){	//ガードの連続入力を許可する
							commandInput[0] = 0;
							commandInput[1] = 0;
						}
					}
					commandInput[2] = 0;			//コマンド入力終わり
				}else {
					commandInput[2] = commandInput[2] + 1;	//工程番号を進める
				}


				#if debug_mode_command
					cout << "input show(end)  ";
					for(Counter=1;Counter<9;Counter++){
						cout << " : " << gameInfoIni[Counter][4];
					}
					cout << endl;
				#endif
			}
			/*
			gameInfoIni[1][4] left
			gameInfoIni[2][4] right
			gameInfoIni[3][4] up
			gameInfoIni[4][4] down
			gameInfoIni[5][4] A
			gameInfoIni[6][4] B
			gameInfoIni[7][4] C
			gameInfoIni[8][4] D
			gameInfoIni[9][4] pause
			*/
		}else{
			for(Counter=1;Counter<9;Counter++){
				gameInfoIni[Counter][4]=0;
			}
		}

		if( playerSide == 0xB ){
			BYTE InputTemp = 0;
			if( gameInfoIni[1][4] ){
				InputTemp |= key_left;
			}
			if( gameInfoIni[2][4] ){
				InputTemp |= key_right;
			}
			if( gameInfoIni[3][4] ){
				InputTemp |= key_up;
			}
			if( gameInfoIni[4][4] ){
				InputTemp |= key_down;
			}
			if( gameInfoIni[5][4] ){
				InputTemp |= key_A;
			}
			if( gameInfoIni[6][4] ){
				InputTemp |= key_B;
			}
			if( gameInfoIni[7][4] ){
				InputTemp |= key_C;
			}
			if( gameInfoIni[8][4] ){
				InputTemp |= key_D;
			}
			casterDat->boosterInput = InputTemp;
		}


		/*
		for(Counter=1;Counter<9;Counter++){
			if(gameInfoIni[Counter][4]){
				if(gameInfoIni[Counter][5]==0){
					Input.type = INPUT_KEYBOARD;
					Input.ki.wVk = gameInfoIni[Counter][2];
					Input.ki.wScan = MapVirtualKey(gameInfoIni[Counter][2], 0);
					Input.ki.dwFlags = 0;
					Input.ki.dwExtraInfo = 0;
					Input.ki.time = 0;

					SendInput(1, &Input, sizeof(INPUT));
					gameInfoIni[Counter][5] = 1;
				}
			}else{
				if(gameInfoIni[Counter][5]){	//押されている
					Input.type = INPUT_KEYBOARD;
					Input.ki.wVk = gameInfoIni[Counter][2];
					Input.ki.wScan = MapVirtualKey(gameInfoIni[Counter][2], 0);
					Input.ki.dwFlags = KEYEVENTF_KEYUP;
					Input.ki.dwExtraInfo = 0;
					Input.ki.time = 0;

					SendInput(1, &Input, sizeof(INPUT));
					gameInfoIni[Counter][5] = 0;
				}
			}
		}
		*/
	}
}






int boosterDatClass::RefleshDat(){		//データの読み込み
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".RefreshDat() " << endl;
	#endif

	boosterDatInitFlg=1;									//boosterDatInitFlg
	hWnd = FindWindow( NULL , casterDat->windowName );
	if(hWnd != 0){
		GetWindowThreadProcessId( hWnd , &PID );
		if(PID != 0){
			hProcess  = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
			ReadProcessMemory( hProcess , (void*)gameTimeAddress , &gameTime , 4, NULL );	//gameTime取得
			ReadProcessMemory( hProcess , (void*)baseAddress , &battleFlg , 4, NULL );		//戦闘状態か取得
			CloseHandle( hProcess );
		}else{
			return 0xF;
		}
	}else{
		//ボタン入力を開放
		for(Counter=1;Counter<9;Counter++){
			if(gameInfoIni[Counter][5]){	//押されている
				Input.type = INPUT_KEYBOARD;
				Input.ki.wVk = gameInfoIni[Counter][2];
				Input.ki.wScan = MapVirtualKey(gameInfoIni[Counter][2], 0);
				Input.ki.dwFlags = KEYEVENTF_KEYUP;
				Input.ki.dwExtraInfo = 0;
				Input.ki.time = 0;

				SendInput(1, &Input, sizeof(INPUT));
				gameInfoIni[Counter][4]=0;
				gameInfoIni[Counter][5]=0;
			}
		}
		return 0xF;
	}
	if(battleFlg==0){return 0xF;}

	if(gameTimeTemp == gameTime || gameTime == 0){				//時間が進んでるかチェック、gameTime==0チェック
		for(Counter=1;Counter<9;Counter++){
			//時間が止まってるときは入力なし
			if(gameInfoIni[Counter][5]){	//押されている
				Input.type = INPUT_KEYBOARD;
				Input.ki.wVk = gameInfoIni[Counter][2];
				Input.ki.wScan = MapVirtualKey(gameInfoIni[Counter][2], 0);
				Input.ki.dwFlags = KEYEVENTF_KEYUP;
				Input.ki.dwExtraInfo = 0;
				Input.ki.time = 0;

				SendInput(1, &Input, sizeof(INPUT));
				gameInfoIni[Counter][4]=0;
				gameInfoIni[Counter][5]=0;
			}
		}
		return 1;
	}

	if(gameTime < 0x150){return 1;}						//ウェイト

	if (casterDat->boosterWarningsFlg && abs(gameTime - gameTimeTemp) > 2) {
		int n = (gameTime - gameTimeTemp) / 2;
		if (n < 160) {
			cout << "Booster lost " << (n-1) << " frame(s)" << endl;
		}
	}
	gameTimeTemp = gameTime;

	bufSize = myBuf_size;							//mybuf[] は char
	hProcess  = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	ReadProcessMemory( hProcess , (void*)myBase , &myBuf[0] , bufSize, &bufSize );
	CloseHandle( hProcess );
	if(bufSize != myBuf_size){
		return 1;
	}

	for(Counter=0;Counter < 10 ;Counter++){
		if(gameInfoInput[Counter][0]){
			gameInfoInput[Counter][1] = (DWORD)*(WORD*)gameInfoInput[Counter][0];
		}
	}
	for(Counter=0;Counter < 20 ;Counter++){
		if(gameInfoPara[Counter][0]){
			gameInfoPara[Counter][1] = (DWORD)*(WORD*)gameInfoPara[Counter][0];
		}
	}
	for(Counter=0;Counter < 10 ;Counter++){
		if(gameInfoImpress[Counter][0]){
			gameInfoImpress[Counter][1] = (DWORD)*(WORD*)gameInfoImpress[Counter][0];
		}
	}

	gameInfoPara[0][1] = *(DWORD*)gameInfoPara[0][0];	//float類
	gameInfoPara[2][1] = *(DWORD*)gameInfoPara[2][0];
	gameInfoPara[4][1] = *(DWORD*)gameInfoPara[4][0];
	gameInfoImpress[0][1] = *(DWORD*)gameInfoImpress[0][0];
	//データ格納完了

	if(gameInfoPara[0][1] < 0x42200000){return 1;}			//ゲーム内でfloatが初期化済みかチェック
	if(gameInfoPara[0][1] > 0x449B0000){return 1;}			//これが無いとエラーが頻発
	if(gameInfoPara[4][1] > 0x44610000){return 1;}
	if(gameInfoImpress[0][1] < 0x3F000000){return 1;}
	if(gameInfoImpress[0][1] > 0x40000000){return 1;}

	return 0;
}




