#include "mainDatClass.h"
#include <mmsystem.h>
#include <direct.h>
#include <shlwapi.h>
using namespace std;

#define rand_show 0

void mainDatClass::iniReadStringNonUnicode(const WCHAR *section, const WCHAR *name,
				const WCHAR *default_value, char *dest, int len,
				const WCHAR *path) {
	WCHAR str[1024];
	
	GetPrivateProfileStringW(section, name, default_value, str, len, path);
	str[len] = 0;
	
	WCHAR *c = str;
	
	while (*c) {
		*dest++ = *c++;
	}
	*dest = 0;
}

void mainDatClass::printDate(ostream *stream) {
	SYSTEMTIME time;
	if (systemTimeFlg) {
		GetSystemTime(&time);
	} else {
		GetLocalTime(&time);
	}
	*stream << dec << time.wYear << '/';

	if (time.wMonth < 10) {
		*stream << '0';
	}
	*stream << time.wMonth << '/';
	if (time.wDay < 10) {
		*stream << '0';
	}
	*stream << time.wDay;
}

void mainDatClass::printTime(ostream *stream) {
	SYSTEMTIME time;
	if (systemTimeFlg) {
		GetSystemTime(&time);
	} else {
		GetLocalTime(&time);
	}
	*stream << dec;
	if (time.wHour < 10) {
		*stream << '0';
	}
	*stream << time.wHour << ':';
	if (time.wMinute < 10) {
		*stream << '0';
	}
	*stream << time.wMinute << ':';
	if (time.wSecond < 10) {
		*stream << '0';
	}
	*stream << time.wSecond;
}

const char *mainDatClass::getCharacterName(int ch) {
	switch(ch) {
	case 0 : return "Reimu";
	case 1 : return "Marisa";
	case 2 : return "Sakuya";
	case 3 : return "Alice";
	case 4 : return "Patchouli";
	case 5 : return "Youmu";
	case 6 : return "Remilia";
	case 7 : return "Yuyuko";
	case 8 : return "Yukari";
	case 9 : return "Suika";
	case 10 : return "Meiling";
	default : return "Unknown";
	}
}

const char *mainDatClass::getCharacterShortName(int ch) {
	switch(ch) {
	case 0 : return "Rei";
	case 1 : return "Mar";
	case 2 : return "Sak";
	case 3 : return "Ali";
	case 4 : return "Pat";
	case 5 : return "You";
	case 6 : return "Rem";
	case 7 : return "Yuy";
	case 8 : return "Yuk";
	case 9 : return "Sui";
	case 10 : return "Mei";
	default : return "Unk";
	}
}

void mainDatClass::cleanString(char *string, bool spaces_okay) {
	char *c = string;

	while (*c) {
		if ((*c<'A' || *c>'Z')
		    && (*c<'a' || *c>'z')
		    && (*c<'0' || *c>'9')
		    && !(*c=='.' || *c=='-' || *c == '_' || *c == '\'')
		    && (*c==' ' && !spaces_okay)) {
			*c='_';
		}

		c++;
	}
}

void mainDatClass::runReplayMetadata() {
	if (replayMetadataSaveFlg) {
		fstream replay_file;
		
		replay_file.open(replayFilename, ios::binary|ios::out|ios::app);
		if (replay_file) {
			struct {
				char id[4];
				unsigned int size;
			} chunk_header;
			
			memcpy(chunk_header.id, "META", 4);
			chunk_header.size = 0;
			replay_file.write((char *)&chunk_header, 8);
			
			// save date and time
			SYSTEMTIME systemTime;
			struct replayDateTime_T {
				WORD	wYear;
				WORD	wMonth;
				WORD	wDayOfWeek;
				WORD	wDay;
				WORD	wHour;
				WORD	wMinute;
				WORD	wSecond;
				WORD	wMilliseconds;
			} replayDateTime;
			
			if (systemTimeFlg) {
				GetSystemTime(&systemTime);
			} else {
				GetLocalTime(&systemTime);
			}
			replayDateTime.wYear = systemTime.wYear;
			replayDateTime.wMonth = systemTime.wMonth;
			replayDateTime.wDayOfWeek = systemTime.wDayOfWeek;
			replayDateTime.wDay = systemTime.wDay;
			replayDateTime.wHour = systemTime.wHour;
			replayDateTime.wMinute = systemTime.wMinute;
			replayDateTime.wSecond = systemTime.wSecond;
			replayDateTime.wMilliseconds = systemTime.wMilliseconds;
			
			memcpy(chunk_header.id, "TIME", 4);
			chunk_header.size = sizeof(replayDateTime);
			replay_file.write((char *)&chunk_header, 8);
			replay_file.write((char *)&replayDateTime, sizeof(replayDateTime));
			
			// save player information
			struct {
				WCHAR	p1name[30];
				WCHAR	p2name[30];
			} player_info;
			
			memset(&player_info, 0, sizeof(player_info));
			
			wsprintfW(player_info.p1name, L"%S", p1PlayerName);
			wsprintfW(player_info.p2name, L"%S", p2PlayerName);
			
			memcpy(chunk_header.id, "PLNM", 4);
			chunk_header.size = sizeof(player_info);
			replay_file.write((char *)&chunk_header, 8);
			replay_file.write((char *)&player_info, sizeof(player_info));
			
			// save palettes
			for (int i = 0; i < 2; ++i) {
				unsigned int pal[256];
				if (getPaletteData(i, pal)) {
					memcpy(chunk_header.id, i==0?"PAL1":"PAL2", 4);
					chunk_header.size = 1024;
					replay_file.write((char *)&chunk_header, 8);
					replay_file.write((char *)pal, 1024);
				}
			}
			
			// version comment
			WCHAR comment[60];
			wsprintfW(comment, L"RollCaster %S", cowcaster_version_string);
			
			memcpy(chunk_header.id, "COMM", 4);
			chunk_header.size = (lstrlenW(comment)+1)*2;
			replay_file.write((char *)&chunk_header, 8);
			replay_file.write((char *)comment, chunk_header.size);
			
			// finish up
			replay_file.close();
		}
	}
}

