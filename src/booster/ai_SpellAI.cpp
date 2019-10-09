#include "conf.h"
#include "boosterDatClass.h"


void boosterDatClass::ReadSpellAI(){
	if(eigenValueSpell[0][0]){
		if(myGameInfo[ _info ][3][0]){
			if(myGameInfo[ _info ][5][0] == 0){
				if(myGameInfo[ _para ][5][0] < 3000){	//HP少ない、宣言可能  ...
					Address = SpellAIbase + eigenValueSpell[0][3];

					if(*Address == 0x25){
						Address2 = Address;
						if(*(Address2 + 1) > 90){
							//スペルカード宣言
							commandInput[0] = __22D;
							decodeTime = 0;
						#if debug_mode_SpellAI
								cout << "宣言" << endl;
							#endif
						}
					}
				}
			}else{
				//スペルカード宣言中
				if(myID==9){	//萃香
					if(statusID >= 0xC0 && statusID <= 0xC7){	//ミッシング系
						SpellAIBuf[0][0][0]=1;
					}else{
						SpellAIBuf[0][0][0]=0;
					}
				}

				if(spell_control == 0 || *enGameInfo[ _status ][1][0] == 2 || *enGameInfo[ _status ][1][0] == 5){
					if(SpellAIBuf[0][0][0]==0){
						if(myGameInfo[ _info ][2][0]==0){	//霊力のチェック
							//スペル発動可能　
							Address = SpellAIbase + eigenValueSpell[0][3];
							for(Counter=2; Counter<40; Counter+=2){
								Address2 = Address + Counter;
								if(*Address2 == (BYTE)myGameInfo[ _info ][8][2]){
									Counter = 40;
									if(*(Address2 + 1) > 80){
										commandInput[0] = statusArray[ *Address2 ][2];
										decodeTime = 0;
										#if debug_mode_SpellAI
											cout << "スペルカード発動" << endl;
										#endif
									}
								}
							}
						}
					}
				}
				if(SpellAIBuf[0][0][0]==1){	//2は発射・展開中
					//スペル開放（オーレリーズサンなど）
					Address = SpellAIbase + eigenValueSpell[0][3];

					if(spell_control == 0 || *enGameInfo[ _status ][1][0] == 2 || *enGameInfo[ _status ][1][0] == 5){
						if(myID==1){
							if(myGameInfo[ _info ][8][2] == 0xBC){	//オーレリーズサン
								for(Counter=2; Counter<40; Counter+=2){
									Address2 = Address + Counter;
									if(*Address2 == 0){
										Counter = 40;
									}
									if(*Address2 == 0xBD || *Address2 == 0xBE){	//発射・展開
										if(*(Address2 + 1) > 80){
											commandInput[0] = statusArray[ *Address2 ][2];
											Counter = 40;
										}
									}
								}
							}
							if(myGameInfo[ _info ][8][2] == 0xBF){	//オーレリーズソーラーシステム
								for(Counter=2; Counter<40; Counter+=2){
									Address2 = Address + Counter;
									if(*Address2 == 0){
										Counter = 40;
									}
									if(*Address2 == 0xC0 || *Address2 == 0xC1){	//発射・展開
										if(*(Address2 + 1) > 80){
											commandInput[0] = statusArray[ *Address2 ][2];
											Counter = 40;
										}
									}
								}
							}
						}
					}
					if(myID==9){	//萃香	//ミッシング系
						if(myGameInfo[ _info ][8][2] == 0xBC || myGameInfo[ _info ][8][2] == 0xBF){
							if(myGameInfo[ _para ][1][0] > 4){
								commandInput[0] = __6;
							}else{
								Address2 = Address + 20;
								if(*(Address2 + 1) > 50){	//要調整
									commandInput[0] = statusArray[ *Address2 ][2];
								}else{
									commandInput[0] = __6;
								}
							}
						}
					}
				}
			}
		}
	}


}

