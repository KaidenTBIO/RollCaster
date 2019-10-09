#ifndef CASTER_INPUTDATA
#define CASTER_INPUTDATA

#if _MSC_VER >= 1300
#	if !defined(_WINDOWS_)
#		if !defined(NO_WIN32_LEAN_AND_MEAN)
#		define WIN32_LEAN_AND_MEAN
#		define  _VCL_LEAN_AND_MEAN
#		endif
#	endif
#	include <windows.h>
#	include <iostream>
#else
#	include <windows.h>
#	include <iostream.h>
#endif

#include "const.h"
#include "etc.h"

class inputDataSubClass{
	private:
	BYTE* data;
	
	public:
	inputDataSubClass();
	~inputDataSubClass();
	int SetInputDataArea( DWORD, BYTE*, WORD );
	void SetInputDataSub( DWORD, BYTE );
	void SetInputDataA( DWORD, BYTE );
	void SetInputDataB( DWORD, BYTE );
	BYTE* GetInputDataAddress( DWORD, WORD );
	int GetInputDataSub( DWORD, BYTE* );
	int GetInputDataA( DWORD, BYTE* );
	int GetInputDataB( DWORD, BYTE* );
	int init();
	gameInfoStruct dataInfo;
};

class inputDataClass{
	private:
	inputDataSubClass inputDataSub[3];
	WORD now;
	WORD next;
	WORD prev;
	
	public:
	int	SetInputDataArea(WORD, DWORD, BYTE*, WORD);
	void	SetInputData(WORD, DWORD, WORD, BYTE);	//sessionNo, gameTime, playerSide, Input
	void	SetInputDataA(WORD, DWORD, BYTE);
	void	SetInputDataB(WORD, DWORD, BYTE);
	int	GetInputData(WORD, DWORD, WORD, BYTE*);
	int	GetInputDataA(WORD, DWORD, BYTE*);
	int	GetInputDataB(WORD, DWORD, BYTE*);
	
	int Start( gameInfoStruct* );
	BYTE* GetInputDataAddress( WORD, DWORD, WORD );
	int init();
	DWORD GetTime( BYTE );
	void SetTime(BYTE, DWORD);
};

#endif
