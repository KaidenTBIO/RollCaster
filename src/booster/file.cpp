#include "conf.h"
#include "boosterDatClass.h"


//OpenDat(char* 開くファイルの名前, char* 展開する場所, DWORD 展開する場所のサイズ)
//CloseDat(char* 書き出すファイルの名前, char* 圧縮を開始する場所, DWORD 圧縮するサイズ)

#define dat_ok 0
#define dat_error_open 1
#define dat_error_init 2
#define dat_error_inflate 3
#define dat_error_overflow 4
#define dat_error_size 5
#define dat_error_end 6


int OpenDat(char* datName, char* AIaddress, DWORD AIsize){
	
	ifstream datopen(datName, ios::binary);
	if( datopen.fail() ){
		return dat_error_open;	//ファイルオープンエラー
	}
	
	if(AIsize == 0){
		//開かなくても同じ
		return dat_ok;
	}
	
	//ここからzlibで展開
	z_stream z;
	char inBuf[inBuf_size];
	int status, flush;
	
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	z.next_in = Z_NULL;
	z.avail_in = 0;
	if (inflateInit(&z) != Z_OK) {
		return dat_error_init;	//初期化エラー
	}
	
	z.next_out = (Bytef*)AIaddress;
	z.avail_out = AIsize;
	status = Z_OK;
	flush = Z_NO_FLUSH;
	
	
	while (status != Z_STREAM_END){
		if (z.avail_in == 0 && flush != Z_FINISH){
			z.next_in = (Bytef*)inBuf;
			datopen.read(inBuf,inBuf_size);
			z.avail_in = datopen.gcount();
			if(z.avail_in < inBuf_size){
				flush = Z_FINISH;
			}
		}
		
		status = inflate(&z, flush);
		if (status == Z_STREAM_END) break;
		
		if (status != Z_OK) {
			inflateEnd(&z);
			return dat_error_inflate;	//展開エラー	//バッファに対してサイズが大きいものではZ_BUF_ERRORが出る
		}
		if (z.avail_out == 0) {
			//オーバーフローへの対処
			z.next_out = (Bytef*)AIaddress;
			z.avail_out = AIsize;
			while (status != Z_STREAM_END){
				if (z.avail_in == 0 && flush != Z_FINISH){
					z.next_in = (Bytef*)inBuf;
					datopen.read(inBuf,inBuf_size);
					z.avail_in = datopen.gcount();
					if(z.avail_in < inBuf_size){
						flush = Z_FINISH;
					}
				}
				status = inflate(&z, flush);
				
				if (z.avail_out != AIsize){
					inflateEnd(&z);
					return dat_error_overflow;	//オーバーフロー
				}
				if (status == Z_STREAM_END) {
					z.avail_out = 0;
					break;
				}
				if (status != Z_OK) {
					inflateEnd(&z);
					return dat_error_inflate;	//展開エラー
				}
			}
		}
	}
//	datopen.close();
	
	if(z.avail_out != 0){
		inflateEnd(&z);
		return dat_error_size;	//AIサイズエラー
	}
	if (inflateEnd(&z) != Z_OK) {
		return dat_error_end;	//開放エラー
	}
	
	return 0;
}


int CloseDat(char* datName, char* AIaddress, DWORD AIsize){
	
	ofstream datclose(datName, ios::binary);
	if ( datclose.fail() ) return dat_error_open;	//ファイルオープンエラー
	
	//ここからzlibで圧縮
	z_stream z;
	char inBuf[inBuf_size];
	char outBuf[outBuf_size];
	int count, flush, status;
	
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) return dat_error_init;	//初期化エラー
	
	z.avail_in = 0;
	z.next_out = (Bytef*)outBuf;
	z.avail_out = outBuf_size;
	flush = Z_NO_FLUSH;
	
	unsigned int AILeft = AIsize;
	unsigned int AICount = 0;
	
	for(;;){
		if(z.avail_in == 0 && flush != Z_FINISH){
			z.next_in = (Bytef*)inBuf;
			if(AILeft <= inBuf_size){
				memcpy((void*)inBuf,(void*)(AIaddress + AICount),AILeft);
				z.avail_in = AILeft;
				AICount = AICount + AILeft;
				AILeft = 0;
				flush = Z_FINISH;
			}else{
				memcpy((void*)inBuf,(void*)(AIaddress + AICount),inBuf_size);
				z.avail_in = inBuf_size;
				AICount = AICount + inBuf_size;
				AILeft = AILeft - inBuf_size;
			}
		}
		
		status = deflate(&z, flush);
		if (status == Z_STREAM_END) break;
		if (z.avail_out == 0) {
			datclose.write(outBuf,outBuf_size);
			z.next_out = (Bytef*)outBuf;
			z.avail_out = outBuf_size;
		}
	}
	
	count = outBuf_size - z.avail_out;
	if(count != 0){
		datclose.write(outBuf,count);
	}
	datclose.close();
	
	if (deflateEnd(&z) != Z_OK) return dat_error_end;	//開放エラー
	return 0;
}



