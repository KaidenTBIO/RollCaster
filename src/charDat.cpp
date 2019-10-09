#include "charDatClass.h"
using namespace std;

charDatClass::charDatClass(){
	Button = &joyStatus.Button1;
	diFlg = 0;

	memset( inputBufBody, 0, sizeof( inputBufBody ) );
	memset( inputBufChar, 0, sizeof( inputBufChar ) );
	memset( &joyStatus, 0, sizeof( joyStatus ) );
	memset( keyStatus, 0, sizeof( keyStatus ) );

}

charDatClass::~charDatClass(){
	//none
}

void charDatClass::end(){
	if(diFlg){
		device->Unacquire();
		device->Release();
		di->Release();
		diFlg = 0;
	}
}

int charDatClass::init_internal() {
	hWnd = FindWindow( NULL , windowName );
	return 0;
}

int charDatClass::init(){
	if( !(*th075Flg) ) return 1;
	if( init_internal() ) return 1;

	BYTE iniBuf[40];

	if(playerSide==0xA){
		ReadProcessMemory( *hProcess , (void*)(0x671418 + 0x28) , iniBuf , 40 , NULL );
	}else{
		ReadProcessMemory( *hProcess , (void*)(0x671418 + 0x50) , iniBuf , 40 , NULL );
	}

	//萃夢想の設定を読む
	inputDeviceType = iniBuf[0];
	if(inputDeviceType == 0xFF){
		//キーボード入力
		keyIniLeft  = iniBuf[0x4];
		keyIniRight = iniBuf[0x8];
		keyIniUp    = iniBuf[0xC];
		keyIniDown  = iniBuf[0x10];
		keyIniA = iniBuf[0x14];
		keyIniB = iniBuf[0x18];
		keyIniC = iniBuf[0x1C];
		keyIniD = iniBuf[0x20];
		keyIniP = iniBuf[0x24];
	}else{
		//ジョイパッド入力
		keyIniA = iniBuf[0x14];
		keyIniB = iniBuf[0x18];
		keyIniC = iniBuf[0x1C];
		keyIniD = iniBuf[0x20];
		keyIniP = iniBuf[0x24];
		if(keyIniA > 32 || keyIniB > 32 || keyIniC > 32 || keyIniD > 32 || keyIniP > 32) return 1;
	}
	if( diInit() ){
		diFlg = 0;
		return 1;
	}

	diFlg = 1;
	return 0;
}

int charDatClass::init_if_kbd(){
	if( !(*th075Flg) ) return 1;
	if( init_internal() ) return 1;

	BYTE tempDeviceType;
	if(playerSide==0xA){
		ReadProcessMemory( *hProcess , (void*)(0x671418 + 0x28) , &tempDeviceType , 1 , NULL );
	} else {
		ReadProcessMemory( *hProcess , (void*)(0x671418 + 0x50) , &tempDeviceType , 1 , NULL );
	}

	if (tempDeviceType == 0xff) {
		return init();
	}

	return 1;
}

int charDatClass::init_simple_kbd(){
	if( !(*th075Flg) ) return 1;
	if( init_internal() ) return 1;

	if (!init_if_kbd()) {
		return 0;
	}

	inputDeviceType = 0xff;
	keyIniUp = DIK_UPARROW;
	keyIniDown = DIK_DOWNARROW;
	keyIniLeft = DIK_LEFTARROW;
	keyIniRight = DIK_RIGHTARROW;
	keyIniA = DIK_Z;
	keyIniB = DIK_X;
	keyIniC = DIK_C;
	keyIniD = DIK_V;
	keyIniP = DIK_1;

	if( diInit() ){
		diFlg = 0;
		return 1;
	}

	diFlg = 1;
	return 0;
}

BYTE charDatClass::GetInput(){

	BYTE Input = 0;

	if( !diFlg ) return 0;

	if(inputDeviceType==0xFF){
		if( hWnd != GetForegroundWindow() ){
			return 0;
		}

		//キーボード入力取得
		if( device->GetDeviceState(256, keyStatus ) ){
			cout << "GetDeviceState Error" << endl;
		}
		//未チェック
		if( keyStatus[keyIniLeft]  & 0x80 )	Input = Input | key_left;
		if( keyStatus[keyIniRight] & 0x80 )	Input = Input | key_right;
		if( keyStatus[keyIniUp]  & 0x80 )	Input = Input | key_up;
		if( keyStatus[keyIniDown] & 0x80 )	Input = Input | key_down;
		if( keyStatus[keyIniA] & 0x80 )	Input = Input | key_A;
		if( keyStatus[keyIniB] & 0x80 )	Input = Input | key_B;
		if( keyStatus[keyIniC] & 0x80 )	Input = Input | key_C;
		if( keyStatus[keyIniD] & 0x80 )	Input = Input | key_D;
		if( keyStatus[keyIniP] & 0x80 )	Input = Input | key_P;
	}else{
		//ジョイパッド入力取得
		if ( isPolledDevice )	device->Poll();
		if( device->GetDeviceState(80, &joyStatus) ){
			cout << "GetDeviceState Error" << endl;
		}
		if( joyStatus.X < 0 ) {
			Input = Input | key_left;
		} else if( 0 < joyStatus.X ) {
			Input = Input | key_right;
		}
		if( joyStatus.Y < 0 ){
			Input = Input | key_up;
		} else if ( 0 < joyStatus.Y ) {
			Input = Input | key_down;
		}
		if( Button[keyIniA] & 0x80 ) Input = Input | key_A;
		if( Button[keyIniB] & 0x80 ) Input = Input | key_B;
		if( Button[keyIniC] & 0x80 ) Input = Input | key_C;
		if( Button[keyIniD] & 0x80 ) Input = Input | key_D;
		if( Button[keyIniP] & 0x80 ) Input = Input | key_P;
	}
	return Input;
}