void mainDatClass::runAutoSaveRename(){
	if (autoSaveReplayNameFlg) {
		char encodedName[8];
		memset(encodedName, ' ', 8);
		memcpy(encodedName, p1ShortName, strlen(p1ShortName));
		memcpy(encodedName+4, p2ShortName, strlen(p2ShortName));
		for (int i = 0; i < 8; ++i) {
			char c = encodedName[i];

			if (c >= 'A' && c <= 'Z') {
				encodedName[i] = (c-'A');
			} else if (c >= 'a' && c <= 'z') {
				encodedName[i] = (c-'a')+26;
			} else if (c >= '0' && c <= '9') {
				encodedName[i] = (c-'0')+52;
			} else if (c == '-') {
				encodedName[i] = 63;
			} else if (c == '_') {
				encodedName[i] = 75;
			} else if (c == '.') {
				encodedName[i] = 71;
			} else {
				encodedName[i] = 0x64;
			}
		}

		fstream replay_file;

		replay_file.open(replayFilename, ios::binary|ios::out|ios::in);
		if (replay_file) {
			replay_file.write(encodedName, 8);
			replay_file.close();
		}
	}
	
	if (autoSaveReplayFileRenameFlg) { // && p1PlayerName[0] && p2PlayerName[0]) {
		WCHAR new_replay_filename[1200];
		int n = 1;
		bool has_n = 0;
		int fn_end;
		
		new_replay_filename[0] = '\0';

		if (iniPath) {
			GetPrivateProfileStringW( isRollIni?L"ADVANCED":L"MAIN", L"th075File", L"", new_replay_filename, 200, iniPath);
			
			PathRemoveFileSpecW(new_replay_filename);
			
			if (new_replay_filename[0]) {
				lstrcatW(new_replay_filename, L"\\");
			}
		}
		
		lstrcatW(new_replay_filename, L"replay\\");
		
		fn_end = lstrlenW(new_replay_filename);

		while (1) {
			new_replay_filename[fn_end] = '\0';
			WCHAR *c = new_replay_filename + fn_end;
			const WCHAR *src = replayFilenameFormat;
			while (*src) {
				if (*src == '%') {
					src++;
					if (*src == 'A' || *src == 'B') {
						int side;
						const char *str;
						bool isTemp;
						if (*src == 'A') {
							side = 0xA;
							str = p1PlayerName;
							isTemp = p1TempName;
						} else {
							side = 0xB;
							str = p2PlayerName;
							isTemp = p2TempName;
						}

						// hack to get around speccing issues.
						if (myInfo.terminalMode == mode_root && (!*str || isTemp)) {
							if (!unknownNameFlg) {
								if (myInfo.playerSide == side) {
									str = "Me";
								} else {
									str = inet_ntoa(Away.sin_addr);
								}
							} else {
								str = "Unknown";
							}
						}
						wsprintfW(c, L"%S", str);
						//cleanString(c, 1);
						c += lstrlenW(c);
					} else if (*src == 'a') {
						wsprintfW(c, L"%S", p1ShortName);
						//cleanString(c, 1);
						c += lstrlenW(c);
					} else if (*src == 'b') {
						wsprintfW(c, L"%S", p2ShortName);
						//cleanString(c, 1);
						c += lstrlenW(c);
					} else if (*src == '%') {
						*c++ = '%';
					} else if (*src == 'n') {
						wsprintfW(c, L"%3.3d", n);
						c += lstrlenW(c);
						has_n = 1;
					} else if (*src == 'd') {
						SYSTEMTIME time;
						if (systemTimeFlg) {
							GetSystemTime(&time);
						} else {
							GetLocalTime(&time);
						}
						wsprintfW(c, L"%4.4d%2.2d%2.2d", time.wYear, time.wMonth, time.wDay);
						c += lstrlenW(c);
					} else if (*src == 'D') {
						SYSTEMTIME time;
						if (systemTimeFlg) {
							GetSystemTime(&time);
						} else {
							GetLocalTime(&time);
						}
						wsprintfW(c, L"%2.2d%2.2d%2.2d", time.wYear%100, time.wMonth, time.wDay);
						c += lstrlenW(c);
					} else if (*src == 't') {
						SYSTEMTIME time;
						if (systemTimeFlg) {
							GetSystemTime(&time);
						} else {
							GetLocalTime(&time);
						}
						wsprintfW(c, L"%2.2d%2.2d%2.2d", time.wHour, time.wMinute, time.wSecond);
						c += lstrlenW(c);
					} else if (*src == '1' || *src == '2') {
						WORD character = lastCharacterA;
						if (*src == '2') {
							character = lastCharacterB;
						}

						const char *str = getCharacterName(character);
						
						wsprintfW(c, L"%S", str);
						c += lstrlenW(c);
					} else if (*src == '3') {
						wsprintfW(c, L"%d", lastCharacterA+1);
						c += lstrlenW(c);
					} else if (*src == '4') {
						wsprintfW(c, L"%d", lastCharacterB+1);
						c += lstrlenW(c);
					} else if (*src == '5' || *src == '6') {
						WORD character = lastCharacterA;
						if (*src == '6') {
							character = lastCharacterB;
						}

						const char *str = getCharacterShortName(character);

						wsprintfW(c, L"%S", str);
						c += lstrlenW(c);
					} else if (*src == 'S') {
						src++;

						int value;

						switch (*src) {
						case '1': value = myInfo.A.firstSpell+1; break;
						case '2': value = myInfo.A.secondSpell+1; break;
						case '3': value = myInfo.B.firstSpell+1; break;
						case '4': value = myInfo.B.secondSpell+1; break;
						default: value = -1;
						}
						if (value > 0) {
							wsprintfW(c, L"%d", value);
							c += lstrlenW(c);
						}
					} else if (*src == 'm') {
						wsprintfW(c, L"%d", lastGameTime/120);
						c += lstrlenW(c);
					} else if (*src == 'M') {
						wsprintfW(c, L"%1.2f", (double)lastGameTime/120.0);
						c += lstrlenW(c);
					} else if (*src == 'f') {
						wsprintfW(c, L"%d", lastGameTime);
						c += lstrlenW(c);
					} else if (*src == 'F') {
						if (newCWflg) {
							lstrcpyW(c,L"(FPU)");
							c += lstrlenW(c);
						}
					} else if (*src == 'L') {
						if (myInfo.terminalMode == mode_broadcast) {
							wsprintfW(c, L"%d,%d", delayTimeA/2,delayTimeB/2);
							c += lstrlenW(c);
						} else if (myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug) {
							wsprintfW(c, L"%d", delayTime/2);
							c += lstrlenW(c);
						} else {
							*c++ = 'S';
						}
					} else if (*src == '&') {
						src++;

						const WCHAR *start = src;
						while (*src && *src != ';') {
							src++;
						}

						if (src != start && newCWflg) {
							int n = src - start;
							
							memcpy(c, start, n*2);
							c += n;
						}
					}

					if (*src != '\0') {
						src++;
					}
				} else {
					*c++ = *src++;
				}
			}
			lstrcpyW(c, L".rep");

			if (GetFileAttributesW(new_replay_filename) == 0xFFFFFFFF) {
				break;
			}
			n++;

			if (!has_n) {
				cout << "Error: Replay filename already exists - Is your format lacking a %n or %t?" << endl;
				n = 0;
				break;
			}
		}

		if (n != 0) {
			WCHAR *c = new_replay_filename;
			while (*c != '\0') {
				if (*c == '\\') {
					*c = '\0';
					_wmkdir(new_replay_filename);
					*c++ = '\\';
				}
				c++;
			}
			
			WCHAR replayFilenameW[280];
			memcpy(replayFilenameW, new_replay_filename, fn_end*2);
			
			wsprintfW(replayFilenameW+fn_end-7, L"%S", replayFilename);
			
			MoveFileW(replayFilenameW, new_replay_filename);
		}
	}
}

void mainDatClass::loadPalette(int ch, int n, int player) {
	char palfilename[80];
	char *s;

	sprintf(palfilename, "palette\\%s-%d.pal", getCharacterName(ch), n+1);

	n += (ch*4) + (player*11*4);

	if (player >= 2) {
		n = (11*4*2) + player - 2;
		palettes[n].state = PALETTE_EMPTY;
	}

	if (palettes[n].state != PALETTE_EMPTY) {
		return;
	}

	ifstream file(palfilename);
	if (file) {
		char buf[1024];
		file.read(buf, 1024);
		file.close();

		WORD *dpal = (WORD *)palettes[n].data;
		DWORD *spal = (DWORD *)buf;

		for (int i = 0; i < 256; ++i) {
			DWORD p = *spal++;
			// 0x8000 is mask bit, it is fixed at load time
			*dpal++ = ((p&0xf8)>>3) | (((p>>8)&0xf8)<<2) | (((p>>16)&0xf8)<<7) | 0x8000;
		}

		palettes[n].state = PALETTE_LOADED;
	} else {
		palettes[n].state = PALETTE_INVALID;
	}
}

void mainDatClass::requestPaletteInfo(int color) {
	// minimum protocol version for palette data is 4
	if (enCowCaster < 4 ||
		(myInfo.terminalMode != mode_root && myInfo.terminalMode != mode_branch && myInfo.terminalMode != mode_subbranch)) {
		palettes[color].state = PALETTE_INVALID;
		return;
	}

	int paletteReqCounter = 0;
	int max_time = 200;

	BYTE bcolor = color;

	for(;;){
		if (palettes[color].state != PALETTE_EMPTY) {
			break;
		}

		if (paletteReqCounter == 0) {
			paletteReqCounter = 30;

			if (myInfo.terminalMode == mode_root) {
				SendCmd(dest_away, cmd_req_palette, &bcolor, 1);
			} else if (myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch) {
				SendCmd(dest_root, cmd_req_palette, &bcolor, 1);
			}
		}

		if (--max_time <= 0) {
			cout << "Couldn't get palette information within 10s time, skipping..." << endl;
			break;
		}

		if( GetEsc() ) break;
		if( !roopFlg ) break;
		Sleep(50);
	}
}