int boosterDatClass::CloseBackAI(){
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".CloseBackAI() " << endl;
	#endif
	#if debug_mode_noAI
		return 0;
	#endif
	
	if(strcmp(backAIname,"init")==0 || strcmp(backAIname,"error")==0 || strcmp(backAIname,"second")==0){
		return 1;
	}
	
	
	char datName[50];
	char datNameTemp[50];
	
	strcpy(datName,"dat\\\0");
	strcat(datName, backAIname);
	CreateDirectory(datName, NULL);
	
	strcat(datName, "\\\0");
	strcat(datName, backAIname);
	strcpy(datNameTemp,datName);
	strcat(datName, "_back.dat\0");
	strcat(datNameTemp, "_back.tmp\0");
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Close" << endl;
	#endif
	
	Flg = CloseDat(datNameTemp, AI + AIsizeArray[0] + AIsizeArray[1] + AIsizeArray[2] + AIsizeArray[3], AIsizeArray[4]);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Close failed." << endl;
		#endif
		return 1;
	}
	
	
	Flg = remove(datName);				//AIDat.datを置き換える。	//そもそもファイルが無かったときエラー
	Flg = rename(datNameTemp,datName);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Replace failed." << endl;
		#endif
		return 1;
	}
	return 0;
}

int boosterDatClass::OpenBackAI(char AIName[16]){
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".OpenBackAI() " << endl;
	#endif
	
	strcpy(backAIname,AIName);
	
	#if debug_mode_noAI
		return 0;
	#endif
	
	char datName[50];
	strcpy(datName,"dat\\\0");
	strcat(datName, AIName);
	strcat(datName,"\\\0");
	strcat(datName, AIName);
	strcat(datName, "_back.dat\0");
	
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Open" << endl;
	#endif
	
	Flg = OpenDat(datName, AI + AIsizeArray[0] + AIsizeArray[1] + AIsizeArray[2] + AIsizeArray[3], AIsizeArray[4]);
	
	#if debug_mode_file
		if( Flg ){
			if( Flg == dat_error_open )	cout << "debug : " << hex << playerSide << "." << datName << " Open failed." << endl;
			if( Flg == dat_error_init )	cout << "debug : " << hex << playerSide << "." << datName << " error at init" << endl;
			if( Flg == dat_error_inflate )	cout << "debug : " << hex << playerSide << "." << datName << " error at inflate" << endl;
			if( Flg == dat_error_overflow )	cout << "debug : " << hex << playerSide << "." << datName << " overflow" << endl;
			if( Flg == dat_error_size )	cout << "debug : " << hex << playerSide << "." << datName << " AIsize error" << endl;
			if( Flg == dat_error_end )	cout << "debug : " << hex << playerSide << "." << datName << " error at end" << endl;
		}
	#endif
	
	if( Flg ){
		if( Flg == dat_error_open ){
			return 1;
		}else{
			if( Flg == dat_error_inflate || Flg == dat_error_overflow || Flg == dat_error_size ){
				char datNameError[50];
				strcpy(datNameError, datName);
				strcat(datNameError, ".error\0");
				remove(datNameError);
				rename(datName, datNameError);
				return 1;
			}else{
				strcpy(backAIname,"error\0");
				return 0xF;
			}
		}
	}
	
	return 0;
}



