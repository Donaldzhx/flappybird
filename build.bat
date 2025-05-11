@echo off
setlocal

set ANDROID_SDK_ROOT=C:\Users\zx884\AppData\Local\Android\Sdk
set ANDROID_NDK_ROOT=C:\Users\zx884\AppData\Local\Android\Sdk\ndk\29.0.13113456
set PATH=%ANDROID_SDK_ROOT%\tools;%ANDROID_SDK_ROOT%\platform-tools;%PATH%
set ADB=C:\Users\zx884\AppData\Local\Android\platform-tools\adb.exe

set APKNAME=FlappyBird
set ANDROIDVERSION=30
set ANDROIDTARGET=30

set KEYSTORE_PASSWORD=12345678
echo Cleaning previous builds ...
rmdir /s /q app\builds

echo Creating build directories ...
mkdir app\build\intermediates\ndk
mkdir app\build\outputs\apk

echo Building native code ...
cd app\src\main
call %ANDROID_NDK_ROOT%\ndk-build
if %errorlevel% neq 0 (
    echo Error building native code!
    echo Error code: %errorlevel%
    exit /b %errorlevel%
)
cd ..\..\..

echo Creating empty APK...
%ANDROID_SDK_ROOT%\build-tools\35.0.1\aapt package -f -M app\src\main\AndroidManifest.xml -S app\src\main\res -A app\src\main\assets -I %ANDROID_SDK_ROOT%\platforms\android-%ANDROIDTARGET%\android.jar -F app\build\outputs\apk\unaligned.apk
if %errorlevel% neq 0 (
    echo Error creating empty APK!
    echo Error code: %errorlevel%
    exit /b %errorlevel%
)

mkdir lib

:: Copy files from libs to a temporary folder
xcopy "app\src\main\libs\*" "lib\" /E /I /Y

:: Add the contents of the temporary folder to the archive in the lib folder
start /min /wait WinRAR A -r app\build\outputs\apk\unaligned.apk "lib\*"

echo Aligning APK ...
call %ANDROID_SDK_ROOT%\build-tools\35.0.1\apksigner sign --ks mykeystore.jks --ks-pass:%KEYSTORE_PASSWORD% --out app\build\outputs\apk\%APKNAME%-signed.apk app\build\outputs\apk\%APKNAME%.apk
if %errorlevel% neq 0 (
    echo Error signing APK!
    echo Error code: %errorlevel%
    exit /b %errorlevel%
)

:: Delete temporary folder
rmdir /s /q lib

echo Deleting unnecessary files ...
del /q "app\build\outputs\apk\%APKNAME%.apk"
del /q "app\build\outputs\apk\%APKNAME%-signed.apk.idsig"
del /q "app\build\outputs\apk\unaligned.apk"

echo APK successfully created: app\build\outputs\apk\%APKNAME%-signed.apk

:: Debug moment
echo Clear logcat
%ADB% logcat -c
echo installing APK
%ADB% install app\build\outputs\apk\%APKNAME%-signed.apk
start /min timeout.exe 1
echo Starting logging
start %ADB% logcat -s flappy

exit

endlocal