bool mainDatClass::getPaletteData(int player_no, unsigned int *dest) {
	if (paletteNo[player_no] == -1) {
		return 0;
	}
	
	paletteEntryStruct *paldata;
	
	paldata = &palettes[paletteNo[player_no]];
	
	unsigned short *src = (unsigned short *)paldata->data;
	
	// have to convert back, whee
	for (int i = 0; i < 256; ++i) {
		unsigned short c = *src++;
		unsigned int v;
		
		v  = (c&0x8000)?0xff000000:0;
		v |= ((c&0x7e00)>>7)<<16;
		v |= ((c&0x03e0)>>2)<<8;
		v |= ((c&0x001f)<<3);
		
		*dest++ = v;
	}
	
	return 1;
}

void mainDatClass::runPaletteUpdate(DWORD addr) {
	paletteEntryStruct *paldata = 0;
	int color = -1;
	bool invalid = 0;

	BYTE Acolor = ((myInfo.A.color >> 2)&3) + (myInfo.A.ID << 2);
	BYTE Bcolor = ((myInfo.B.color >> 2)&3) + (myInfo.B.ID << 2);
	
	if (paletteCounter == 0 && (myInfo.A.color & 2)) {
		paldata = &palettes[Acolor];
		color = Acolor;
	} else if (paletteCounter == 1 && (myInfo.B.color & 2)) {
		paldata = &palettes[Bcolor + (4*11)];
		color = Bcolor + (4*11);
	}

	if (processRemotePalettesFlg == 2) {
		if (myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch) {
			invalid = true;
		}

		if (myInfo.terminalMode == mode_root) {
			if (myInfo.playerSide == 0xB && paletteCounter == 0) {
				invalid = true;
			} else if (myInfo.playerSide == 0xA && paletteCounter == 1) {
				invalid = true;
			}
		}
	}
	
	if (color >= 0 && (paldata->state != PALETTE_INVALID || processRemotePalettesFlg == 1)) {
		if (paldata->state == PALETTE_EMPTY) {
			if (((myInfo.terminalMode == mode_root || myInfo.terminalMode == mode_debug) && myInfo.playerSide == (0xA+paletteCounter))
			    || myInfo.terminalMode == mode_broadcast) {
				int c = color;
				if (paletteCounter == 1) {
					c -= (4*11);
				}
				loadPalette(c>>2, c&0x3, paletteCounter);
			} else {
				requestPaletteInfo(color);
			}
		}

		if (processRemotePalettesFlg == 1 && !invalid) {
			int c = color;
			if (paletteCounter == 1) {
				c -= (4*11);
			}
			loadPalette(c>>2, c&0x3, 2+paletteCounter);
			color = (4*11*2)+paletteCounter;
			paldata = &palettes[color];
		}

		if (paldata->state == PALETTE_LOADED && !invalid) {
			if (paletteCounter == 1) {
				if (paletteNo[0] >= 0) {
					if (!memcmp(paldata->data, palettes[paletteNo[0]].data, 512)) {
						// clash, strip palette.
						return;
					}
				}
			}
			paletteNo[paletteCounter] = color;

			BYTE newpaldata[512];
			const WORD *spal = (WORD *)paldata->data;
			WORD *dpal = (WORD *)newpaldata;

			ReadProcessMemory(pi.hProcess, (void *)addr, (void *)newpaldata, 512, 0);

			for (int i = 0; i < 256; ++i) {
				*dpal = (*dpal & 0x8000) | (*spal++ & 0x7fff);
				dpal++;
			}

			WriteProcessMemory(pi.hProcess, (void *)addr, (void *)newpaldata, 512, 0);
		} else {
			paletteNo[paletteCounter] = -1;
		}
	} else {
		paletteNo[paletteCounter] = -1;
	}

	WORD zero = 0;
	WriteProcessMemory(pi.hProcess, (void *)addr, (void *)&zero, 2, 0);
}

int mainDatClass::TestPort( SOCKADDR_IN* TargetTemp ){
	if( !TargetTemp ) return status_error;

	int Res;
	SOCKET sTemp;
	SOCKADDR_IN Target;
	SOCKADDR_IN HereTemp;
	memset( &HereTemp, 0, sizeof(SOCKADDR_IN) );

	WaitForSingleObject( hMutex, INFINITE );
	Target = *TargetTemp;
	ReleaseMutex( hMutex );



	HereTemp.sin_family = AF_INET;
	HereTemp.sin_addr.s_addr = htonl( INADDR_ANY );
	HereTemp.sin_port = htons( 10000 + rand()%1000 );

	Res = status_error;

	sTemp = socket(AF_INET , SOCK_DGRAM , 0);
	if( sTemp != INVALID_SOCKET ){
		if( bind( sTemp, (SOCKADDR*)&HereTemp, sizeof(SOCKADDR_IN)) >= 0 ){

			BYTE data[8];
			data[0] = cmd_version;
			data[1] = cmd_casters;
			data[2] = cmd_space_2;
			data[3] = cmd_space_3;
			data[4] = cmd_testport;
			*(WORD*)&data[5] = myPort;

			testPortFlg = 0;
			int Counter = 0;
			for(;;){
				sendto( sTemp, (const char*)data, 7, 0, (SOCKADDR*)&Target, sizeof(SOCKADDR_IN) );
				Sleep(200);
				if( testPortFlg ){
					Res = status_ok;
					break;
				}
				if( Counter > 10 ){
					Res = status_bad;
					break;
				}
				if( GetEsc() ){
					break;
				}
				Counter++;
			}
		}
		closesocket(sTemp);
	}
	return Res;
}



typedef struct{
	WORD Port;
	SOCKET s;
}echoThStruct;

unsigned __stdcall echoTh( void* Address ){
	if( !Address ) return 1;
	echoThStruct* echoThData = (echoThStruct*)Address;

	SOCKADDR_IN addr;
	int size;
	int addrSize = sizeof(SOCKADDR_IN);
	size = recvfrom( echoThData->s, NULL, NULL, 0, (SOCKADDR*)&addr, &addrSize);
	if( size < 0 ){
		echoThData->Port = 0;
		return 1;
	}
	echoThData->Port = addr.sin_port;
	return 0;
}

int GetMyPortSub( SOCKET* s, WORD* myPortTemp ){
	if( !myPortTemp ) return 1;
	*myPortTemp = 0;

	DWORD res;
	echoThStruct echoThData;
	HANDLE hEchoTh;
	SOCKADDR_IN Echo;
	memset( &Echo, 0, sizeof(SOCKADDR_IN) );
	Echo.sin_family = AF_INET;
	Echo.sin_addr.s_addr = inet_addr( "127.0.0.1" );

	echoThData.Port = 0;
	echoThData.s = socket(AF_INET , SOCK_DGRAM , 0);
	if( echoThData.s == INVALID_SOCKET ) return 1;

	int Counter = 0;
	for(;;){
		Echo.sin_port = 10000 + rand()%10000;
		if( bind( echoThData.s, (SOCKADDR*)&Echo, sizeof(Echo) ) >= 0 ) break;
		if( Counter > 100 ) {
			closesocket(echoThData.s);
			return 1;
		}
		Counter++;
		Sleep( 1 );
	}


	hEchoTh = (HANDLE)_beginthreadex(NULL, 0, echoTh, &echoThData, 0, NULL);
	if( !hEchoTh ) return 1;

	Counter = 0;
	for(;;){
		sendto( *s, NULL, 0, 0, (SOCKADDR*)&Echo, sizeof(Echo) );

		res = WaitForSingleObject( hEchoTh, 10 );
		if( echoThData.Port ){
			*myPortTemp = echoThData.Port;
			break;
		}
		if( res == WAIT_OBJECT_0 || res == WAIT_FAILED ){
			closesocket(echoThData.s);
			return 1;
		}
		if( Counter > 20 ) return 1;
		Counter++;
	}
	closesocket(echoThData.s);
	Sleep(10);
	CloseHandle(hEchoTh);


	return 0;
}

