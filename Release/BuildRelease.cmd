@rem To be compitable with Win7 And Win10(64 bits),
@rem We write this cmd file in according to VsDevCmd.bat(VS2012 开发人员命令提示)

@call :GetVSCommonToolsDirHelper32 HKLM > nul 2>&1
@set "VS110COMNTOOLS=%VS110COMNTOOLS%Common7\Tools\"
@call "%VS110COMNTOOLS%VCVarsQueryRegistry.bat" 32bit No64bit
@set DevEnvDir=%VSINSTALLDIR%Common7\IDE
@set "PATH=%DevEnvDir%;%PATH%"

@if exist "log.txt" del /F /Q "log.txt"
devenv.exe "..\Gs9330.sln" /build Release /project VcProject /out log.txt
@type log.txt
@del /F /Q "log.txt"
devenv.exe "..\Gs9330.sln" /build Release /project VcUnitTestProject /out log.txt
@type log.txt
pause

del log.txt

@goto :End

:GetVSCommonToolsDirHelper32
@for /F "tokens=1,2*" %%i in ('reg query "%1\SOFTWARE\Microsoft\VisualStudio\SxS\VS7" /v "11.0"') DO (
	@if "%%i"=="11.0" (
		@SET "VS110COMNTOOLS=%%k"
	)
)

:End
