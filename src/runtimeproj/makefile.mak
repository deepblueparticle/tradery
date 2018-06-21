!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP="$(TOOLSPATH)\cl.exe"


#OUTDIR=.\Release
#INTDIR=.\Release
# Begin Custom Macros
#OutDir=.\Release
# End Custom Macros

ALL :   "$(OUTDIR)\runtimeproj.dll"

CLEAN :
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\vc70.idb"
	-@erase "$(INTDIR)\runtimeproj.obj"
	-@erase "$(OUTDIR)\runtimeproj.dll"

"$(OUTDIR)" :
	-@ if NOT EXIST "$(OUTDIR)" mkdir "$(OUTDIR)"
"$(INTDIR)" :
	-@ if NOT EXIST "$(INTDIR)" mkdir "$(INTDIR)"

CL_COMMON_ARGS=/nologo $(INCLUDEPATH) /EHa /W3 /GR /c /D "UNICODE" \
	/D "_UNICODE" /D "WIN32" /D "_USRDLL" /D "_WINDLL" /D "PLUGIN_EXPORTS" /D _CRT_SECURE_NO_WARNINGS

!IF "$(CFG)" == "Debug"
CPP_PROJ=$(CL_COMMON_ARGS) /Od /RTC1 /MDd /TP /Z7 /D "_DEBUG" /Fp"$(INTDIR)\runtimeprojd" 
!ELSE
CPP_PROJ=$(CL_COMMON_ARGS) /G7 /GS /MD /GX /D "NDEBUG" /Fp"$(INTDIR)\runtimeprojr" 
!ENDIF

LINK32="$(TOOLSPATH)\link.exe"
MANIFEST_TOOL="$(TOOLSPATH)\mt.exe"
MANIFEST_PATH="$(OUTDIR)\runtimeproj.dll.manifest"

LINK_COMMON_ARGS=/DLL kernel32.lib user32.lib misc.lib miscwin.lib core.lib plugin.lib /nologo /subsystem:WINDOWS \
	/MACHINE:X86 /INCREMENTAL:no /pdb:"$(OUTDIR)\runtimeproj.pdb" $(LIBPATH)

!IF "$(CFG)" == "Debug"
LINK32_FLAGS=$(LINK_COMMON_ARGS) /DEBUG /out:"$(OUTDIR)\runtimeproj.dll" 
!ELSE
LINK32_FLAGS=$(LINK_COMMON_ARGS) /OPT:REF /OPT:ICF /out:"$(OUTDIR)\runtimeproj.dll"
#		 /MANIFEST /MANIFESTFILE:$(MANIFEST_PATH) /MANIFESTUAC:"level='asInvoker' uiAccess='false'"
!ENDIF

LINK32_OBJS= \
	"$(INTDIR)\stdafx.obj" \
	"$(OUTDIR)\runtimeproj.obj"

"$(OUTDIR)\runtimeproj.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)
	-@erase "$(OUTDIR)\runtimeproj.obj"
#	$(MANIFEST_TOOL) $(MANIFEST_FLAGS) /nologo /outputresource:"$(OUTDIR)\runtimeproj.dll;#1"

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("runtimeproj.dep")
!INCLUDE "runtimeproj.dep"
!ELSE 
!MESSAGE Warning: cannot find "runtimeproj.dep"
!ENDIF 
!ENDIF 


SOURCE="$(PROJDIR)\stdafx.cpp"

"$(INTDIR)\stdafx.obj" : $(SOURCE) "$(INTDIR)" 
	$(CPP) $(CPP_PROJ) /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" $(SOURCE)

SOURCE="$(PROJDIR)\runtimeproj.cpp"

!IFDEF BUILDERRORSFILE
ERRFILE =  > $(BUILDERRORSFILE)
!ELSE
ERRFILE = ""
!ENDIF

!MESSAGE "this is the errors file: "
!MESSAGE $(ERRFILE)
!MESSAGE " THIS WAS THE ERRORS FILE "

BUILDRUNTIMEPROJ_OBJ=$(CPP) $(CPP_PROJ) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE) $(ERRFILE)

"$(OUTDIR)\runtimeproj.obj" : $(SOURCE) "$(OUTDIR)" "$(OUTDIR)\defines.h"
	$(BUILDRUNTIMEPROJ_OBJ)

