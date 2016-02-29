@rem To be compitable with Win7 And Win10(64 bits),
@rem We write this cmd file in according to VsDevCmd.bat(VS2012 开发人员命令提示)

@svn update
@for /f %%i in ('svn info --show-item revision') do set version="%%i"
@echo #define ExeVersion %version% > Version.h