int mainDatClass::GetMyPort(){
	WORD myPortTemp;
	if( GetMyPortSub( &s, &myPortTemp ) ){
//		cout << "Port : BAD ( UDP." << myPort << " Open failed )" << endl;
		cout << "Port : Unknown" << endl;
		myPort = 0;
		return 1;
	}
	myPort = ntohs( myPortTemp );
	cout << "Port : Using UDP." << dec << myPort << endl;
	return 0;
}


int mainDatClass::GetDelay( BYTE dest, float* delayTemp ){
	#if debug_mode_func
		cout << "debug : GetDelay()" << endl;
	#endif
	clock_t pTime;
	delayTimeObsNo = 0;
	DWORD timeTemp;


	//取り扱う時間の精度を設定
	TIMECAPS timeCaps;
	WORD timeCapsFlg = 0;
	if( timeGetDevCaps( &timeCaps, sizeof(timeCaps) ) == TIMERR_NOERROR ){
		if( timeBeginPeriod( timeCaps.wPeriodMin ) == TIMERR_NOERROR ){
			timeCapsFlg = 1;
		}
	}

	for(;;){
		timeTemp = timeGetTime();
		SendCmd( dest, cmd_delayobs, &timeTemp, 4 );
		if( delayTimeObsNo > 4 ) break;
		if( GetEsc() ) return 1;
		Sleep(50);
	}


	//時間の精度の設定を戻す
	if( timeCapsFlg ){
		if( TIMERR_NOERROR != timeEndPeriod( timeCaps.wPeriodMin ) ) return 1;
	}


	*delayTemp = ( delayTimeObs[ 0 ] + delayTimeObs[ 1 ] + delayTimeObs[ 2 ] + delayTimeObs[ 3 ] + delayTimeObs[ 4 ] ) /  5 ;
	return 0;
}

int mainDatClass::GetPlayerSide(){
	#if debug_mode_func
		cout << "debug : GetPlayerSide()" << endl;
	#endif
	myRandNo = 1;
	if( playerSideFlg == 1 ){
		myRand = 0xFF;
	}else if( playerSideFlg == 2 ){
		myRand = 1;
	}else{
		myRand = (BYTE)( 1 + rand()%254 );
	}
	BYTE myRandTemp = myRand;

	myInfo.playerSide = 0;
	enInfo.playerSide = 0;

	enRandNo = 0;
	enRand = 0;
	for(;;){
//		cout << (WORD)myRandNo << "." << (WORD)myRand << endl;
//		if( enInfo.playerSide ) break;
		if( myRandNo == enRandNo ){

			if( enRand == myRand ){
				if( myRandNo > 10 ) srand( (unsigned)time( NULL ) );

				if( playerSideFlg && playerSideHostFlg && enRandNo < 10 ){
					if( playerSideFlg == 1 ){
						myRand = 0xFF;
					}else{
						myRand = 1;
					}
				}else if( playerSideFlg && !playerSideHostFlg && enRandNo < 5 ){
					if( playerSideFlg == 1 ){
						myRand = 0xFF;
					}else{
						myRand = 1;
					}
				}else{
					while( myRandTemp == myRand ){
						myRand = (BYTE)( 1 + rand() % 254 );
					}
				}

				if( myRandNo > 250 ){
					return 1;
				}else{
					myRandNo = myRandNo + 1;
				}
				myRandTemp = myRand;
			}else{
				if( myRand > enRand ){
					myInfo.playerSide = 0xA;
					enInfo.playerSide = 0xB;
				}else{
					myInfo.playerSide = 0xB;
					enInfo.playerSide = 0xA;
				}
				break;
			}
		}else if( myRandNo < enRandNo ){
			if( playerSideFlg && playerSideHostFlg && enRandNo < 10 ){
				if( playerSideFlg == 1 ){
					myRand = 0xFF;
				}else{
					myRand = 1;
				}
			}else if( playerSideFlg && !playerSideHostFlg && enRandNo < 5 ){
				if( playerSideFlg == 1 ){
					myRand = 0xFF;
				}else{
					myRand = 1;
				}
			}else{
				while( myRandTemp == myRand ){
					myRand = (BYTE)( 1 + rand() % 254 );
				}
			}
			myRandNo = enRandNo;
			myRandTemp = myRand;
		}
		SendCmd( dest_away, cmd_rand );
//		SendCmd( dest_away, cmd_playerside );
		if( GetEsc() ) return 1;
		Sleep(50);
	}

	if( !myInfo.playerSide ){
		if( enInfo.playerSide == 0xA ){
			myInfo.playerSide = 0xB;
		}else if( enInfo.playerSide == 0xB ){
			myInfo.playerSide = 0xA;
		}
	}
	//要検討
	enInfo.playerSide = 0;
	for(;;){
		if( enInfo.playerSide ) break;
		SendCmd( dest_away, cmd_playerside );
		if( GetEsc() ) return 1;
		Sleep(100);
	}
	return 0;
}

int mainDatClass::GetEsc(){

	if( escSelectFlg ){
		HWND  hForWnd;
		DWORD PID;

		hForWnd = GetForegroundWindow();
		if( hForWnd ){
			GetWindowThreadProcessId( hForWnd , &PID );
			if( GetCurrentProcessId() == PID ){
				if(GetKeyState(VK_ESCAPE)<0) return 1;
			}else if( th075Flg ){
				if( hForWnd == FindWindow( NULL , windowName ) ){
					if(GetKeyState(VK_ESCAPE)<0) return 1;
				}
			}
		}else{
			if(GetKeyState(VK_ESCAPE)<0) return 1;
		}
	}else{
		if(GetKeyState(VK_ESCAPE)<0) return 1;
	}
	return 0;
}

int mainDatClass::WriteCode( void* Address, BYTE code ){
	if( !th075Flg ) return 1;
	WriteProcessMemory( hProcess , Address , &code  , 1 , NULL );
	return 0;
}

int mainDatClass::UnRockTime(){
	if( !th075Flg ) return 1;
	BYTE code[6];
	code[0] = 0x8B;	//MOV EAX,DWORD PTR SS:[EBP-184]
	code[1] = 0x85;
	code[2] = 0x7C;
	code[3] = 0xFE;
	code[4] = 0xFF;
	code[5] = 0xFF;
	WriteMemory( (void*)0x602FA1, code, 6 );
	FlushInstructionCache(pi.hProcess, NULL, 0);
	rockFlg = 0;
	
	if (rollbackOk) {
		WriteCode((void *)rollbackLockModeAddr, 0);
	}
	
	return 0;
}
int mainDatClass::RockTime(){
	if( !th075Flg ) return 1;
	BYTE code[6];
	code[0] = 0xE9;	//JMP 00603539
	code[1] = 0x93;
	code[2] = 0x05;
	code[3] = 0x00;
	code[4] = 0x00;
	code[5] = 0x90;	//NOP
	WriteMemory( (void*)0x602FA1, code, 6 );
	FlushInstructionCache(pi.hProcess, NULL, 0);
	rockFlg = 1;
	
	if (rollbackOk) {
		WriteCode((void *)rollbackLockModeAddr, 1);
	}
	
	return 0;
}