void DecodeInput(BYTE Input, int* inputBuf){
	if(Input == key_P){
		inputBuf[0] = 0;
		inputBuf[1] = 0;
		inputBuf[2] = 0;
		inputBuf[3] = 0;
		inputBuf[4] = 0;
		inputBuf[5] = 0;
		inputBuf[6] = inputBuf[6] + 1;
	}else{
		inputBuf[6] = 0;
		if(Input & key_right || Input & key_left){
			if(Input & key_left){
				if(inputBuf[0] > 0){
					inputBuf[0] = 0;
				}
				inputBuf[0] = inputBuf[0] - 1;
			}else{
				if(inputBuf[0] < 0){
					inputBuf[0] = 0;
				}
				inputBuf[0] = inputBuf[0] + 1;
			}
		}else{
			inputBuf[0] = 0;
		}

		if(Input & key_up || Input & key_down){
			if(Input & key_up){
				if(inputBuf[1] > 0){
					inputBuf[1] = 0;
				}
				inputBuf[1] = inputBuf[1] - 1;
			}else{
				if(inputBuf[1] < 0){
					inputBuf[1] = 0;
				}
				inputBuf[1] = inputBuf[1] + 1;
			}
		}else{
			inputBuf[1] = 0;
		}

		if(Input & key_A){
			inputBuf[2] = inputBuf[2] + 1;
		}else{
			inputBuf[2] = 0;
		}

		if(Input & key_B){
			inputBuf[3] = inputBuf[3] + 1;
		}else{
			inputBuf[3] = 0;
		}

		if(Input & key_C){
			inputBuf[4] = inputBuf[4] + 1;
		}else{
			inputBuf[4] = 0;
		}

		if(Input & key_D){
			inputBuf[5] = inputBuf[5] + 1;
		}else{
			inputBuf[5] = 0;
		}
	}
}

int charDatClass::SetInput( BYTE Input ){
	BYTE   pauseFlg;
	DWORD  myBase;

	if( !(*th075Flg) ) return 1;
	if( !(*hProcess) ) return 1;

	DecodeInput(Input, inputBufBody);

	if(playerSide==0xA){
		WriteProcessMemory( *hProcess , (void*)0x671664 , inputBufBody , 28 , NULL );
		ReadProcessMemory( *hProcess , (void*)0x671418 , &myBase , 4 , NULL );
	}else{
		WriteProcessMemory( *hProcess , (void*)0x6716AC , inputBufBody , 28 , NULL );
		ReadProcessMemory( *hProcess , (void*)0x67141C , &myBase , 4 , NULL );
	}
	ReadProcessMemory( *hProcess , (void*)0x67161C , &pauseFlg , 1 , NULL );
	if(myBase){
		if( !pauseFlg ){
			DecodeInput(Input, inputBufChar);
			WriteProcessMemory( *hProcess , (void*)(myBase + 0x4C0) , inputBufChar , 24 , NULL );
		}
	}

	return 0;
}

int charDatClass::SetBodyInput( BYTE Input ){
	if( !(*th075Flg) ) return 1;
	if( !(*hProcess) ) return 1;

	DecodeInput(Input, inputBufBody);
	if(playerSide==0xA){
		WriteProcessMemory( *hProcess , (void*)0x671664 , inputBufBody , 28 , NULL );
	}else{
		WriteProcessMemory( *hProcess , (void*)0x6716AC , inputBufBody , 28 , NULL );
	}
	return 0;
}

int charDatClass::SetCharInput( BYTE Input ){
	BYTE   pauseFlg;
	DWORD  myBase;

	if( !(*th075Flg) ) return 1;
	if( !(*hProcess) ) return 1;

	if(playerSide==0xA){
		ReadProcessMemory( *hProcess , (void*)0x671418 , &myBase , 4 , NULL );
	}else{
		ReadProcessMemory( *hProcess , (void*)0x67141C , &myBase , 4 , NULL );
	}
	ReadProcessMemory( *hProcess , (void*)0x67161C , &pauseFlg , 1 , NULL );
	if(myBase){
		if( !pauseFlg ){
			DecodeInput(Input, inputBufChar);
			WriteProcessMemory( *hProcess , (void*)(myBase + 0x4C0) , inputBufChar , 24 , NULL );
		}
	}
	return 0;
}

