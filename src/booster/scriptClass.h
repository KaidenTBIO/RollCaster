#ifndef TH075BOOSTER_SCRIPT
#define TH075BOOSTER_SCRIPT

#include "windows.h"
#include "conf.h"
using namespace std;

#define body_size 1024

typedef struct{
	char* div;
	char* arg1;
	char* arg2;
	char* op;
	
}contextStruct;


class scriptClass{
	public :
	
	scriptClass();
	~scriptClass();
	BYTE GetInput();
	
	char* now;
	char* body;
	
	int init( char* );
	int GetLine( char*, DWORD );
	int DivideLine( char* , contextStruct*);
	
};

#endif