int mainDatClass::SetBodyBreakPoint(){
	if( !th075Flg ) return 1;
	BYTE code[14];
	code[0] = 0xCC;	//INT3
	code[1] = 0x8B;	//MOV EAX,DWORD PTR SS:[EBP-184]
	code[2] = 0x85;
	code[3] = 0x7C;
	code[4] = 0xFE;
	code[5] = 0xFF;
	code[6] = 0xFF;
	code[7] = 0x8B;	//MOV EDX,DWORD PTR DS:[EAX]
	code[8] = 0x10;
	code[9] = 0x8B;	//MOV ECX,EAX
	code[10] = 0xC8;
	code[11] = 0x90;	//NOP
	code[12] = 0x90;	//NOP
	code[13] = 0x90;	//NOP
	
	int amount = 14;
	if (rollbackOk) {
		amount = 7;
	}
	
	ReadProcessMemory(hProcess, (void*)body_int3_address, bodyOldCode, amount, NULL);
	WriteProcessMemory(hProcess, (void*)body_int3_address, code, amount, NULL);
	return 0;
}

int mainDatClass::RemoveBodyBreakPoint(){
	if( !th075Flg ) return 1;

	int amount = 14;
	if (rollbackOk) {
		amount = 7;
	}
	
	WriteProcessMemory(hProcess, (void*)body_int3_address, bodyOldCode, amount, NULL);
	return 0;
}



//Char
int mainDatClass::SetCharBreakPoint(){
	if( !th075Flg ) return 1;
	BYTE code[2];
	code[0] = 0xCC;
	code[1] = 0x90;
	WriteProcessMemory(hProcess, (void*)0x445831, code, 2, NULL);
	return 0;
}
int mainDatClass::RemoveCharBreakPoint(){
	if( !th075Flg ) return 1;
	BYTE code[2];
	code[0] = 0x8B;
	code[1] = 0xEC;
	WriteProcessMemory(hProcess, (void*)0x445831, code, 2, NULL);
	return 0;
}

//rand
int mainDatClass::SetRandBreakPoint(){
	if( !th075Flg ) return 1;

	BYTE code[40];
	code[0] = 0x90;	//INT3 or NOP
	code[1] = 0x69;	//IMUL EAX, DWORD PTR DS : [66C000], 15A4E35	//66C000→rand_address
	code[2] = 0x05;
	/*
	code[3] = 0x00;
	code[4] = 0xC0;
	code[5] = 0x66;
	code[6] = 0x00;
	*/
	*(DWORD*)&code[3] = rand_address;
	code[7] = 0x35;
	code[8] = 0x4E;
	code[9] = 0x5A;
	code[10] = 0x01;
	code[11] = 0x40;	//INC EAX
	code[12] = 0xA3;	//MOV DWORD PTR DS : [66C000]	//66C000→rand_address
	/*
	code[13] = 0x00;
	code[14] = 0xC0;
	code[15] = 0x66;
	code[16] = 0x00;
	*/
	*(DWORD*)&code[13] = rand_address;
	code[17] = 0xC1;	//SHR EAX, 10
	code[18] = 0xE8;
	code[19] = 0x10;
	code[20] = 0x25;	//AND EAX, 7FFF
	code[21] = 0xFF;
	code[22] = 0x7F;
	code[23] = 0x00;
	code[24] = 0x00;

	#if rand_show
		code[25] = 0xCC;	//INT3
	#else
		code[25] = 0x90;	//NOP
	#endif
	code[26] = 0xEB;	//JMP SHORT 4555CB
	code[27] = 0x20;

	WriteMemory( (void*)rand_int3_address, code, 28 );

	return 0;
}
int mainDatClass::RemoveRandBreakPoint(){
	if( !th075Flg ) return 1;

	BYTE code[40];
	code[0] = 0x0F;	//MOVSX EAX,BYTE PTR DS:[66C238]
	code[1] = 0xBE;
	code[2] = 0x05;
	code[3] = 0x38;
	code[4] = 0xC2;
	code[5] = 0x66;
	code[6] = 0x00;
	code[7] = 0x83;	//CMP EAX, 2
	code[8] = 0xF8;
	code[9] = 0x02;
	code[10] = 0x75;	//JNZ SHORT 4555C6
	code[11] = 0x2B;
	code[12] = 0xB9;	//MOV ECX, 6718B8
	code[13] = 0xB8;
	code[14] = 0x18;
	code[15] = 0x67;
	code[16] = 0x00;
	code[17] = 0xE8;	//CALL 414620
	code[18] = 0x7B;
	code[19] = 0xF0;
	code[20] = 0xFB;
	code[21] = 0xFF;
	code[22] = 0x85;	//TEST EAX, EAX
	code[23] = 0xC0;
	code[24] = 0x76;	//JBE SHORT 4555C4
	code[25] = 0x1B;
	code[26] = 0xB9;
	code[27] = 0xB8;
	WriteMemory( (void*)rand_int3_address, code, 28 );

	return 0;
}

int mainDatClass::SetWindowResize() {
	if( !th075Flg ) return 1;

	// Kill the fader off if window is too big or if transitions are disabled, whacking dgraphics-error too
	if (windowWidth > 1024 || windowDisableTransitionsFlg) {
		DWORD frames = 0;
		WriteCode( (void *)0x432038, frames );
		WriteCode( (void *)0x432130, frames );
		// DGraphics-Error
		WriteCode( (void*)0x401a00, 0xc3 ); // RETN
	}

	if (windowWidth == 640) {
	    return 0;
	}

	windowMultiplier = (float)windowWidth / 640.0;

	// Window width/height
	WriteProcessMemory( pi.hProcess, (void *)0x602AF2, &windowWidth, 4, 0);
	WriteProcessMemory( pi.hProcess, (void *)0x602B18, &windowHeight, 4, 0);
	WriteProcessMemory( pi.hProcess, (void *)0x602DA5, &windowHeight, 4, 0);
	WriteProcessMemory( pi.hProcess, (void *)0x602DAA, &windowWidth, 4, 0);

	// Fader windowWidth/windowHeight
	WriteProcessMemory( pi.hProcess, (void *)0x43241A, &windowHeight, 4, 0);
	WriteProcessMemory( pi.hProcess, (void *)0x43241F, &windowWidth, 4, 0);
	WriteProcessMemory( pi.hProcess, (void *)0x43236A, &windowHeight, 4, 0);
	WriteProcessMemory( pi.hProcess, (void *)0x43236F, &windowWidth, 4, 0);

	WriteProcessMemory( pi.hProcess, (void *)0x66c084, &windowMultiplier, 4, 0);
	BYTE code[2] = {
		0xCC, // INT3
		0x90  // NOP
	};

	WriteMemory( (void*)render_scale_int3_address, code, 2);

	if (windowWidth > 1024) {
		WriteMemory( (void*)high_res_fix_int3_address, code, 2);
	}

	WriteMemory( (void*)fader_scale_int3_address1, code, 2);
	WriteMemory( (void*)fader_scale_int3_address2, code, 2);

	BYTE code2[2] = {
		0xCC, // INT3
		0xC3  // RETN
	};
	WriteMemory( (void*)fader_scale_int3_address3, code2, 2);
	WriteMemory( (void*)fader_scale_int3_address4, code2, 2);

	if (windowFilterState > 0) {
		// Force bilinear filtering of entire screen
		DWORD filter = 2; // D3DTEXF_LINEAR
		WriteCode( (void *)0x402440, filter);

		if (windowFilterState < 3) {
			// ... Except the character sprites.
			WriteMemory( (void*)sprite_filter_int3_address_start, code, 2);
			if (windowFilterState == 1) {
				WriteMemory( (void*)sprite_filter_int3_address_end2, code, 2);
			} else {
				WriteMemory( (void*)sprite_filter_int3_address_end, code, 2);
			}
		}
	}

	return 0;
}