int boosterDatClass::CloseLocalAI(){
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".CloseLocalAI() " << endl;
	#endif
	#if debug_mode_noAI
		return 0;
	#endif
	
	if(strcmp(localAIname,"init")==0 || strcmp(localAIname,"error")==0 || strcmp(localAIname,"second")==0){
		return 1;
	}
	
	
	char datName[50];
	char datNameTemp[50];
	
	strcpy(datName,"dat\\\0");
	strcat(datName, localAIname);
	CreateDirectory(datName, NULL);
	
	strcat(datName, "\\\0");
	strcat(datName, localAIname);
	strcpy(datNameTemp,datName);
	strcat(datName, "_local.dat\0");
	strcat(datNameTemp, "_local.tmp\0");
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Close" << endl;
	#endif
	
	Flg = CloseDat(datNameTemp, AI + AIsizeArray[0] + AIsizeArray[1] + AIsizeArray[2], AIsizeArray[3]);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Close failed." << endl;
		#endif
		return 1;
	}
	
	
	Flg = remove(datName);				//AIDat.datを置き換える。	//そもそもファイルが無かったときエラー
	Flg = rename(datNameTemp,datName);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Replace failed." << endl;
		#endif
		return 1;
	}
	return 0;
}

int boosterDatClass::OpenLocalAI(char AIName[16]){
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".OpenLocalAI() " << endl;
	#endif
	
	strcpy(localAIname,AIName);
	
	#if debug_mode_noAI
		return 0;
	#endif
	
	char datName[50];
	strcpy(datName,"dat\\\0");
	strcat(datName, AIName);
	strcat(datName,"\\\0");
	strcat(datName, AIName);
	strcat(datName, "_local.dat\0");
	
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Open" << endl;
	#endif
	
	Flg = OpenDat(datName, AI + AIsizeArray[0] + AIsizeArray[1] + AIsizeArray[2], AIsizeArray[3]);
	
	#if debug_mode_file
		if( Flg ){
			if( Flg == dat_error_open )	cout << "debug : " << hex << playerSide << "." << datName << " Open failed." << endl;
			if( Flg == dat_error_init )	cout << "debug : " << hex << playerSide << "." << datName << " error at init" << endl;
			if( Flg == dat_error_inflate )	cout << "debug : " << hex << playerSide << "." << datName << " error at inflate" << endl;
			if( Flg == dat_error_overflow )	cout << "debug : " << hex << playerSide << "." << datName << " overflow" << endl;
			if( Flg == dat_error_size )	cout << "debug : " << hex << playerSide << "." << datName << " AIsize error" << endl;
			if( Flg == dat_error_end )	cout << "debug : " << hex << playerSide << "." << datName << " error at end" << endl;
		}
	#endif
	
	if( Flg ){
		if( Flg == dat_error_open ){
			return 1;
		}else{
			if( Flg == dat_error_inflate || Flg == dat_error_overflow || Flg == dat_error_size ){
				char datNameError[50];
				strcpy(datNameError, datName);
				strcat(datNameError, ".error\0");
				remove(datNameError);
				rename(datName, datNameError);
				return 1;
			}else{
				strcpy(backAIname,"error\0");
				return 0xF;
			}
		}
	}
	
	return 0;
}



int boosterDatClass::CloseIndividualAI(){
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".CloseIndividualAI() " << endl;
	#endif
	#if debug_mode_noAI
		return 0;
	#endif
	
	if(strcmp(individualAIenName,"second") ==0){
		for(Counter=0;Counter<16;Counter++){individualAIenName[Counter]='\0';}
		strcpy(individualAIenName,individualAImyName);
	}
	if(strcmp(individualAImyName,"init")==0 || strcmp(individualAIenName,"init")==0
	|| strcmp(individualAImyName,"error")==0 || strcmp(individualAIenName,"error")==0
	|| strcmp(individualAImyName,"second")==0){
		return 1;
	}
	
	char datName[50];
	char datNameTemp[50];
	
	strcpy(datName,"dat\\\0");
	strcat(datName, individualAImyName);
	CreateDirectory(datName, NULL);
	
	strcat(datName, "\\_vs_\0");
	strcat(datName, individualAIenName);
	strcpy(datNameTemp,datName);
	strcat(datName, ".dat\0");
	strcat(datNameTemp, ".tmp\0");
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Close" << endl;
	#endif
	
	Flg = CloseDat(datNameTemp, AI + AIsizeArray[0] + AIsizeArray[1], AIsizeArray[2]);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Close failed." << endl;
		#endif
		return 1;
	}
	
	Flg = remove(datName);				//AIDat.datを置き換える。	//そもそもファイルが無かったときエラー
	Flg = rename(datNameTemp,datName);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Replace failed." << endl;
		#endif
		return 1;
	}
	return 0;
}

