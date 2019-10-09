#include "conf.h"
#include "boosterDatClass.h"
#include "HL.h"

void boosterDatClass::ReadLNAI(){
//	if(eigenValueLN[0][0] && *enGameInfo[ _status ][5][0] != 0){
	if(eigenValueLN[0][0] && *enGameInfo[ _status ][5][0] != 0 && *enGameInfo[ _status ][0][0] != 2 && *enGameInfo[ _status ][0][0] != 9){	//実験中
		
		//暫定
		//本来はManageAIが行うべきこと
		BYTE	commandTemp = 0;
		DWORD	decodeTimeTemp = 0;
		
		
		//自分前行動を検索
		Address = LNAIbase + eigenValueLN[0][3];
		for(Counter=1000; Counter<50000; Counter+=1000){
			Address2 = Address + Counter;
			if(*(Address2) == (BYTE)myGameInfo[ _status ][5][0]){
				Counter = 50000;
			}
			if(Counter == 49000 || *(Address2)==0){	//hitしていたら50000になっている筈
				Address2 = 0;
				Counter = 50000;
			}
		}
		if(Address2){
			for(Line=0; Line<20; Line++){
				if(LNAIBuf[Line][0][0] && LNAIBuf[Line][0][0] != 0xFF){
					if(LNAIBuf[Line][1][0] == 4 || LNAIBuf[Line][1][0] == 5 || LNAIBuf[Line][1][0] == 8){
						for(Counter2=20; Counter2<1000; Counter2+=20){
							Address3 = Address2 + Counter2;
							if(*(Address3) == 0){
								Counter2 = 1000;
							}else{
								if(*(Address3) == (BYTE)LNAIBuf[Line][2][0]){
									Counter2 = 1000;
									Address4 = Address3 + 4;
									if(*Address4 && (GetL(Address4 + 2) > 0xA || *(Address3 + 2)==0)){
										if(*(Address4 + 1) == (BYTE)((gameTime - LNAIBuf[Line][3][0]) / 10)){
											commandTemp = *Address4;
											decodeTimeTemp = 0;
										}
									}else{
										if(*(Address3 + 2)
										&& (LNAIBuf[Line][1][0]== 3 || LNAIBuf[Line][1][0]== 4 || LNAIBuf[Line][1][0]== 5
										 || LNAIBuf[Line][1][0]== 7 || LNAIBuf[Line][1][0]== 8)){
											if(*(Address3 + 1) == (BYTE)((gameTime - LNAIBuf[Line][3][0]) / 10)){
												if(GetL(Address3 + 2) > 0x7){
													commandTemp = __4;
												}else{
													commandTemp = __1;
												}
												decodeTimeTemp = 0;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		Address = LNAIbase + eigenValueLN[0][3];
		if(*(Address) == 0xFF){					//最初は必ず0xFF
			Address2 = Address;
			for(Line=0; Line<20; Line++){
				if(LNAIBuf[Line][0][0] == 0xFF){
					for(Counter2=20; Counter2<1000; Counter2+=20){
						Address3 = Address2 + Counter2;
						if(*(Address3) == 0){
							Counter2 = 1000;
						}else{
							if(*(Address3) == (BYTE)LNAIBuf[Line][2][0]){
								Counter2 = 1000;
								Address4 = Address3 + 4;
								if(*Address4 && (GetL(Address4 + 2) > 0xA || *(Address3 + 2)==0)){
									if(*(Address4 + 1) == (BYTE)((gameTime - LNAIBuf[Line][3][0]) / 10)){
										//実験中	//超反応のバックダッシュを無効にする
										if( !(*Address4==__D4 && *(Address4 + 1)<10 && myGameInfo[ _para ][1][1]<2) ){
											commandTemp = *Address4;
											decodeTimeTemp = 0;
										}
									}
								}else{
									if(*(Address3 + 2)
									&& (LNAIBuf[Line][1][0]== 3 || LNAIBuf[Line][1][0]== 4 || LNAIBuf[Line][1][0]== 5
									 || LNAIBuf[Line][1][0]== 7 || LNAIBuf[Line][1][0]== 8)){
										if(*(Address3 + 1) == (BYTE)((gameTime - LNAIBuf[Line][3][0]) / 10)){
											if(GetL(Address3 + 2) > 0x5){
												commandTemp = __4;
											}else{
												commandTemp = __1;
											}
											decodeTimeTemp = 0;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		
		//後ろ向きのときの処理
		if(myGameInfo[ _info ][0][1]){
			if(commandTemp == __D4 || commandTemp == __D6 || commandTemp == __D7 || commandTemp == __D8 || commandTemp == __D9
			|| commandTemp == __7 || commandTemp == __9
			|| commandTemp == __1 || commandTemp == __4){
				if(commandTemp == __D4 || commandTemp == __D6){
					if(commandTemp == __D4){
						commandTemp = __D6;
					}else{
						commandTemp = __D4;
					}
				}
				if(commandTemp == __D7 || commandTemp == __D9){
					if(commandTemp == __D7){
						commandTemp = __D9;
					}else{
						commandTemp = __D7;
					}
				}
				if(commandTemp == __7 || commandTemp == __9){
					if(commandTemp == __7){
						commandTemp = __9;
					}else{
						commandTemp = __7;
					}
				}
			}else{
				commandTemp = 0;
			}
		}
		if(commandTemp){
			commandInput[0] = commandTemp;
			decodeTime = decodeTimeTemp;
			commandInput[3] = 0;
		}
		
	}
}


void boosterDatClass::CallLNAI(){
	if(eigenValueLN[0][0]){
		if( !( gameTime < intervalFlg || gameTime < enDat->intervalFlg ) ){
			if(*enGameInfo[ _status ][2][2] && *enGameInfo[ _status ][2][2] != __5){
				for(Line=0; Line<20; Line++){
					if(LNAIBuf[Line][0][0]==0){
						#if debug_mode_LNAI
							cout << "ライン新設" << endl;
						#endif
						LNAIBuf[Line][0][0] = 0xFF;					//工程数初期化
						LNAIBuf[Line][1][0] = *enGameInfo[ _status ][0][0];	//敵行動タイプ・分類１
						LNAIBuf[Line][2][0] = *enGameInfo[ _status ][5][0];	//敵入力ID
						LNAIBuf[Line][3][0] = *enGameInfo[ _status ][5][1];	//敵入力時間	//[5][1]側のオーバーフローとかは今後
						LNAIBuf[Line][4][0] = 0xFF;					//反応開始	//どの状態から来たのか指定しないという意味
						
						if(LNAIBuf[Line][1][0]==5 || LNAIBuf[Line][1][0]==8){
							LNAIBuf[Line][1][1] = 0xFF;		//相関長い
						}else{
							LNAIBuf[Line][1][1] = 128;		//相関
						}
						LNAIBuf[Line][2][1] = 64;			//印象値初期化
						Line = 20;
					}
				}
			}
		}
		for(Line=0; Line<20; Line++){
			if(LNAIBuf[Line][0][0]){
				//DWORD LNAIBuf[10][5][20]
				//LNAIBuf[Line][0][0]工程数
				//LNAIBuf[Line][1][0]敵入力タイプ・分類１
				//LNAIBuf[Line][2][0]敵入力ID
				//LNAIBuf[Line][3][0]敵入力時間
				//LNAIBuf[Line][4][0]前入力ID
				//type 0:固有アドレス 1:相関値 2:印象値 3:時間 4:入力ID
				
				//ここから各ラインの処理
				//LineとIndexは基本的に変更しない
				
				/*
				//劣化をどうするか
				//処理数増えそうだからとりあえず放置
				*/
				if(myGameInfo[ _status ][2][2] && myGameInfo[ _status ][2][2] != __5){
					if(LNAIBuf[Line][0][0] == 0xFF){
						LNAIBuf[Line][0][0] = 1;
					}else{
						LNAIBuf[Line][0][0] = LNAIBuf[Line][0][0] + 1;	//工程数を進める
					}
					Index = LNAIBuf[Line][0][0];
					
					//固有アドレス計算（データの始めまで検索）
					//書き込む場所が無かった場合は0
					//0xFFは最初の項に書き込む
					LNAIBuf[Line][0][Index] = 0;
					if(myGameInfo[ _status ][2][2] != __22C && myGameInfo[ _status ][2][2] != __22D && myGameInfo[ _status ][2][2] != __236D && myGameInfo[ _status ][2][2] != __214D){
						if(LNAIBuf[Line][0][0]==1){
							Counter = 0;
						}else{
							Counter = 1000;
						}
						Address = LNAIbase + eigenValueLN[0][3];
						for(Counter; Counter<50000; Counter+=1000){
							Address2 = Address + Counter;
							if(*(Address2) == 0){
								*(Address2) = (BYTE)LNAIBuf[Line][4][Index -1];
							}
							if(*(Address2) == (BYTE)LNAIBuf[Line][4][Index -1]){
								Counter = 50000;
								for(Counter2=20; Counter2<1000; Counter2+=20){
									Address3 = Address2 + Counter2;
									if(*(Address3) == 0){
										*(Address3) = (BYTE)LNAIBuf[Line][2][0];
									}
									if(*(Address3) == (BYTE)LNAIBuf[Line][2][0]){
										LNAIBuf[Line][0][Index] = (DWORD)(Address3);	//固有単位アドレス
										Counter2 = 1000;
									}
								}
							}
						}
					}
					
					if(Index != 1){
						LNAIBuf[Line][1][Index] = LNAIBuf[Line][1][Index - 1] * 3 / 4;	//相関値引継ぎ
						LNAIBuf[Line][2][Index] = LNAIBuf[Line][2][Index - 1];		//印象値引継ぎ
					}
					LNAIBuf[Line][3][Index] = myGameInfo[ _status ][5][1];	//時間
					LNAIBuf[Line][4][Index] = myGameInfo[ _status ][5][0];	//入力
				}
				
				if(LNAIBuf[Line][0][0] == 0xFF){
					Index = 1;
				}else{
					Index = LNAIBuf[Line][0][0];
				}
				
				if(LNAIBuf[Line][1][Index] >= __magnification){
					LNAIBuf[Line][1][Index] = LNAIBuf[Line][1][Index] - __magnification;	//相関値は減少する
				}else{
					LNAIBuf[Line][1][Index] = 0;
				}
				
				if(*enGameInfo[ _status ][2][2]){
					LNAIBuf[Line][1][Index] = LNAIBuf[Line][1][Index] * 3 / 4;
				}
				
				//テスト
				if(*enGameInfo[ _para ][5][1]){
					LNAIBuf[Line][1][Index] = 0;	//相関を切る
				}
				
				if(myGameInfo[ _para ][5][1] && (myGameInfo[ _status ][0][0]==2 || myGameInfo[ _status ][0][0]==9 || myGameInfo[ _status ][9][0]==0x22)){
					#if debug_mode_LNAI
						cout << "ダメージ" << endl;
					#endif
					for(Counter=1; Counter<=Index; Counter++){
						if(LNAIBuf[Line][2][Counter] >= 10){
							LNAIBuf[Line][2][Counter] = LNAIBuf[Line][2][Counter] - 10;
						}else{
							LNAIBuf[Line][2][Counter] = 0;
						}
					}
					LNAIBuf[Line][1][Index] = 0;	//相関を切る
				}
				
				if(myGameInfo[ _info ][2][2] && (LNAIBuf[Line][2][0]== 0x38 || LNAIBuf[Line][2][0]== 0x39)){	//ブレイク
					//応急処理
					if(LNAIBuf[Line][0][Index]){
						Address3 = (BYTE*)LNAIBuf[Line][0][Index];
						if(*Address3==0x38){	//__22AのID:0x38が共通なので
						#if debug_mode_LNAI
							cout << "上段ガード付与" << endl;
						#endif
							FloatL(Address3 + 2, 15);
						}
						if(*Address3==0x39){	//__22BのID : 0x39
							#if debug_mode_LNAI
								cout << "下段ガード付与" << endl;
							#endif
							FloatL(Address3 + 2, -15);
						}
					}
					LNAIBuf[Line][1][Index] = 0;		//相関を切る
				}
				
 				if(myGameInfo[ _status ][2][2]){
 					if(statusArray[ LNAIBuf[Line][4][Index] ][0]==0xA){
	 					//ここでガード評価	//印象に関わらず、行動の頻度により変動する。
						if(LNAIBuf[Line][0][Index]){
							Address3 = (BYTE*)LNAIBuf[Line][0][Index];
		 					if(statusArray[ LNAIBuf[Line][4][Index] ][2]==__4){
		 						FloatL(Address3 + 2, 1);
		 					}
		 					if(statusArray[ LNAIBuf[Line][4][Index] ][2]==__1){	//改良の余地アリ
		 						FloatL(Address3 + 2, -2);
		 					}
						}
					}
				}
				
				if(Index >= 19 || LNAIBuf[Line][1][Index] < 5){	//相関・要調整
					//終了
					#if debug_mode_LNAI
						cout << playerSide << ".line" << Line << " Close" << endl;
					#endif
					for(Index=1; Index<=LNAIBuf[Line][0][0]; Index++){
						if(LNAIBuf[Line][2][Index] >= 64){
							if(LNAIBuf[Line][0][Index] && LNAIBuf[Line][0][0] !=0xFF){		//アドレスが確保されていたら
								Address3 = (BYTE*)LNAIBuf[Line][0][Index];
								LNAIBuf[Line][0][Index] = 0;	//アドレスを初期化（念のため）
								if(*Address3 == (BYTE)LNAIBuf[Line][2][0]){	//確認
									
									//ここから評価
									//データの半分が占められてしまうから、後退やしゃがみを保存しない。
									//ヘッダ部分にガード情報
				 					Time = (LNAIBuf[Line][3][Index] - LNAIBuf[Line][3][0]) / 10;
									if(statusArray[ LNAIBuf[Line][4][Index] ][2] == __1 || statusArray[ LNAIBuf[Line][4][Index] ][2] == __4){
										if(statusArray[ LNAIBuf[Line][4][Index] ][0] == 0xA){
											//ガード
											if(*(Address3 + 2)){
												
												//ガードの時間の精度
							 					if((BYTE)Time    < 3+ *(Address3 + 1)
							 					&& (BYTE)Time +3 >    *(Address3 + 1)){
													FloatH(Address3 + 2, 1);
												}else{
													FloatH(Address3 + 2, -1);
												}
												
												//時間調整
												if(GetH( Address3 + 2 ) < 0xA){
								 					if((BYTE)Time > 2+ *(Address3 + 1) ){
														*(Address3 + 1) = *(Address3 + 1) + 1;
													}
								 					if((BYTE)(Time +2) < *(Address3 + 1)){
														*(Address3 + 1) = *(Address3 + 1) - 1;
													}
												}
												
											}else{
												*(Address3 + 1) = (BYTE)Time;
												*(Address3 + 2) = 0x88;
												*(Address3 + 3) = 0;
											}
										}else{
											//後退
											//後退しているのにガードが無かったとき精度を下げる
											//精度が一定値以下になったときはリセット
											if(Index != 19 && AIMode ==2){
												if(statusArray[ LNAIBuf[Line][4][Index + 1] ][0] != 0xA){
													//後→次の行動　となったときの余地アリ
													FloatH(Address3 + 2, -1);
												}
												if(GetH(Address3 + 2) < 0x4){
													*(Address3 + 1) = 0;
													*(Address3 + 2) = 0;
													*(Address3 + 3) = 0;
												}
											}
										}
									}else{
										#if debug_mode_LNAI
											cout << "ランクアップ準備" << endl;
										#endif
									 	for(Counter3=4; Counter3<20; Counter3+=4){
							 				Address4 = Address3 + Counter3;
								 			if(*(Address4) == (BYTE)statusArray[ LNAIBuf[Line][4][Index] ][2]){
								 				
								 				//評価
								 				FloatL(Address4 + 2, 1);
							 					
							 					//精度
							 					if((BYTE)Time    < 3+ *(Address4 + 1)
							 					&& (BYTE)Time +3 >    *(Address4 + 1)){	//時間差が少ない
							 						FloatH(Address4 + 2, 1);
								 				}else{
								 					FloatH(Address4 + 2, -1);
								 				}
								 				
								 				//時間調節
								 				if(GetH(Address4 + 2) < 0xA){
								 					if((BYTE)Time > 2+ *(Address4 + 1)){
														*(Address4 + 1) = *(Address4 + 1) + 1;
													}
								 					if((BYTE)Time +2 < *(Address4 + 1)){
														*(Address4 + 1) = *(Address4 + 1) - 1;
													}
												}
								 				
								 				//入れ替え
								 				if(Counter3 != 4){
								 					if(GetL(Address4 + 2) >= GetL(Address4 - 2)){
														#if debug_mode_LNAI
															cout << "ランクアップ" << endl;
														#endif
														
														DWORD Temp;
								 						Temp				= *(DWORD*)(Address4 - 4);
								 						*(DWORD*)(Address4 - 4)	= *(DWORD*)(Address4);
								 						*(DWORD*)(Address4)	= Temp;
									 				}
								 				}
								 				Counter3 = 20;
								 			}else{
								 				if(*(Address4)==0 || Counter3==16){
								 					//作成・上書き
								 					Counter3 = 20;
								 					*(Address4)     = (BYTE)statusArray[ LNAIBuf[Line][4][Index] ][2];
								 					*(Address4 + 1) = (BYTE)Time;
								 					*(Address4 + 2) = 0x08;	//初期値
								 					*(Address4 + 3) = 0;	//空き容量（劣化情報などを格納？）
								 				}
							 				}
							 			}
							 		}
							 	}
							 }
						}else{
							if(((gameTime - LNAIBuf[Line][3][0]) / 10) < 1){
								Time = 0;	//ガードを付与するべき時間
							}else{
								Time = (gameTime - LNAIBuf[Line][3][0]) / 10 - 1;
							}
							
							//入力の遅れによる食らいを補正・ガードを付与
							if(LNAIBuf[Line][0][0] == 0xFF){
								Address = LNAIbase + eigenValueLN[0][3];
								if(*Address == 0){
									*Address = 0xFF;
								}
								if(*Address == 0xFF){
									Address2 = Address;
									for(Counter2=20; Counter2<1000; Counter2+=20){
										Address3 = Address2 + Counter2;
										if(*Address3 == (BYTE)LNAIBuf[Line][2][0]){
											Counter2 = 1000;
											
											//行動の時間・精度を調節
											Address4 = Address3 + 4;
											if(*Address4){
												#if debug_mode_LNAI
													cout << "時間調節" << endl;
												#endif
							 					FloatH(Address4 + 2, -1);
								 				if(GetH(Address4 + 2) < 0xA){
								 					if((BYTE)Time > *(Address4 + 1)){
														*(Address4 + 1) = *(Address4 + 1) + 1;
													}
								 					if((BYTE)Time < *(Address4 + 1)){
														*(Address4 + 1) = *(Address4 + 1) - 1;
													}
												}
											}
											if(*(Address3 + 2)){
												//ガードの時間・精度を調節
												//微妙
												if(myGameInfo[ _info ][2][0]==2){
													if(GetH(Address3 + 2) > 4){
														FloatL(Address3 + 2, -2);
													}else{
														FloatL(Address3 + 2, 2);
													}
												}
						 						FloatH(Address3 + 2, -1);
								 				if(GetH(Address3 + 2) < 0xA){
								 					if((BYTE)Time > *(Address3 + 1)){
														*(Address3 + 1) = *(Address3 + 1) + 1;
													}
								 					if((BYTE)Time < *(Address3 + 1)){
														*(Address3 + 1) = *(Address3 + 1) - 1;
													}
												}
											}else{
												#if debug_mode_LNAI
													cout << "ガード新規追加" << endl;
												#endif
												*(Address3 + 1) = (BYTE)Time;
												*(Address3 + 2) = 0x88;
												*(Address3 + 3) = 0;
											}
										}else{
											if(*(Address3) == 0){
												#if debug_mode_LNAI
													cout << "ガード新規追加" << endl;
												#endif
												Counter2 = 1000;
												
												*(Address3) = (BYTE)LNAIBuf[Line][2][0];
												*(Address3 + 1) = (BYTE)Time;
												*(Address3 + 2) = 0x88;
												*(Address3 + 3) = 0;
											}
										}
									}
								}
							}
							
							//ここからランクダウンなど
							if(LNAIBuf[Line][0][Index] && LNAIBuf[Line][0][0] !=0xFF){	//アドレスが確保されていたら
								Address3 = (BYTE*)LNAIBuf[Line][0][Index];
								LNAIBuf[Line][0][Index] = 0;	//アドレスを初期化（念のため）
								if(*Address3 == (BYTE)LNAIBuf[Line][2][0]){	//確認
									
									if(statusArray[ LNAIBuf[Line][4][Index] ][2] == __1 || statusArray[ LNAIBuf[Line][4][Index] ][2] == __4){
										if(statusArray[ LNAIBuf[Line][4][Index] ][0] == 0xA){
											//ガード
											//ガードしたのにマイナスということは
											//・削られた
											//・ガードの後に食らった
											//今のところ何もしない。
											//後々は攻撃のタイプによって区分する
										}else{
											//後退
											if(Index != 19 && AIMode == 2){
												if(statusArray[ LNAIBuf[Line][4][Index + 1] ][0] != 0xA){
													//後退しているのにガードが無く、マイナス印象ではガードの精度を大きく下げる。
													//初期値は8
													if(myGameInfo[ _info ][2][0]==2){
														if(GetH(Address3 + 2) > 4){
															FloatL(Address3 + 2, -2);
														}else{
															FloatL(Address3 + 2, 2);
														}
													}else{
														FloatH(Address3 + 2, -2);
													}
												}
												if(GetH(Address3 + 2) < 0x4){
													//精度が一定値以下なら情報を削除。
													*(Address3 + 1) = 0;
													*(Address3 + 2) = 0;
													*(Address3 + 3) = 0;
												}
											}
										}
									}else{
										#if debug_mode_LNAI
											cout << "ランクダウン準備" << endl;
										#endif
										
										if(*(Address3 + 2)==0){
											//ガードを新規に追加
											*(Address3 + 1) = (BYTE)Time;
											*(Address3 + 2) = 0x88;
											*(Address3 + 3) = 0;
										}
										
										//ランクダウン
									 	for(Counter3=4; Counter3<20; Counter3+=4){
									 		Address4 = Address3 + Counter3;
								 			if(*(Address4) == (BYTE)statusArray[ LNAIBuf[Line][4][Index] ][2]){
												#if debug_mode_LNAI
													cout <<"減点" << endl;
												#endif
									 			
									 			//評価・精度
									 			//考察の余地アリ
							 					if((BYTE)Time    < 3+ *(Address4 + 1)
							 					&& (BYTE)Time +3 >    *(Address4 + 1)){
							 						FloatHL(Address4 + 2, -1, -1);
								 				}
							 					
							 					if(Counter3 != 16){
							 						if(*(Address4 + 4)){
								 						if(GetL(Address4 + 2) <= GetL(Address4 + 6)){
								 							//ランクダウン
								 							DWORD Temp;
									 						Temp				= *(DWORD*)(Address4 + 4);
									 						*(DWORD*)(Address4 + 4)	= *(DWORD*)(Address4);
									 						*(DWORD*)(Address4)	= Temp;
									 					}
								 					}
								 				}
							 					Counter3 = 20;
							 				}
								 		}
								 	}
								}
							}
						}
						if(LNAIBuf[Line][0][0]==0xFF){
							LNAIBuf[Line][0][0] = 1;
						}
					}
					//印象・保存処理ここまで
					
					//念のため0クリア
					//処理事態はこれを想定しないで書く
					if(LNAIBuf[Line][0][0]==0xFF){
						LNAIBuf[Line][0][0] = 1;
					}
					for(Counter=0; Counter<=LNAIBuf[Line][0][0]; Counter++){
						LNAIBuf[Line][0][Counter] = 0;
						LNAIBuf[Line][1][Counter] = 0;
						LNAIBuf[Line][2][Counter] = 0;
						LNAIBuf[Line][3][Counter] = 0;
						LNAIBuf[Line][4][Counter] = 0;
					}
					//保険ここまで
					
					
					//必須
					LNAIBuf[Line][0][0] = 0;	//工程番号初期化
					LNAIBuf[Line][1][0] = 0;
				}
			}
		}
		
		//固有アドレス更新
		eigenValueLN[1][2] = myGameInfo[ _para ][1][1];	//距離
		eigenValueLN[2][2] = myGameInfo[ _para ][2][1];	//自分高さ
		eigenValueLN[3][2] = *enGameInfo[ _para ][2][1];	//相手高さ
		if(myGameInfo[ _info ][1][0] == 1){
			if(eigenValueLN[1][2] == 0 || eigenValueLN[1][2] == 1){
				eigenValueLN[1][2] = eigenValueLN[1][2] + 5;
			}
		}
		
		for(Counter=9; Counter>0; Counter--){
			if(eigenValueLN[Counter][0]){
				if(eigenValueLN[Counter][2] >= eigenValueLN[Counter][0]){		//値をチェック
					eigenValueLN[Counter][2] = eigenValueLN[Counter][0] -1;
				}											//固有アドレス計算
				eigenValueLN[Counter - 1][3] = eigenValueLN[Counter][1] * eigenValueLN[Counter][2] + eigenValueLN[Counter][3];
			}
		}
	}
}