int mainDatClass::SetCode(){
	if( !th075Flg ) return 1;

	//DirectInputを無効にする
	WriteCode( (void*)0x406E80, 0xC3 );	//RETN

	//ポーズのバッファを無効にする
	WriteCode( (void*)0x43B4C8, 0xEB );	//JMP SHORT

	// Replay
	WriteCode( (void*)replay_int3_address, 0xCC ); // INT3

	// Palette
	WriteCode( (void*)palette_int3_address, 0xCC ); // INT3

	//ランダム
	SetRandBreakPoint();
	
 	if( newCWflg ){
 		CONTEXT ctFPU;
 		ctFPU.ContextFlags = CONTEXT_FLOATING_POINT;
 		GetThreadContext( hProcessTh, &ctFPU );
 		ctFPU.FloatSave.ControlWord &= ~0x300; // normally 0x027f, now 0x007f
 		SetThreadContext( hProcessTh, &ctFPU );
//		cout << hex << "debug : FPU CW set " << ctFPU.FloatSave.ControlWord << endl;

		//replay name
		DWORD status;
		VirtualProtectEx( hProcess, (void*)0x65791F, 6, PAGE_READWRITE, &status);

		char repName[30];
		strcpy( repName, "rpycwc\0" );
		WriteMemory( (void*)0x65791F, repName, 6 );

		VirtualProtectEx( hProcess, (void*)0x65791F, 6, status, &status);
 	}

	return 0;
}
int mainDatClass::RemoveCode(){
	if( !th075Flg ) return 1;

	//DirectInputを有効にする
	WriteCode( (void*)0x406E80, 0x55 );	//PUSH EBP

	//ポーズのバッファを有効にする
	WriteCode( (void*)0x43B4C8 , 0x75 );	//JNZ	SHORT

	// Replay
	WriteCode( (void*)replay_int3_address, 0x51 ); // PUSH ECX

	// Palette
	WriteCode( (void*)palette_int3_address, 0x52 ); // PUSH EDX

	//ランダム
//	RemoveRandBreakPoint();

	//reset repName
	if( newCWflg ){
		//replay name
		DWORD status;
		VirtualProtectEx( hProcess, (void*)0x65791F, 6, PAGE_READWRITE, &status);

		char repName[30];
		strcpy( repName, "replay\0" );
		WriteMemory( (void*)0x65791F, repName, 6 );

		VirtualProtectEx( hProcess, (void*)0x65791F, 6, status, &status);
	}

	return 0;
}

int mainDatClass::SetAutoSave(){
	if( !th075Flg ) return 1;

	autoSaveFlg = 1;

	//Default option should be save replay...
	WriteCode( (void*)0x43BCBE, 2 );
	//Initializating at 13, 3 instead of 0, 0
	WriteCode((void*)0x42B28D, 0x0e);
	WriteCode((void*)0x42B296, 0x03);
	//Preventing B spam from messing it up
	WriteCode((void*)0x42B543, 0x0e);
	WriteCode((void*)0x42B54C, 0x03);
	//Preventing arrow mashing from messing it up
	WriteCode((void*)0x42BDA0, 0xB9);
	WriteCode((void*)0x42BDA1, 0x0E);
	WriteCode((void*)0x42BDA2, 0x00);
	WriteCode((void*)0x42BE0B, 0xB9);
	WriteCode((void*)0x42BE0C, 0x0E);
	WriteCode((void*)0x42BE0D, 0x00);
	WriteCode((void*)0x42BE76, 0xB9);
	WriteCode((void*)0x42BE77, 0x03);
	WriteCode((void*)0x42BE78, 0x00);
	WriteCode((void*)0x42BECC, 0xB9);
	WriteCode((void*)0x42BECD, 0x03);
	WriteCode((void*)0x42BECE, 0x00);

	return 0;
}

int mainDatClass::RemoveAutoSave(){
	if( !th075Flg ) return 1;

	autoSaveFlg = 0;

	if(replaySaveFlg==0) {
		WriteCode( (void*)0x43BCBE, 1 );
	}
	WriteCode((void*)0x42B28D, 0x00);
	WriteCode((void*)0x42B296, 0x00);
	WriteCode((void*)0x42B543, 0x00);
	WriteCode((void*)0x42B54C, 0x00);
	WriteCode((void*)0x42BDA0, 0x83);
	WriteCode((void*)0x42BDA1, 0xC1);
	WriteCode((void*)0x42BDA2, 0x01);
	WriteCode((void*)0x42BE0B, 0x83);
	WriteCode((void*)0x42BE0C, 0xE9);
	WriteCode((void*)0x42BE0D, 0x01);
	WriteCode((void*)0x42BE76, 0x83);
	WriteCode((void*)0x42BE77, 0xC1);
	WriteCode((void*)0x42BE78, 0x01);
	WriteCode((void*)0x42BECC, 0x83);
	WriteCode((void*)0x42BECD, 0xE9);
	WriteCode((void*)0x42BECE, 0x01);

	return 0;
}