void boosterDatClass::CallSpellAI(){
	if(eigenValueSpell[0][0]){
		//2byte * 20

//		SpellAIBuf[0][0][0]	//フラグに利用


		//固有アドレス更新
		//実験中	//評価が正しくされていない？
		eigenValueSpell[1][2] = myGameInfo[ _para ][1][1];	//距離
		eigenValueSpell[2][2] = myGameInfo[ _para ][2][1];	//自分高さ
		eigenValueSpell[3][2] = *enGameInfo[ _para ][2][1];	//相手高さ
		eigenValueSpell[4][2] = *enGameInfo[ _status ][0][0];	//テキ状態・分類1


		for(Counter=9; Counter>0; Counter--){
			if(eigenValueSpell[Counter][0]){
				if(eigenValueSpell[Counter][2] >= eigenValueSpell[Counter][0]){		//値をチェック
					eigenValueSpell[Counter][2] = eigenValueSpell[Counter][0] -1;
				}											//固有アドレス計算
				eigenValueSpell[Counter - 1][3] = eigenValueSpell[Counter][1] * eigenValueSpell[Counter][2] + eigenValueSpell[Counter][3];
			}
		}



		if(myGameInfo[ _status ][2][2] && myGameInfo[ _status ][2][2] != __5){
			if(myGameInfo[ _status ][0][2]==8 || myGameInfo[ _status ][5][0]==0x25){	//スペル関係
				for(Line=1; Line<10; Line++){
					if(SpellAIBuf[Line][0][0]==0){
						SpellAIBuf[Line][0][0] = 0xFF;	//工程数

						if(myGameInfo[ _status ][5][0]==0x25){	//スペルカード宣言
							SpellAIBuf[Line][4][0] = 1;
							SpellAIBuf[Line][1][1] = 0x30;//相関
							SpellAIBuf[Line][2][1] = 65;	//印象	//何も起きない→失敗
							Counter = 0;	//宣言が先頭
						}else{
							//宣言以外
							SpellAIBuf[Line][4][0] = 0;
							SpellAIBuf[Line][1][1] = 128;//相関
							SpellAIBuf[Line][2][1] = 64;	//印象

							//ここから特殊なスペル
							//相関値の初期値が少ない
							//SpellAIBuf[Line][4][0]がフラグ

							if(myID==1){	//魔理沙スペル
								if(myGameInfo[ _status ][5][0]==0xBC || myGameInfo[ _status ][5][0]==0xBF){	//発動
									SpellAIBuf[Line][4][0] = 2;
									SpellAIBuf[Line][1][1] = 0x30;//相関
									SpellAIBuf[Line][2][1] = 65;	//印象
								}
								if(myGameInfo[ _status ][5][0]==0xBD || myGameInfo[ _status ][5][0]==0xBE
								|| myGameInfo[ _status ][5][0]==0xC0 || myGameInfo[ _status ][5][0]==0xC1){	//発射・展開
									SpellAIBuf[0][0][0] = 2;
									SpellAIBuf[Line][4][0] = 3;
									SpellAIBuf[Line][1][1] = 128;//相関
									SpellAIBuf[Line][2][1] = 64;	//印象
								}
							}
							if(myID==2){	//咲夜スペル
								if(myGameInfo[ _status ][5][0]==0xB0 || myGameInfo[ _status ][5][0]==0xB3
								|| myGameInfo[ _status ][5][0]==0xB6 || myGameInfo[ _status ][5][0]==0xB7
								|| myGameInfo[ _status ][5][0]==0xB9 || myGameInfo[ _status ][5][0]==0xBA){
									SpellAIBuf[Line][4][0] = 1;
									SpellAIBuf[Line][1][1] = 0x70;//相関	//溜めが長い
									SpellAIBuf[Line][2][1] = 65;	//印象
								}
							}
							if(myID==4){	//パチュリースペル
								if(myGameInfo[ _status ][5][0]==0xBF){
									SpellAIBuf[Line][4][0] = 1;
									SpellAIBuf[Line][1][1] = 0x60;//相関
									SpellAIBuf[Line][2][1] = 65;	//印象
								}
							}
							if(myID==5){	//妖夢スペル
								if(myGameInfo[ _status ][5][0]==0xBC || myGameInfo[ _status ][5][0]==0xBF){
									SpellAIBuf[Line][4][0] = 1;
									SpellAIBuf[Line][1][1] = 0x60;//相関
									SpellAIBuf[Line][2][1] = 65;	//印象
								}
							}
							if(myID==8){	//紫スペル
								if(myGameInfo[ _status ][5][0]==0xBF){
									SpellAIBuf[Line][4][0] = 1;
									SpellAIBuf[Line][1][1] = 0x60;//相関
									SpellAIBuf[Line][2][1] = 65;	//印象
								}
							}
							if(myID==9){	//萃香スペル
								if(statusID==0xB0 || statusID==0xB3 || statusID==0xB6 || statusID==0xB9){
									SpellAIBuf[Line][4][0] = 0;
									SpellAIBuf[Line][1][1] = 0x60;//相関
									SpellAIBuf[Line][2][1] = 64;	//印象
								}

								if(myGameInfo[ _status ][5][0]==0xBC || myGameInfo[ _status ][5][0]==0xBF){
									SpellAIBuf[Line][4][0] = 0;	//今は通常スペルとして扱う
									SpellAIBuf[Line][1][1] = 0x60;//相関
									SpellAIBuf[Line][2][1] = 64;	//印象
								}
								if(statusID >= 0xC0 && statusID <= 0xC7){	//ミッシング系
									SpellAIBuf[Line][4][0] = 4;
									SpellAIBuf[Line][1][1] = 0xFF;//相関
									SpellAIBuf[Line][2][1] = 65;	//64;	//印象	//要検討	//良案がない
								}
							}
							Counter = 2;	//宣言が先頭なので
						}

						Address = SpellAIbase + eigenValueSpell[0][3];
						SpellAIBuf[Line][0][1] = 0;
						if(SpellAIBuf[Line][4][0] != 4){
							for(Counter; Counter<20; Counter+=2){
								Address2 = Address + Counter;
								if(*Address2 == 0){
									*(Address2) = statusID;
									*(Address2 + 1) = 95;	//初期値
								}
								if(*Address2 == statusID){	//myGameInfo[ _status ][5][0]と同じ	//どちらで書くのがいいか実験
									Counter = 20;
									SpellAIBuf[Line][0][1] = (DWORD)Address2;
								}
							}
						}else{	//SpellAIBuf[Line][4][0] == 4	//ミッシング系
							SpellAIBuf[Line][0][1] = (DWORD)Address;
						}
						SpellAIBuf[Line][3][1] = 0;
						SpellAIBuf[Line][4][1] = (DWORD)statusID;	//(DWORD)myGameInfo[ _info ][8][0];	//符種類

						Line = 10;
					}
				}
			}
		}

		for(Line=1; Line<10; Line++){
			if(SpellAIBuf[Line][0][0]){
				if(SpellAIBuf[Line][1][1] > __magnification){
					SpellAIBuf[Line][1][1] = SpellAIBuf[Line][1][1] - __magnification;
				}else{
					SpellAIBuf[Line][1][1] = 0;
				}

				//通常
				if(SpellAIBuf[Line][4][0]==0){
					if(myGameInfo[ _para ][5][1]){
						SpellAIBuf[Line][2][1] = SpellAIBuf[Line][2][1] - 10;
						SpellAIBuf[Line][1][1] = 0;
						#if debug_mode_SpellAI
							cout << "ダメージ" << endl;
						#endif
					}
					if(statusArray[ *enGameInfo[ _status ][9][1] ][0]==2 || (*enGameInfo[ _status ][0][0]==2 && *enGameInfo[ _para ][5][1])){
						SpellAIBuf[Line][2][1] = SpellAIBuf[Line][2][1] + 10;
						SpellAIBuf[Line][1][1] = 0;
						#if debug_mode_SpellAI
							cout << "与ダメージ" << endl;
						#endif
					}
					if(myID==9 && (statusID == 0xB7 || statusID == 0xBA)){
						SpellAIBuf[Line][2][1] = SpellAIBuf[Line][2][1] + 10;
						SpellAIBuf[Line][1][1] = 0;
						#if debug_mode_SpellAI
							cout << "鬼縛り系成功" << endl;
						#endif
					}
				}

				//宣言など
				if(SpellAIBuf[Line][4][0]==1 || SpellAIBuf[Line][4][0]==2){	//相関値の初期値が少ない
					if(myGameInfo[ _para ][5][1]){
						SpellAIBuf[Line][2][1] = SpellAIBuf[Line][2][1] - 10;
						SpellAIBuf[Line][1][1] = 0;
						#if debug_mode_SpellAI
							cout << "ダメージ" << endl;
						#endif
					}
				}

				//オーレリーズサンなど
				if(SpellAIBuf[Line][4][0]==3){
					if((myID==1 && SpellAIBuf[Line][1][1] > 60) || myID !=1){
						if(myGameInfo[ _para ][5][1]){
							if(SpellAIBuf[Line][2][1] > 10){
								SpellAIBuf[Line][2][1] = SpellAIBuf[Line][2][1] - 10;
							}else{
								SpellAIBuf[Line][2][1] = 0;
							}
							#if debug_mode_SpellAI
								cout << "ダメージ" << endl;
							#endif
						}
						if(*enGameInfo[ _status ][0][0]==2){
							if(SpellAIBuf[Line][2][1] < 118){
								SpellAIBuf[Line][2][1] = SpellAIBuf[Line][2][1] + 10;
							}else{
								SpellAIBuf[Line][2][1] = 128;
							}
							#if debug_mode_SpellAI
								cout << "与ダメージ" << endl;
							#endif
						}
					}
				}

				//ミッシング系
				if(SpellAIBuf[Line][4][0]==4){
					if(statusID >= 0xC0 && statusID <= 0xC7){
						if(*enGameInfo[ _status ][0][0]==2){
							if(SpellAIBuf[Line][2][1] < 118){
								SpellAIBuf[Line][2][1] = SpellAIBuf[Line][2][1] + 10;
							}else{
								SpellAIBuf[Line][2][1] = 128;
							}
							SpellAIBuf[Line][1][1] = 0;
							#if debug_mode_SpellAI
								cout << "与ダメージ" << endl;
							#endif
						}
					}else{
						SpellAIBuf[0][0][0] = 0;
						SpellAIBuf[Line][1][1] = 0;	//相関を切る
					}
				}

				if(SpellAIBuf[Line][1][1] < 8){
					//終了
					/*
					#if debug_mode_SpellAI
						cout << "終了" << endl;
					#endif
					*/
					if(SpellAIBuf[Line][0][1]){
						if(SpellAIBuf[Line][4][0] != 4){
							Address2 = (BYTE*)SpellAIBuf[Line][0][1];
							SpellAIBuf[Line][0][1] = 0;
							if(*Address2 == (BYTE)SpellAIBuf[Line][4][1]){
								if(SpellAIBuf[Line][2][1] > 64){
									//好印象
									//長く残すデータなので母数を大きく
									if(*(Address2 + 1) < 109){
										*(Address2 + 1) = *(Address2 + 1) + 1;
									}
									if(SpellAIBuf[Line][4][0]==2){
										SpellAIBuf[0][0][0] = 1;	//発動成功
										#if debug_mode_SpellAI
											cout << "オーレリーズ系発動成功" << endl;
										#endif
									}
									if(SpellAIBuf[Line][4][0]==3){
										SpellAIBuf[0][0][0] = 0;	//発射成功
										#if debug_mode_SpellAI
											cout << "オーレリーズ系成功" << endl;
										#endif
									}

									#if debug_mode_SpellAI
										cout << "スペル好印象" << endl;
									#endif
								}else{
									//悪印象
									*(Address2 + 1) = *(Address2 + 1) - (BYTE)(*(Address2 + 1) / 10);
									if(SpellAIBuf[Line][4][0]==3){
										SpellAIBuf[0][0][0] = 0;	//発射失敗
										#if debug_mode_SpellAI
											cout << "オーレリーズ系失敗" << endl;
										#endif
									}
									#if debug_mode_SpellAI
										cout << "スペル悪印象" << endl;
									#endif
								}
							}
						}else{	//SpellAIBuf[Line][4][0] == 4;	//ミッシング系
							Address = (BYTE*)SpellAIBuf[Line][0][1];
							for(Counter=20; Counter<40; Counter+=2){
								Address2 = Address + Counter;
								if(*Address2==0){
									*(Address2) = (BYTE)SpellAIBuf[Line][4][1];
									*(Address2 + 1) = 95;
								}
								if(*Address2==(BYTE)SpellAIBuf[Line][4][1]){
									if(SpellAIBuf[Line][2][1] > 64){
										//好印象
										if(*(Address2 + 1) < 110){
											*(Address2 + 1) = *(Address2 + 1) + 1;
										}
										if(Counter!=20){
											if(*(Address2 + 1) >= *(Address2 - 1)){
												//ランクアップ
												DWORD Temp;
												Temp                   = (DWORD)*(WORD*)(Address2 - 2);
												*(WORD*)(Address2 - 2) = *(WORD*)Address2;
												*(WORD*)Address2       = (WORD)Temp;
											}
										}
										#if debug_mode_SpellAI
											cout << "ミッシング系好印象" << endl;
										#endif
									}else{
										//悪印象
										if(*(Address2 + 1)){
											*(Address2 + 1) = *(Address2 + 1) - 1;
										}
										if(Counter != 38 && *(Address2 + 2)){
											if(*(Address2 + 1) <= *(Address2 + 3)){
												DWORD Temp;
												Temp                   = (DWORD)*(WORD*)Address2;
												*(WORD*)Address2       = *(WORD*)(Address2 + 2);
												*(WORD*)(Address2 + 2) = (WORD)Temp;
											}
										}
										#if debug_mode_SpellAI
											cout << "ミッシング系悪印象" << endl;
										#endif
									}
									Counter = 40;
								}
							}
						}
					}
					SpellAIBuf[Line][0][0] = 0;
				}
			}
		}


	}
}
