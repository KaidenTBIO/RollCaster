CPP = bcc32
CPPFLAGS = -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -O2 -WM

OBJ1	= class.obj init.obj develop.obj file.obj roop.obj HL.obj
OBJ2	= ai_LNAI.obj ai_SNAI.obj ai_ManageAI.obj ai_SpellAI.obj ai_SWAI.obj ai_RecoverAI.obj ai_BackAI.obj script.obj


all: $(OBJ1) $(OBJ2)

.cpp.obj:
	$(CPP) $(CPPFLAGS) -c $<

# cleanup
clean:
	-del *.obj
	-del *.exe
	-del *.tds

class.obj: conf.h boosterDatClass.h scriptClass.h
init.obj: conf.h boosterDatClass.h scriptClass.h
develop.obj: conf.h boosterDatClass.h scriptClass.h
file.obj: conf.h boosterDatClass.h scriptClass.h
roop.obj: conf.h boosterDatClass.h scriptClass.h
HL.obj: HL.h
ai_LNAI.obj: conf.h boosterDatClass.h HL.h scriptClass.h
ai_SNAI.obj: conf.h boosterDatClass.h HL.h scriptClass.h
ai_ManageAI.obj: conf.h boosterDatClass.h scriptClass.h
ai_SpellAI.obj: conf.h boosterDatClass.h scriptClass.h
ai_SWAI.obj: conf.h boosterDatClass.h HL.h scriptClass.h
ai_RecoverAI.obj: conf.h boosterDatClass.h scriptClass.h
ai_BackAI.obj: conf.h boosterDatClass.h scriptClass.h
script.obj: conf.h boosterDatClass.h scriptClass.h scriptClass.h