int mainDatClass::hookDLLCode(const WCHAR *name, bool useNowDir) {
	HMODULE hKernel32 = GetModuleHandle("Kernel32");
	LPTHREAD_START_ROUTINE pLoadLibrary =
		(LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");

	WCHAR dll_path[1024];
	int dll_path_len;
	
	dll_path[0] = '\0';
	
	if (useNowDir) {
		lstrcpyW(dll_path, nowDir);
	} else {
		if (iniPath) {
			GetPrivateProfileStringW( isRollIni?L"ADVANCED":L"MAIN", L"th075File", L"", dll_path, 200, iniPath);
			
			PathRemoveFileSpecW(dll_path);
		}
	}
	
	if (!dll_path[0]) {
		lstrcpyW(dll_path, nowDir);
	}
	
	lstrcatW(dll_path, name);
	
	if (GetFileAttributesW(dll_path) == 0xffffffff) {
		return 0;
	}
	
	dll_path_len = (lstrlenW(dll_path)+1)*2;
	
	void *dll_addr = VirtualAllocEx(pi.hProcess, 0, dll_path_len, MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(pi.hProcess, dll_addr, dll_path, dll_path_len, 0);
	
	HANDLE hThread = CreateRemoteThread(pi.hProcess, 0, 0, pLoadLibrary, dll_addr, 0, 0);
	
	if (hThread == 0) {
		return 0;
	}
	
	return 1;	
}

int mainDatClass::hookEnglishDLL() {
	return hookDLLCode(L"\\th075e.dll", 0);
}

int mainDatClass::hookStateDLL() {
	rollbackFlg = GetPrivateProfileIntW( L"ROLLBACK", L"enable", 1, iniPath );
	rollbackMinInputDelay = GetPrivateProfileIntW( L"ROLLBACK", L"minInputDelay", rollbackMinInputDelay, iniPath );
	rollbackMaxRewind = GetPrivateProfileIntW( L"ROLLBACK", L"maxRewind", rollbackMaxRewind, iniPath );
	
	if (!rollbackFlg) {
		return 0;
	}
	
	// check caster folder first
	int res = hookDLLCode(L"\\state.dll", 1);
	
	if (!res) {
		res = hookDLLCode(L"\\state.dll", 0);
	}
	
	if (res) {
		rollbackOk = 1;
	}

	if (rollbackFlg && !rollbackOk) {
		cout << "ERROR : Could not load state.dll : rewinding disabled" << endl;
	}
	
	return res;
}

void mainDatClass::getHookAddresses() {
	if (rollbackOk) {
		ReadMemory((void *)0x68fffc, &rollbackRewindAddr, 4);
	 	ReadMemory((void *)0x68fff8, &rollbackLockModeAddr, 4);
	}
}

void mainDatClass::runKeybind(int key) {
	switch(key) {
	case KEY_AUTOSAVE_ON:
		SetAutoSave();
		cout << "setting : AutoSave on" << endl;
		break;
	case KEY_AUTOSAVE_OFF:
		RemoveAutoSave();
		cout << "setting : AutoSave off" << endl;
		break;
	case KEY_AUTOSAVE_TOGGLE:
		if (autoSaveFlg) {
			runKeybind(KEY_AUTOSAVE_OFF);
		} else {
			runKeybind(KEY_AUTOSAVE_ON);
		}
		break;
	case KEY_AUTONEXT_ON:
		autoNextFlg = 1;
		cout << "setting : AutoNext on" << endl;
		break;
	case KEY_AUTONEXT_OFF:
		autoNextFlg = 0;
		cout << "setting : AutoNext off" << endl;
		break;
	case KEY_AUTONEXT_TOGGLE:
		if (autoNextFlg) {
			runKeybind(KEY_AUTONEXT_OFF);
		} else {
			runKeybind(KEY_AUTONEXT_ON);
		}
		break;
	case KEY_ROUNDCOUNT_CYCLE:
		roundShowFlg = (roundShowFlg+1)%3;
		cout << "setting : RoundShow " << roundShowFlg << " : ";
		if (roundShowFlg == 1) {
			cout << "Print end of session summary." << endl;
		} else if (roundShowFlg == 2) {
			cout << "Print all match info." << endl;
		} else {
			cout << "Print nothing." << endl;
		}
		break;
	case KEY_BGM_TOGGLE:
		if (bgmToggleMode == 0) {
			bgmToggleMode = 1;
			bgmOrigVolume = 0;
			ReadProcessMemory(pi.hProcess, (void *)0x6714b7, (void *)&bgmOrigVolume, 1, 0);
		} else {
			bgmToggleMode = 3;
		}
		break;
	case KEY_ALWAYSONTOP_TOGGLE:
		toggleWindowTopFlg = 1;
		break;
	default:
		// anything else is handled by the specific chunks of code that use them
		break;
	}
}

void mainDatClass::updateKeybinds() {
	HWND hFocus = GetForegroundWindow();
	if (!hWnd || (hFocus && hFocus != hWnd)) {
		for (int i = 0; i < KEY_COUNT; ++i) {
			keystate[i] = 0;
		}

		return;
	}
	
	for (int i = 0; i < KEY_COUNT; ++i) {
		WORD key = keybinds[i];
		if (key != 0) {
			if (GetKeyState(key) < 0) {
				if (keystate[i] < 2) {
					keystate[i]++;
				}
				if (keystate[i] == 1) {
					runKeybind(i);
				}
			} else {
				keystate[i] = 0;
			}
		}
	}
}

int mainDatClass::ReadMemory(void* Address, void* data, DWORD size){
	DWORD	sizeRet;
	if( !th075Flg ) return 1;
	ReadProcessMemory( hProcess , Address , data , size , &sizeRet );
	if(sizeRet != size) return 2;
	return 0;
}

int mainDatClass::WriteMemory(void* Address, void* data, DWORD size){
	DWORD	sizeRet;
	if( !th075Flg ) return 1;
	WriteProcessMemory( hProcess , Address , data , size , &sizeRet );
	if(sizeRet != size) return 2;
	return 0;
}

//データ送信
int mainDatClass::SendData(int dest, void* Address, DWORD size){
	return SendDataSub( dest, Address, size, task_main );
}
int mainDatClass::SendData(SOCKADDR_IN* addr, void* Address, DWORD size){
	return SendDataSub( addr, Address, size, task_main );
}

int mainDatClass::SendDataR(int dest, void* Address, DWORD size){
	return SendDataSub( dest, Address, size, task_recv );
}
int mainDatClass::SendDataR(SOCKADDR_IN* addr, void* Address, DWORD size){
	return SendDataSub( addr, Address, size, task_recv );
}

int mainDatClass::SendDataSub( int dest, void* Address, DWORD size, WORD Flg ){
	if(size > stask_buf_size) return 1;

	int startValue = 0;
	int endValue = 0;
	if( Flg == task_main ){
		startValue = 0;
		endValue = 19;
	}else if( Flg == task_recv ){
		startValue = 20;
		endValue = 39;
	}else if( Flg == task_manage ){
		startValue = 40;
		endValue = 49;
	}

	int Counter;
	for(Counter=startValue; Counter<=endValue; Counter++){
		if(sTask[Counter].Flg == 0){
			sTask[Counter].dest = dest;
			memcpy( sTask[Counter].data, Address, size);
			sTask[Counter].size = size;
			sTask[Counter].Flg = stask_data;

			SetEvent(hSendEvent);
			break;
		}
		if(Counter==endValue){
			return 1;
		}
	}
	return 0;
}

int mainDatClass::SendDataSub( SOCKADDR_IN* addr, void* Address, DWORD size, WORD Flg ){
	if(size > stask_buf_size) return 1;

	int startValue = 0;
	int endValue = 0;
	if( Flg == task_main ){
		startValue = 0;
		endValue = 19;
	}else if( Flg == task_recv ){
		startValue = 20;
		endValue = 39;
	}else if( Flg == task_manage ){
		startValue = 40;
		endValue = 49;
	}

	int Counter;
	for(Counter=startValue; Counter<=endValue; Counter++){
		if(sTask[Counter].Flg == 0){
			sTask[Counter].dest = dest_addr;
			WaitForSingleObject( hMutex, INFINITE );
			sTask[Counter].addr = *addr;
			ReleaseMutex( hMutex );

			memcpy( sTask[Counter].data, Address, size);
			sTask[Counter].size = size;
			sTask[Counter].Flg = stask_data;

			SetEvent(hSendEvent);
			break;
		}
		if(Counter==endValue){
			return 1;
		}
	}
	return 0;
}


//連結データ送信
int mainDatClass::SendData(int dest, void* Address, DWORD size, void* Address2, DWORD size2){
	return SendDataSub( dest, Address, size, Address2, size2, task_main );
}
int mainDatClass::SendData(SOCKADDR_IN* addr, void* Address, DWORD size, void* Address2, DWORD size2){
	return SendDataSub( addr, Address, size, Address2, size2, task_main );
}

int mainDatClass::SendDataR(int dest, void* Address, DWORD size, void* Address2, DWORD size2){
	return SendDataSub( dest, Address, size, Address2, size2, task_recv );
}
int mainDatClass::SendDataR(SOCKADDR_IN* addr, void* Address, DWORD size, void* Address2, DWORD size2){
	return SendDataSub( addr, Address, size, Address2, size2, task_recv );
}
int mainDatClass::SendDataR(int dest, void* Address, DWORD size, void* Address2, DWORD size2, void* Address3, DWORD size3 ){
	return SendDataSub( dest, Address, size, Address2, size2, Address3, size3, task_recv );
}
int mainDatClass::SendDataR(SOCKADDR_IN* addr, void* Address, DWORD size, void* Address2, DWORD size2, void* Address3, DWORD size3 ){
	return SendDataSub( addr, Address, size, Address2, size2, Address3, size3, task_recv );
}

int mainDatClass::SendDataSub( int dest, void* Address, DWORD size, void* Address2, DWORD size2, void* Address3, DWORD size3, WORD Flg ){
	if(size + size2 + size3 > stask_buf_size) return 1;

	int startValue = 0;
	int endValue = 0;
	if( Flg == task_main ){
		startValue = 0;
		endValue = 19;
	}else if( Flg == task_recv ){
		startValue = 20;
		endValue = 39;
	}else if( Flg == task_manage ){
		startValue = 40;
		endValue = 49;
	}

	int Counter;
	for(Counter=startValue; Counter<=endValue; Counter++){
		if(sTask[Counter].Flg == 0){
			sTask[Counter].dest = dest;
			memcpy( sTask[Counter].data, Address, size);
			memcpy( sTask[Counter].data + size, Address2, size2);
			memcpy( sTask[Counter].data + size + size2, Address3, size3);
			sTask[Counter].size = size + size2 + size3;
			sTask[Counter].Flg = stask_data;

			SetEvent(hSendEvent);
			break;
		}
		if(Counter==endValue){
			return 1;
		}
	}
	return 0;
}

int mainDatClass::SendDataSub( SOCKADDR_IN* addr, void* Address, DWORD size, void* Address2, DWORD size2, void* Address3, DWORD size3, WORD Flg ){
	if(size + size2 + size3 > stask_buf_size) return 1;

	int startValue = 0;
	int endValue = 0;
	if( Flg == task_main ){
		startValue = 0;
		endValue = 19;
	}else if( Flg == task_recv ){
		startValue = 20;
		endValue = 39;
	}else if( Flg == task_manage ){
		startValue = 40;
		endValue = 49;
	}

	int Counter;
	for(Counter=startValue; Counter<=endValue; Counter++){
		if(sTask[Counter].Flg == 0){
			sTask[Counter].dest = dest_addr;

			WaitForSingleObject( hMutex, INFINITE );
			sTask[Counter].addr = *addr;
			ReleaseMutex( hMutex );
			memcpy( sTask[Counter].data, Address, size);
			memcpy( sTask[Counter].data + size, Address2, size2);
			memcpy( sTask[Counter].data + size + size2, Address3, size3);
			sTask[Counter].size = size + size2 + size3;
			sTask[Counter].Flg = stask_data;

			SetEvent(hSendEvent);
			break;
		}
		if(Counter==endValue){
			return 1;
		}
	}
	return 0;
}


int mainDatClass::SendDataSub( int dest, void* Address, DWORD size, void* Address2, DWORD size2, WORD Flg ){
	if(size + size2 > stask_buf_size) return 1;

	int startValue = 0;
	int endValue = 0;
	if( Flg == task_main ){
		startValue = 0;
		endValue = 19;
	}else if( Flg == task_recv ){
		startValue = 20;
		endValue = 39;
	}else if( Flg == task_manage ){
		startValue = 40;
		endValue = 49;
	}

	int Counter;
	for(Counter=startValue; Counter<=endValue; Counter++){
		if(sTask[Counter].Flg == 0){
			sTask[Counter].dest = dest;
			memcpy( sTask[Counter].data, Address, size);
			memcpy( sTask[Counter].data + size, Address2, size2);
			sTask[Counter].size = size + size2;
			sTask[Counter].Flg = stask_data;

			SetEvent(hSendEvent);
			break;
		}
		if(Counter==endValue){
			return 1;
		}
	}
	return 0;
}

int mainDatClass::SendDataSub( SOCKADDR_IN* addr, void* Address, DWORD size, void* Address2, DWORD size2, WORD Flg ){
	if(size + size2 > stask_buf_size) return 1;

	int startValue = 0;
	int endValue = 0;
	if( Flg == task_main ){
		startValue = 0;
		endValue = 19;
	}else if( Flg == task_recv ){
		startValue = 20;
		endValue = 39;
	}else if( Flg == task_manage ){
		startValue = 40;
		endValue = 49;
	}

	int Counter;
	for(Counter=startValue; Counter<=endValue; Counter++){
		if(sTask[Counter].Flg == 0){
			sTask[Counter].dest = dest_addr;

			WaitForSingleObject( hMutex, INFINITE );
			sTask[Counter].addr = *addr;
			ReleaseMutex( hMutex );
			memcpy( sTask[Counter].data, Address, size);
			memcpy( sTask[Counter].data + size, Address2, size2);
			sTask[Counter].size = size + size2;
			sTask[Counter].Flg = stask_data;

			SetEvent(hSendEvent);
			break;
		}
		if(Counter==endValue){
			return 1;
		}
	}
	return 0;
}

//アドレス
int mainDatClass::SendArea(int dest, void* Address, DWORD size){
	return SendAreaSub( dest, Address, size, task_main );
}
int mainDatClass::SendArea(SOCKADDR_IN* addr, void* Address, DWORD size){
	return SendAreaSub( addr, Address, size, task_main );
}

int mainDatClass::SendAreaR(int dest, void* Address, DWORD size){
	return SendAreaSub( dest, Address, size, task_recv );
}
int mainDatClass::SendAreaR(SOCKADDR_IN* addr, void* Address, DWORD size){
	return SendAreaSub( addr, Address, size, task_recv );
}

int mainDatClass::SendAreaSub( int dest, void* Address, DWORD size, WORD Flg ){
	if(size > stask_buf_size) return 1;

	int startValue = 0;
	int endValue = 0;
	if( Flg == task_main ){
		startValue = 0;
		endValue = 19;
	}else if( Flg == task_recv ){
		startValue = 20;
		endValue = 39;
	}else if( Flg == task_manage ){
		startValue = 40;
		endValue = 49;
	}

	int Counter;
	for(Counter=startValue; Counter<=endValue; Counter++){
		if(sTask[Counter].Flg == 0){
			sTask[Counter].dest = dest;
			sTask[Counter].Address = Address;
			sTask[Counter].size = size;
			sTask[Counter].Flg = stask_area;

			SetEvent(hSendEvent);
			break;
		}
		if(Counter == endValue){
			return 1;
		}
	}
	return 0;
}

int mainDatClass::SendAreaSub( SOCKADDR_IN* addr, void* Address, DWORD size, WORD Flg ){
	if(size > stask_buf_size) return 1;

	int startValue = 0;
	int endValue = 0;
	if( Flg == task_main ){
		startValue = 0;
		endValue = 19;
	}else if( Flg == task_recv ){
		startValue = 20;
		endValue = 39;
	}else if( Flg == task_manage ){
		startValue = 40;
		endValue = 49;
	}

	int Counter;
	for(Counter=startValue; Counter<=endValue; Counter++){
		if(sTask[Counter].Flg == 0){
			sTask[Counter].dest = dest_addr;
			WaitForSingleObject( hMutex, INFINITE );
			sTask[Counter].addr = *addr;
			ReleaseMutex( hMutex );

			sTask[Counter].Address = Address;
			sTask[Counter].size = size;
			sTask[Counter].Flg = stask_area;

			SetEvent(hSendEvent);
			break;
		}
		if(Counter==endValue){
			return 1;
		}
	}
	return 0;
}

//SendCmd()
int mainDatClass::SendCmd( int dest, BYTE Cmd ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendData( dest, data, 5 ) ) return 1;
	return 0;
}

int mainDatClass::SendCmd( SOCKADDR_IN* addr, BYTE Cmd ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendData( addr, data, 5 ) ) return 1;
	return 0;
}

