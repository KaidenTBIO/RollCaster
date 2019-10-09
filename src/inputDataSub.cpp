#include "inputDataClass.h"
using namespace std;


int inputDataSubClass::init(){
	if( !data ) return 1;
	memset( &dataInfo, 0, sizeof( gameInfoStruct ) );
	
	//”O‚Ì‚½‚ß
	Sleep(100);
	
	memset( data, 0xFF, inputBuf_size );
	return 0;
}

inputDataSubClass::inputDataSubClass(){
	memset( &dataInfo, 0, sizeof(dataInfo) );
	data = (BYTE*)malloc( inputBuf_size );
}

inputDataSubClass::~inputDataSubClass(){
	if( data ){
		free( data );
		data = NULL;
	}
}

//void *memcpy(void *dest, void *src, size_t size);
int inputDataSubClass::SetInputDataArea( DWORD Time, BYTE* Address, WORD Size ){
	//—vŒŸ“¢
	if( Time >= inputBuf_size ) return 1;
	if( (ULONGLONG)Time + (ULONGLONG)Size >= inputBuf_size ) return 1;
	memcpy( &data[ Time ], Address, Size );
	return 0;
}
BYTE* inputDataSubClass::GetInputDataAddress( DWORD Time, WORD Size ){
	//—vŒŸ“¢
	if( Time >= inputBuf_size ) return 0;
	if( (ULONGLONG)Time + (ULONGLONG)Size >= inputBuf_size ) return 0;
	if( data[ Time + Size ] == 0xFF ) return 0;
	
	return &data[ Time ];
}


//SetInput
//Žb’è
void inputDataSubClass::SetInputDataSub( DWORD Time, BYTE Input ){
	if( Time < inputBuf_size ){
		if( data[ Time ] == 0xFF ){
			if( Input == 0xFF ) Input = 0;
			data[ Time ] = Input;
		}
	}
}
void inputDataSubClass::SetInputDataA( DWORD Time, BYTE Input ){
	SetInputDataSub( Time, Input );
}

void inputDataSubClass::SetInputDataB( DWORD Time, BYTE Input ){
	SetInputDataSub( Time + 1, Input );
}

//GetInput
//Žb’è
int inputDataSubClass::GetInputDataSub( DWORD Time, BYTE* Input ){
	if( Time >= inputBuf_size ) return 1;
	if( data[ Time ] == 0xFF ) return 1;
	if( Input ) *Input = data[ Time ];
	return 0;
}

int inputDataSubClass::GetInputDataA( DWORD Time, BYTE* Input ){
	if( GetInputDataSub( Time, Input ) ) return 1;
	return 0;
}

int inputDataSubClass::GetInputDataB( DWORD Time, BYTE* Input ){
	if( GetInputDataSub( Time + 1, Input ) ) return 1;
	return 0;
}
