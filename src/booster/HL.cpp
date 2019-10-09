#include "HL.h"

void FloatHL(BYTE* Target, int hs, int ls){
	if(Target){
		//上位4ビット
		if(hs > 0){
			hs = hs * 0x10;
			if(*Target + hs > 0xFF){
				*Target = *Target | 0xF0;
			}else{
				*Target = *Target + hs;
			}
		}
		if(hs < 0){
			hs = hs * 0x10;
			if(*Target + hs < 0x0F){
				*Target = *Target & 0x0F;
			}else{
				*Target = *Target + hs;
			}
		}
		
		//下位4ビット
		if(ls > 0){
			if((*Target & 0x0F) + ls > 0xF){
				*Target = *Target | 0x0F;
			}else{
				*Target = *Target + ls;
			}
		}
		if(ls < 0){
			if((*Target & 0x0F) + ls < 0x0){
				*Target = *Target & 0xF0;
			}else{
				*Target = *Target + ls;
			}
		}
	}
}

void FloatH(BYTE* Target, int hs){
	if(Target){
		//上位4ビット
		if(hs > 0){
			hs = hs * 0x10;
			if(*Target + hs > 0xFF){
				*Target = *Target | 0xF0;
			}else{
				*Target = *Target + hs;
			}
		}
		if(hs < 0){
			hs = hs * 0x10;
			if(*Target + hs < 0x0F){
				*Target = *Target & 0x0F;
			}else{
				*Target = *Target + hs;
			}
		}
	}
}

void FloatL(BYTE* Target, int ls){
	if(Target){
		//下位4ビット
		if(ls > 0){
			if((*Target & 0x0F) + ls > 0xF){
				*Target = *Target | 0x0F;
			}else{
				*Target = *Target + ls;
			}
		}
		if(ls < 0){
			if((*Target & 0x0F) + ls < 0x0){
				*Target = *Target & 0xF0;
			}else{
				*Target = *Target + ls;
			}
		}
	}
}


void SetH(BYTE* Target, BYTE hs){
	if(Target){
		if(hs > 0xF){
			hs = 0xF;
		}
		*Target = *Target & 0x0F;
		if(hs > 0xF){
			*Target = *Target | 0xF0;
		}else{
			*Target = *Target | (hs << 4);
		}
	}
}
void SetL(BYTE* Target, BYTE ls){
	if(Target){
		if(ls > 0xF){
			ls = 0xF;
		}
		*Target = *Target & 0xF0;
		if(ls > 0xF){
			*Target = *Target | 0x0F;
		}else{
			*Target = *Target | ls;
		}
	}
}

BYTE GetH(BYTE* Target){
	return (*Target & 0xF0) >> 4;
}

BYTE GetL(BYTE* Target){
	return *Target & 0x0F;
}
