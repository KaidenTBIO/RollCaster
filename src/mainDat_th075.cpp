#include "mainDatClass.h"
using namespace std;

//http://nienie.com/~masapico/doc_ApiSpy.html
//↑のページ「MASAPICO'S Page」を参考

int mainDatClass::th075Roop( DWORD* deInfo ){
	CONTEXT ct;
	
	*deInfo = 0;
	/* デバッグの継続 */
	if( deInitFlg ){
		if( !ContinueDebugEvent(de.dwProcessId, de.dwThreadId, ContinueStatus) ){
			WaitForSingleObject( hPrintMutex, INFINITE );
			cout << "ERROR : ContinueDebugEvent" << endl;
			ReleaseMutex( hPrintMutex );
			return 1;
		}
	}else{
		deInitFlg = 1;
	}
	if( !WaitForDebugEvent(&de, INFINITE) ){
		WaitForSingleObject( hPrintMutex, INFINITE );
		cout << "ERROR : WaitForDebugEvent" << endl;
		ReleaseMutex( hPrintMutex );
		return 1;
	}

	ContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	
	switch(de.dwDebugEventCode) {
	case CREATE_PROCESS_DEBUG_EVENT:
		{
			// Linux/wine 1.1 has a really retarded bug here.
			//
			// On initial startup, it launches a _second_ process,
			// which in turn generates a second create process
			// debug event, which is obviously very bad for us.
			//
			// I don't even know what this process is.
			// Presumably something to do with DLLs, because
			// if another GUI process is already running, this
			// does not happen, but Caster itself does not
			// cause anything.
			//
			// Anyway, ignoring this secondary process, and
			// making it accept processThID errors down below
			// make it work.
			if (th075Flg == 1 && wineFlg) {
				break;
			}
			
			th075Flg = 1;
			hProcessTh  = de.u.CreateProcessInfo.hThread;
			processThID = de.dwThreadId;
			
			// stackbase is gotten down below, in de_body,
			// because it's a constant memory location.
			pStackBase = 0;

			//priority
			if( priorityFlg == 1 ){
				SetPriorityClass( hProcess, ABOVE_NORMAL_PRIORITY_CLASS );
				SetThreadPriority( hTh075Th, THREAD_PRIORITY_ABOVE_NORMAL );
			}
			if( priorityFlg == 2 ){
				SetPriorityClass( hProcess, HIGH_PRIORITY_CLASS );
				SetThreadPriority( hTh075Th, THREAD_PRIORITY_HIGHEST );
			}
			
			//windowMode
			//if( myInfo.terminalMode == mode_branch || myInfo.terminalMode == mode_subbranch || myInfo.terminalMode == mode_debug ){
				if( windowModeFlg ){
					WriteCode( (void*)0x41939D, 0xB5 );	//PUSH 6714B5
					WriteCode( (void*)0x41939E, 0x14 );
				}
			//}

			if( stageLimitCancelFlg ){
				//ステージ制限無効
				BYTE code[8];
				code[0] = 0xE9;
				code[1] = 0x9F;
				code[2] = 0x00;
				code[3] = 0x00;
				code[4] = 0x00;
				code[5] = 0x90;
				code[6] = 0x90;
				WriteMemory( (void*)0x43541D, code, 7 );
			}

			if( replaySaveFlg || autoSaveFlg==1 ){
				//対戦終了時のメニューのデフォルトをリプレイ保存にする
				WriteCode( (void*)0x43BCBE, 2 );
				if( autoSaveFlg==1 ) {
					SetAutoSave();
				}
			}

			bgmToggleMode = 0;

			// Kill DInput-Error box
			WriteCode( (void*)0x403d00, 0xc3 ); // RETN

			if (windowResizeFlg == 1) {
				SetWindowResize();
			}

			// this gets around another bug with Linux+wine and
			// SetThreadContext not actually setting the
			// fpu control word, by forcing a FLDCW instruction.
			// The original version of this apparently did not
			// actually work. This is some new code which does.
			if (newCWflg && wineFlg) {
				BYTE data[10] = { 0x7f, 0x00, 0xd9, 0x2d, 0xc6, 0x88, 0x43, 0x00, 0x90, 0x90 };
				WriteMemory( (void*)0x4388c6, data, 10);
				WriteCode( (void*)0x43347d, 0x47);
			}

			SetBodyBreakPoint();
			SetCharBreakPoint();
			
			// Get around some DLL loading/locking issues by
			// delaying the loading time until a little later.
			rollbackOk = 0;
			if (useth075eDLL || rollbackFlg) {
				BYTE data[8+7] = { 0x6a, 0x10, // PUSH 16
						   0xff, 0x15, 0x6c, 0x70, 0x65, 0x00, // CALL KERNEL32.Sleep
						   0xCC, // INT3
						   0xEB, 0xFD, // JMP SHORT YADA YADA
						   0x90, 0x90, 0x90, 0x90 };
				WriteMemory( (void*)(dllhook_int3_address-0x8), data, 7+8);
				dllHookState = 5;
			} else {
				dllHookState = 0;
			}
			
			FlushInstructionCache(pi.hProcess, NULL, 0);
		}
		break;

	case EXIT_THREAD_DEBUG_EVENT:
		if (dllHookState == 2 || dllHookState == 4) {
			if (de.u.ExitThread.dwExitCode == 0 && dllHookState == 2) {
				rollbackOk = 0;
				cout << "ERROR : state.dll failed to load : rewinding disabled" << endl;
			}
			dllHookState -= 1;
			
			WriteCode( (void*)dllhook_int3_address, 0xCC);
			FlushInstructionCache(pi.hProcess, NULL, 0);
		}
		break;

	case EXIT_PROCESS_DEBUG_EVENT:
		th075Flg = 0;

		return 1;

	case EXCEPTION_DEBUG_EVENT: /* 例外発生 */
		switch(de.u.Exception.ExceptionRecord.ExceptionCode) {
		case EXCEPTION_BREAKPOINT:
			/* ブレークポイントに遭遇した場合 */
			ct.ContextFlags = CONTEXT_CONTROL;
			
			if( de.dwThreadId != processThID ){
				WaitForSingleObject( hPrintMutex, INFINITE );
				if (wineFlg) {
					cout << "DEBUG : processThID " << endl;
				} else {
					cout << "ERROR : processThID " << endl;
				}
				ReleaseMutex( hPrintMutex );
				
				if (wineFlg) {
					// wine 1.1 bug, see CREATE_PROCESS_DEBUG_EVENT griping
					ContinueStatus = DBG_CONTINUE;
				
					break;
				}
				return 1;
			}
			
			if( !GetThreadContext(hProcessTh, &ct) ){
				WaitForSingleObject( hPrintMutex, INFINITE );
				cout << "ERROR : GetThreadContext" << endl;
				ReleaseMutex( hPrintMutex );
				return 1;
			}

			if(ct.Eip - 1 == body_int3_address) {
				*deInfo = de_body;
				
				if (pStackBase == 0) {
					// First pass, fetch some data.
					// can't remember which esp is in
					ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
					if( !GetThreadContext(hProcessTh, &ct) ){
						WaitForSingleObject( hPrintMutex, INFINITE );
						cout << "ERROR : GetThreadContext ( stack )" << endl;
						ReleaseMutex( hPrintMutex );
						return 1;
					}

					pStackBase = (void *)(ct.Esp + 996);

					// Read window name
					DWORD windowAddress;
					ReadProcessMemory( pi.hProcess, (void *)0x602c14, &windowAddress, 4, 0);
					
					windowName[159]= '\0';
					ReadProcessMemory( pi.hProcess, (void *)windowAddress, windowName, 159, 0);
				}
				
				// Thus ugly hack changes the bgm volume.
				// Basically, it does a fake function call to the bgm setting function and
				// removes its parameter so as not to wonk up the stack.
				if (bgmToggleMode == 1 || bgmToggleMode == 3) {
					ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
					if( !GetThreadContext(hProcessTh, &ct) ){
						WaitForSingleObject( hPrintMutex, INFINITE );
						cout << "ERROR : GetThreadContext ( volume )" << endl;
						ReleaseMutex( hPrintMutex );
						return 1;
					}

					ct.Esp -= 4;
					DWORD bgmFunc = 0x407dd0;
					WriteProcessMemory(pi.hProcess, (void *)ct.Esp, (void *)&ct.Eip, 4, 0);
					ct.Eip = bgmFunc;

					BYTE code[3] = { 0x90, 0x90, 0x90 };
					WriteProcessMemory(pi.hProcess, (void *)0x407e00, (void *)&code, 3, 0);

					BYTE volume = 0x20;

					if (bgmToggleMode == 1) {
						ReadProcessMemory(pi.hProcess, (void *)0x66c214, (void *)&volume, 1, 0);
						if (volume > 0x5) {
							volume -= 0x5;
						} else {
							volume = 0;
							bgmToggleMode = 2;
						}
					} else if (bgmToggleMode == 2) {
						volume = 0x0;
					} else if (bgmToggleMode == 3) {
						ReadProcessMemory(pi.hProcess, (void *)0x66c214, (void *)&volume, 1, 0);
						if (((WORD)volume)+0x5 > bgmOrigVolume) {
							volume = bgmOrigVolume;
							bgmToggleMode = 4;
						} else {
							volume += 0x5;
						}
					}

					WriteProcessMemory(pi.hProcess, (void *)0x66c214, (void *)&volume, 1, 0);

					ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
					if( !SetThreadContext(hProcessTh, &ct) ){
						WaitForSingleObject( hPrintMutex, INFINITE );
						cout << "ERROR : SetThreadContext ( volume )" << endl;
						ReleaseMutex( hPrintMutex );
						return 1;
					}

					FlushInstructionCache(pi.hProcess, NULL, 0);
				} else if (bgmToggleMode == 4) {
					BYTE code[3] = { 0x03, 0x45, 0x08 };

					WriteProcessMemory(pi.hProcess, (void *)0x407e00, (void *)&code, 3, 0);

					bgmToggleMode = 0;
				}

				updateKeybinds();
			}else if(ct.Eip - 1 == rand_int3_address){
				//rand
				int rand;
				ReadMemory( (void*)rand_address, &rand, 4 );
				rand = 22695477 * rand + 1;
				rand = (rand >> 16) & 0x7FFF;
				cout << hex << rand << endl;
			}else if(ct.Eip - 1 == rand_int3_address2){
				//rand
				CONTEXT ctRand;
				ctRand.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
				if( !GetThreadContext(hProcessTh, &ctRand) ){
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : GetThreadContext ( rand )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				DWORD loc;
				ReadProcessMemory(pi.hProcess, (void *)(ctRand.Esp+0xC), &loc, 4, 0);

			}else if(ct.Eip - 1 == char_int3_address){
				*deInfo = de_char;

				ct.ContextFlags = CONTEXT_CONTROL;
				ct.Ebp = ct.Esp;
				if(!SetThreadContext(hProcessTh, &ct)){
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : SetThreadContext ( char )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}
			}else if(ct.Eip - 1 == replay_int3_address) {
				ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
				if( !GetThreadContext(hProcessTh, &ct) ){
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : GetThreadContext ( replay )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				ct.Esp -= 4;
				WriteProcessMemory(pi.hProcess, (void *)ct.Esp, (void *)&ct.Ecx, 4, 0);

				if (!SetThreadContext(hProcessTh, &ct) ) {
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : SetThreadContext ( replay )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				if (ct.Ecx != 0) {
					int n;
					for (n = 0; n < 59; ++n) {
						ReadProcessMemory(pi.hProcess, (void *)(ct.Ecx+n), (void *)&replayFilename[n], 1, 0);
						if (!replayFilename[n]) {
							break;
						}
					}
					replayFilename[n] = '\0';
				} else {
					replayFilename[0] = '\0';
				}
				replaySavedFlg = 1;
			}else if(ct.Eip - 1 == palette_int3_address) {
				ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
				if( !GetThreadContext(hProcessTh, &ct) ){
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : GetThreadContext ( palette )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				ct.Esp -= 4;
				WriteProcessMemory(pi.hProcess, (void *)ct.Esp, (void *)&ct.Edx, 4, 0);

				if (!SetThreadContext(hProcessTh, &ct) ) {
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : SetThreadContext ( palette )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				DWORD addr;
				ReadProcessMemory(pi.hProcess, (void *)(ct.Ebp-0x4c), (void *)&addr, 4, 0);
				ReadProcessMemory(pi.hProcess, (void *)(addr+4), (void *)&addr, 4, 0);
				
				runPaletteUpdate(addr);

				paletteCounter = 1 - paletteCounter;
			}else if (ct.Eip - 1 == render_scale_int3_address) {
				// Scaling factor call

				// All of IaMP's rendering functions are scaled through a universal scaling factor
				// that gets updated when it switches between scene and UI graphics, so we simply
				// override that and multiply them by our own value to retain proper scale at
				// any resolution.

				// IaMP normally sends untranslated vertices to D3D so this is by far the least
				// annoying way to do things, but breaks the fader pretty bad.

				ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
				if( !GetThreadContext(hProcessTh, &ct) ){
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : GetThreadContext ( render_scale )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				ct.Ebp = ct.Esp;

				float f[3];
				ReadProcessMemory(pi.hProcess, (void *)(ct.Esp + 0x8), (void *)&f, sizeof(f), 0);
				f[0] *= windowMultiplier;
				f[1] *= windowMultiplier;
				f[2] *= windowMultiplier;
				WriteProcessMemory(pi.hProcess, (void *)(ct.Esp + 0x8), (void *)&f, sizeof(f), 0);

				if (!SetThreadContext(hProcessTh, &ct) ) {
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : SetThreadContext ( render_scale )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}
			}else if (ct.Eip - 1 == high_res_fix_int3_address) {
				// High resolutions (>1024) do not properly reset the screen offset
				// coordinates due to Direct3D whining about the fader texture not
				// being large enough for the screen. So we reset them here.
				ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
				if( !GetThreadContext(hProcessTh, &ct) ){
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : GetThreadContext ( high_res_fix )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				ct.Ebp = ct.Esp;

				float f = 0.0;
				WriteProcessMemory(pi.hProcess, (void *)0x66C084, (void *)&windowMultiplier, sizeof(windowMultiplier), 0);
				WriteProcessMemory(pi.hProcess, (void *)0x671220, (void *)&f, sizeof(f), 0);
				WriteProcessMemory(pi.hProcess, (void *)0x671224, (void *)&f, sizeof(f), 0);

				if (!SetThreadContext(hProcessTh, &ct) ) {
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : SetThreadContext ( high_res_fix )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}
			}else if (ct.Eip - 1 == fader_scale_int3_address1 || ct.Eip - 1 == fader_scale_int3_address2) {
				// Fader - Startup, reset to original scaling factor

				// Fader works by taking a screenshot of the screen, storing in a texture, and then
				// rendering the texture to the screen with some multiple value. Instead of, like,
				// drawing an alpha-blended rectangle over the screen to darken it. Dumb!
				// So we have to reset to normal screen scaling factors for this.
				ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
				if( !GetThreadContext(hProcessTh, &ct) ){
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : GetThreadContext ( fader_scale )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				ct.Ebp = ct.Esp;

				float f = 1.0;
				WriteProcessMemory(pi.hProcess, (void *)0x66C084, (void *)&f, sizeof(f), 0);

				if (!SetThreadContext(hProcessTh, &ct) ) {
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : SetThreadContext ( fader_scale )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}
			}else if (ct.Eip - 1 == fader_scale_int3_address3 || ct.Eip - 1 == fader_scale_int3_address4) {
				// Fader - Cleanup, reset to original scaling factor
				WriteProcessMemory(pi.hProcess, (void *)0x66C084, (void *)&windowMultiplier, sizeof(windowMultiplier), 0);
			}else if (ct.Eip - 1 == sprite_filter_int3_address_start || ct.Eip - 1 == sprite_filter_int3_address_end
				  || ct.Eip - 1 == sprite_filter_int3_address_end2) {
				// Disable bilinear filtering for sprites and possibly effects, then
				// turn them back on afterwards.
				ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
				if( !GetThreadContext(hProcessTh, &ct) ){
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : GetThreadContext ( sprite_filter )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				// To do this, we simply push the d3d call onto the stack and change the
				// instruction pointer. Additionally, since we're overriding a push call,
				// dump that on the stack too while we're here.
				DWORD d3dMemLoc, d3dCallTable, d3dSetTextureStageStateCall;

				ReadProcessMemory(pi.hProcess, (void *)0x671230, &d3dMemLoc, 4, 0);
				ReadProcessMemory(pi.hProcess, (void *)d3dMemLoc, &d3dCallTable, 4, 0);
				ReadProcessMemory(pi.hProcess, (void *)(d3dCallTable+0xFC), &d3dSetTextureStageStateCall, 4, 0);

				DWORD callTable[6];

				callTable[0] = ct.Eip;
				callTable[1] = d3dMemLoc;
				callTable[2] = 0; // stage 0
				callTable[3] = 16; // D3DTSS_MAGFILTER

				// off or on?
				if (ct.Eip -1 == sprite_filter_int3_address_start) {
					callTable[4] = 1; // D3DTEXF_POINT
				} else {
					callTable[4] = 2; // D3DTEXF_LINEAR
				}

				// address_end is a push 1 statement, both address_start and address_start0 are push 0
				if (ct.Eip - 1 == sprite_filter_int3_address_end) {
					callTable[5] = 1;
				} else {
					callTable[5] = 0;
				}

				ct.Esp -= 4*6;
				WriteProcessMemory(pi.hProcess, (void *)ct.Esp, callTable, 4*6, 0);

				ct.Eip = d3dSetTextureStageStateCall;

				if (!SetThreadContext(hProcessTh, &ct) ) {
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : SetThreadContext ( sprite_filter )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}

				FlushInstructionCache(pi.hProcess, NULL, 0);
			}else if (ct.Eip-1 == dllhook_int3_address) {
				// delays dll hooking until now, so other
				// stuff can get loaded up properly, then
				// restore the original code and continue
				// on our merry way.
				WriteCode( (void*)dllhook_int3_address, 0x90);
				FlushInstructionCache(pi.hProcess, NULL, 0);
				
				if (dllHookState == 5) {
					if (useth075eDLL) {
						int res = hookEnglishDLL();
					
						if (res) {
							dllHookState = 4;
						} else {
							dllHookState = 3;
						}
					} else {
						dllHookState = 3;
					}
				}
				
				if (dllHookState == 3) {
					if (rollbackFlg) {
						int res = hookStateDLL();
					
						if (res) {
							dllHookState = 2;
						} else {
							dllHookState = 1;
						}
					} else {
						dllHookState = 1;
					}
				}
				
				if (dllHookState == 1) {
					getHookAddresses();
					
					ct.ContextFlags = CONTEXT_CONTROL;
					if( !GetThreadContext(hProcessTh, &ct) ){
						WaitForSingleObject( hPrintMutex, INFINITE );
						cout << "ERROR : GetThreadContext ( dllhook )" << endl;
						ReleaseMutex( hPrintMutex );
						return 1;
					}
				
					ct.Eip = ct.Eip - 1;
				
					BYTE data[8+7] = { 0xe8, 0x00, 0x00, 0x00, 0x0f, 0x95, 0xc0, 0xc3, 0x6a, 0x60, 0x68, 0x78, 0x0f, 0x66, 0x00 };
					WriteMemory((void *)(dllhook_int3_address-8), data, 7+8);
				
					if (!SetThreadContext(hProcessTh, &ct) ) {
						WaitForSingleObject( hPrintMutex, INFINITE );
						cout << "ERROR : SetThreadContext ( dllhook )" << endl;
						ReleaseMutex( hPrintMutex );
						return 1;
					}

					FlushInstructionCache(pi.hProcess, NULL, 0);
				}
			}

			//起動時に常に必要
			ContinueStatus = DBG_CONTINUE;

			break;

		case EXCEPTION_SINGLE_STEP: // シングルステップ実行例外
			// 再びブレークポイントを設置する

			{
				BYTE newCode = 0xCC;
				if( !WriteProcessMemory(pi.hProcess, (void*)body_int3_address, &newCode, 1, NULL) ){
					WaitForSingleObject( hPrintMutex, INFINITE );
					cout << "ERROR : Set new code ( body )" << endl;
					ReleaseMutex( hPrintMutex );
					return 1;
				}
			}
			FlushInstructionCache(pi.hProcess, NULL, 0);
			// シングルステップモードを中止
			ct.ContextFlags = CONTEXT_CONTROL;
			if(!GetThreadContext(hProcessTh, &ct)){
				WaitForSingleObject( hPrintMutex, INFINITE );
				cout << "ERROR : GetThreadContext ( single step )" << endl;
				ReleaseMutex( hPrintMutex );
				return 1;
			}
			ct.EFlags &= ~EFLAGS_TF;
			if(!SetThreadContext(hProcessTh, &ct)){
				WaitForSingleObject( hPrintMutex, INFINITE );
				cout << "ERROR : SetThreadContext ( single step )" << endl;
				ReleaseMutex( hPrintMutex );
				return 1;
			}

			ContinueStatus = DBG_CONTINUE;
			break;
		}
		break;
		
	}


	return 0;
}

