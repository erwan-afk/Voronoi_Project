setlocal
set SDL_LINK="https://github.com/libsdl-org/SDL/releases/download/release-2.24.2/SDL2-devel-2.24.2-mingw.zip"
bitsadmin /transfer mydownloadjob /download %SDL_LINK% "%TMP%\SDL2.zip"

call :UnZipFile "%TMP%\SDL2" "%TMP%\SDL2.zip"

:UnZipFile <ExtractTo> <newzipfile>
set vbs="%temp%\_.vbs"
if exist %vbs% del /f /q %vbs%
>%vbs%  echo Set fso = CreateObject("Scripting.FileSystemObject")
>>%vbs% echo If NOT fso.FolderExists(%1) Then
>>%vbs% echo fso.CreateFolder(%1)
>>%vbs% echo End If
>>%vbs% echo set objShell = CreateObject("Shell.Application")
>>%vbs% echo set FilesInZip=objShell.NameSpace(%2).items()
>>%vbs% echo objShell.NameSpace(%1).CopyHere FilesInZip, 16
>>%vbs% echo Set fso = Nothing
>>%vbs% echo Set objShell = Nothing
cscript //nologo %vbs%
del %vbs%

for /f "delims=" %%i in ('where g++') do set GCC=%%i

mkdir SDL2
XCOPY /C /s /Y "%TMP%\SDL2\SDL2-2.24.2\" ".\SDL2\"