int boosterDatClass::OpenIndividualAI(char AIName[16],char AINameI[16]){
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".OpenIndividualAI() " << endl;
	#endif
	
	if(strcmp(AINameI,"second") ==0){
		for(Counter=0;Counter<16;Counter++){AINameI[Counter]='\0';}
		strcpy(AINameI,AIName);
	}
	strcpy(individualAImyName,AIName);
	strcpy(individualAIenName,AINameI);
	
	#if debug_mode_noAI
		return 0;
	#endif
	
	char datName[50];
	strcpy(datName,"dat\\\0");
	strcat(datName, AIName);
	strcat(datName,"\\_vs_\0");
	strcat(datName, AINameI);
	strcat(datName, ".dat\0");
	
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Open" << endl;
	#endif
	
	Flg = OpenDat(datName, AI + AIsizeArray[0] + AIsizeArray[1], AIsizeArray[2]);
	
	#if debug_mode_file
		if( Flg ){
			if( Flg == dat_error_open )	cout << "debug : " << hex << playerSide << "." << datName << " Open failed." << endl;
			if( Flg == dat_error_init )	cout << "debug : " << hex << playerSide << "." << datName << " error at init" << endl;
			if( Flg == dat_error_inflate )	cout << "debug : " << hex << playerSide << "." << datName << " error at inflate" << endl;
			if( Flg == dat_error_overflow )	cout << "debug : " << hex << playerSide << "." << datName << " overflow" << endl;
			if( Flg == dat_error_size )	cout << "debug : " << hex << playerSide << "." << datName << " AIsize error" << endl;
			if( Flg == dat_error_end )	cout << "debug : " << hex << playerSide << "." << datName << " error at end" << endl;
		}
	#endif
	
	if( Flg ){
		if( Flg == dat_error_open ){
			return 1;
		}else{
			if( Flg == dat_error_inflate || Flg == dat_error_overflow || Flg == dat_error_size ){
				char datNameError[50];
				strcpy(datNameError, datName);
				strcat(datNameError, ".error\0");
				remove(datNameError);
				rename(datName, datNameError);
				return 1;
			}else{
				strcpy(backAIname,"error\0");
				return 0xF;
			}
		}
	}
	
	return 0;
}


int boosterDatClass::CloseSpellAI(){
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".CloseSpellAI() " << endl;
	#endif
	#if debug_mode_noAI
		return 0;
	#endif
	
	if(strcmp(spellAIname,"error")==0 || strcmp(spellAIname,"init")==0 || strcmp(spellAIname,"second")==0){return 1;}
	
	
	char datName[50];
	char datNameTemp[50];
	
	strcpy(datName,"dat\\\0");
	strcat(datName, spellAIname);
	CreateDirectory(datName, NULL);
	
	strcat(datName, "\\\0");
	strcat(datName, spellAIname);
	strcpy(datNameTemp,datName);
	strcat(datName, "_spellcard.dat\0");
	strcat(datNameTemp, "_spellcard.tmp\0");
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Close" << endl;
	#endif
	
	Flg = CloseDat(datNameTemp, AI + AIsizeArray[0], AIsizeArray[1]);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Close failed." << endl;
		#endif
		return 1;
	}
	
	Flg = remove(datName);				//AIDat.datを置き換える。	//そもそもファイルが無かったときエラー
	Flg = rename(datNameTemp,datName);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Replace failed." << endl;
		#endif
		return 1;
	}
	return 0;
}


int boosterDatClass::OpenSpellAI(char AIName[16]){		//AI開く
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".OpenSpellAI() " << endl;
	#endif
	
	strcpy(spellAIname,AIName);
	
	#if debug_mode_noAI
		return 0;
	#endif
	
	char datName[50];
	strcpy(datName,"dat\\\0");
	strcat(datName, AIName);
	strcat(datName,"\\\0");
	strcat(datName, AIName);
	strcat(datName, "_spellcard.dat\0");
	
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Open" << endl;
	#endif
	
	//start : AIsizeArray[0]
	//end   : AIsizeArray[0] + AIsizeArray[1]
	
	Flg = OpenDat(datName, AI + AIsizeArray[0], AIsizeArray[1]);
	
	#if debug_mode_file
		if( Flg ){
			if( Flg == dat_error_open )	cout << "debug : " << hex << playerSide << "." << datName << " Open failed." << endl;
			if( Flg == dat_error_init )	cout << "debug : " << hex << playerSide << "." << datName << " error at init" << endl;
			if( Flg == dat_error_inflate )	cout << "debug : " << hex << playerSide << "." << datName << " error at inflate" << endl;
			if( Flg == dat_error_overflow )	cout << "debug : " << hex << playerSide << "." << datName << " overflow" << endl;
			if( Flg == dat_error_size )	cout << "debug : " << hex << playerSide << "." << datName << " AIsize error" << endl;
			if( Flg == dat_error_end )	cout << "debug : " << hex << playerSide << "." << datName << " error at end" << endl;
		}
	#endif
	
	if( Flg ){
		if( Flg == dat_error_open ){
			return 1;
		}else{
			if( Flg == dat_error_inflate || Flg == dat_error_overflow || Flg == dat_error_size ){
				char datNameError[50];
				strcpy(datNameError, datName);
				strcat(datNameError, ".error\0");
				remove(datNameError);
				rename(datName, datNameError);
				return 1;
			}else{
				strcpy(backAIname,"error\0");
				return 0xF;
			}
		}
	}
	
	return 0;
}



