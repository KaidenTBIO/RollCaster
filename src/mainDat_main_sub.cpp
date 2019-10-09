#include "mainDatClass.h"
using namespace std;

int mainDatClass::Communicate(){
	//アクセス相手がどんな状況か、など
	//myInfo.terminalMode = mode_root など

	accessFlg = status_default;

	char access_packet[5 + 1 + 16 + 5 + 1];
	int access_packet_len = 6;
	memcpy(access_packet, cowcaster_id, 5);
	if (myPlayerName[0] && myShortName[0]) {
		access_packet[5] = 1;

		int name_len = strlen(myPlayerName);
		access_packet[access_packet_len++] = name_len;
		memcpy(access_packet+access_packet_len, myPlayerName, name_len);
		access_packet_len += name_len;

		name_len = strlen(myShortName);
		access_packet[access_packet_len++] = name_len;
		memcpy(access_packet+access_packet_len, myShortName, name_len);
		access_packet_len += name_len;
	} else {
		access_packet[5] = 0;
	}
	access_packet[access_packet_len++] = floatControlFlg;

	for(;;){
		SendCmd( dest_access, cmd_access, (void *)access_packet, access_packet_len );
		Sleep(200);
		if( accessFlg == status_ok ){
			//アクセスOK
			myInfo.terminalMode = mode_root;
			lastTime.Away = nowTime;
			WaitForSingleObject( hMutex, INFINITE );
			Away = Access;
			memset( &Access, 0, sizeof(Access) );
			ReleaseMutex( hMutex );
			break;

		}else  if( accessFlg == status_bad ){
			//アクセスBAD
//			cout << "STATUS : BAD ( Communicate() )" << endl;
			myInfo.terminalMode = mode_branch;
			lastTime.Root = nowTime;
			WaitForSingleObject( hMutex, INFINITE );
			Root = Access;
			memset( &Access, 0, sizeof(Access) );
			ReleaseMutex( hMutex );
			break;

		}else if( accessFlg == status_error ){
			cout << "ERROR : ( version error? / float setting )" << endl;
			myInfo.terminalMode = mode_default;
			WaitForSingleObject( hMutex, INFINITE );
			memset( &Access, 0, sizeof(SOCKADDR_IN) );
			ReleaseMutex( hMutex );
			return 1;

		}
		if( !(Access.sin_addr.s_addr) ){
			cout << "ERROR : TIMEOUT? ( Access )" << endl;
			return 1;
		}
		if( GetEsc() ) return 1;
	}
	return 0;
}


