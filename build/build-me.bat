@setlocal
@set TMPPRJ=scidown
@set TMPLOG=bldlog-1.txt
@set TMPSRC=..
@set VCVERS=14
@set DOINST=0
@REM ############################################
@REM NOTE: SPECIAL INSTALL LOCATION
@REM Adjust to suit your environment
@REM ##########################################
@set TMPINST=F:\Projects\software.x64
@set TMPOPTS=-DCMAKE_INSTALL_PREFIX=%TMPINST%

@set GENERATOR=Visual Studio %VCVERS% Win64
@set VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio %VCVERS%.0
@set VC_BAT=%VS_PATH%\VC\vcvarsall.bat
@if NOT EXIST "%VS_PATH%" goto NOVS
@if NOT EXIST "%VC_BAT%" goto NOBAT
@set BUILD_BITS=%PROCESSOR_ARCHITECTURE%

@set TMPOPTS=%TMPOPTS% -G "%GENERATOR%"

@call chkmsvc %TMPPRJ%

@echo Begin %DATE% %TIME%, output to %TMPLOG%
@echo Begin %DATE% %TIME% > %TMPLOG%

@echo Setting environment - CALL "%VC_BAT%" %BUILD_BITS%
@echo Setting environment - CALL "%VC_BAT%" %BUILD_BITS% >> %TMPLOG%
@call "%VC_BAT%" %BUILD_BITS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto NOSETUP

@echo Doing: 'cmake %TMPSRC% %TMPOPTS%'
@echo Doing: 'cmake %TMPSRC% %TMPOPTS%' >> %TMPLOG%
@cmake %TMPSRC% %TMPOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

@echo Doing: 'cmake --build . --config debug'
@echo Doing: 'cmake --build . --config debug' >> %TMPLOG%
@cmake --build . --config debug >> %TMPLOG%
@if ERRORLEVEL 1 goto ERR2

@echo Doing: 'cmake --build . --config release'
@echo Doing: 'cmake --build . --config release' >> %TMPLOG%
@cmake --build . --config release >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3

@echo Appears a successful build
@echo.
@if "%DOINST%x" == "1x" goto DNCHK
@echo No install at this time... Set DOINST=1
@echo.
@goto END

:DNCHK
@echo.
@echo Note install location %TMPINST%
@echo *** CONTINUE with install? *** Only 'y' continues ***
@echo.
@ask
@if ERRORLEVEL 2 goto NOASK
@if ERRORLEVEL 1 goto DOINST
@echo.
@echo No install at this time...
@echo.
@goto END

:DOINST
cmake -P cmake_install.cmake
@REM echo Doing: 'cmake --build . --config release --target INSTALL'
@REM echo Doing: 'cmake --build . --config release --target INSTALL' >> %TMPLOG%
@REM cmake --build . --config release --target INSTALL >> %TMPLOG% 2>&1
@REM fa4 " -- " %TMPLOG%

@echo Done build and install of %TMPPRJ%...

@goto END

:NOASK
@echo.
@echo Can NOT find 'ask' utility in PATH
@echo See : https://gitorious.org/fgtools/gtools
@echo.
@goto END


:ERR1
@echo cmake config, generation error
@goto ISERR

:ERR2
@echo debug build error
@goto ISERR

:ERR3
@echo release build error
@goto ISERR

:NOVS
@echo Can not locate "%VS_PATH%"! *** FIX ME *** for your environment
@goto ISERR

:NOBAT
@echo Can not locate "%VC_BAT%"! *** FIX ME *** for your environment
@goto ISERR

:NOSETUP
@echo MSVC setup FAILED!
@goto ISERR

:ISERR
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