int boosterDatClass::CloseAI(){		//AI保存　まずテンプファイルに保存して、元のを消してからリネーム。
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".CloseAI() " << endl;
	#endif
	#if debug_mode_noAI
		return 0;
	#endif
	
	if(strcmp(baseAIname,"error")==0 || strcmp(baseAIname,"init")==0 || strcmp(baseAIname,"second")==0){return 1;}
	
	
	char datName[50];
	char datNameTemp[50];
	
	strcpy(datName,"dat\\\0");
	strcat(datName, baseAIname);
	CreateDirectory(datName, NULL);
	
	strcat(datName, "\\\0");
	strcat(datName, baseAIname);
	strcpy(datNameTemp,datName);
	strcat(datName, ".dat\0");
	strcat(datNameTemp, ".tmp\0");
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Close" << endl;
	#endif
	
	//start : 0
	//end   : AIsizeArray[0]
	
	Flg = CloseDat(datNameTemp, AI, AIsizeArray[0]);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Close failed." << endl;
		#endif
		return 1;
	}
	
	Flg = remove(datName);				//AIDat.datを置き換える。	//そもそもファイルが無かったときエラー
	Flg = rename(datNameTemp, datName);
	if(Flg){
		#if debug_mode_file
			cout << "debug : " << hex << playerSide << "." << datName << " Replace failed." << endl;
		#endif
		return 1;
	}
	return 0;
}


int boosterDatClass::OpenAI(char AIName[16]){		//AI開く
	#if debug_mode
		cout << "debug : " << hex << playerSide << ".OpenAI() " << endl;
	#endif
	
	strcpy(baseAIname,AIName);
	
	#if debug_mode_noAI
		return 0;
	#endif
	
	char datName[50];
	strcpy(datName,"dat\\\0");
	strcat(datName, AIName);
	strcat(datName,"\\\0");
	strcat(datName, AIName);
	strcat(datName, ".dat\0");
	
	
	#if debug_mode_file
		cout << "debug : " << hex << playerSide << "." << datName << " Open" << endl;
	#endif
	
	Flg = OpenDat(datName, AI, AIsizeArray[0]);
	
	#if debug_mode_file
		if( Flg ){
			if( Flg == dat_error_open )	cout << "debug : " << hex << playerSide << "." << datName << " Open failed." << endl;
			if( Flg == dat_error_init )	cout << "debug : " << hex << playerSide << "." << datName << " error at init" << endl;
			if( Flg == dat_error_inflate )	cout << "debug : " << hex << playerSide << "." << datName << " error at inflate" << endl;
			if( Flg == dat_error_overflow )	cout << "debug : " << hex << playerSide << "." << datName << " overflow" << endl;
			if( Flg == dat_error_size )	cout << "debug : " << hex << playerSide << "." << datName << " AIsize error" << endl;
			if( Flg == dat_error_end )	cout << "debug : " << hex << playerSide << "." << datName << " error at end" << endl;
		}
	#endif
	
	if( Flg ){
		if( Flg == dat_error_open ){
			return 1;
		}else{
			if( Flg == dat_error_inflate || Flg == dat_error_overflow || Flg == dat_error_size ){
				char datNameError[50];
				strcpy(datNameError, datName);
				strcat(datNameError, ".error\0");
				remove(datNameError);
				rename(datName, datNameError);
				return 1;
			}else{
				strcpy(backAIname,"error\0");
				return 0xF;
			}
		}
	}
	
	return 0;
}