int mainDatClass::SendCmdR( int dest, BYTE Cmd ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendDataR( dest, data, 5 ) ) return 1;
	return 0;
}

int mainDatClass::SendCmdR( SOCKADDR_IN* addr, BYTE Cmd ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendDataR( addr, data, 5 ) ) return 1;
	return 0;
}

//SendCmd+
int mainDatClass::SendCmd( int dest, BYTE Cmd, void* Address, DWORD size ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendData( dest, data, 5, Address, size ) ) return 1;
	return 0;
}

int mainDatClass::SendCmd( SOCKADDR_IN* addr, BYTE Cmd, void* Address, DWORD size ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendData( addr, data, 5, Address, size ) ) return 1;
	return 0;
}

int mainDatClass::SendCmdR( int dest, BYTE Cmd, void* Address, DWORD size ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendDataR( dest, data, 5, Address, size ) ) return 1;
	return 0;
}

int mainDatClass::SendCmdR( SOCKADDR_IN* addr, BYTE Cmd, void* Address, DWORD size ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendDataR( addr, data, 5, Address, size ) ) return 1;
	return 0;
}

//3
int mainDatClass::SendCmdR( int dest, BYTE Cmd, void* Address2, DWORD size2, void* Address3, DWORD size3 ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendDataR( dest, data, 5, Address2, size2, Address3, size3 ) ) return 1;
	return 0;
}

int mainDatClass::SendCmdR( SOCKADDR_IN* addr, BYTE Cmd, void* Address2, DWORD size2, void* Address3, DWORD size3 ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendDataR( addr, data, 5, Address2, size2, Address3, size3 ) ) return 1;
	return 0;
}

int mainDatClass::SendCmdM( int dest, BYTE Cmd ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendDataSub( dest, data, 5, task_manage ) ) return 1;
	return 0;
}

int mainDatClass::SendCmdM( SOCKADDR_IN* addr, BYTE Cmd ){
	BYTE data[5];
	data[0] = cmd_version;
	data[1] = cmd_casters;
	data[2] = cmd_space_2;
	data[3] = cmd_space_3;
	data[4] = Cmd;
	if( SendDataSub( addr, data, 5, task_manage ) ) return 1;
	return 0;
}
