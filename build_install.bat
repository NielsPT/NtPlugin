@echo off
setlocal enabledelayedexpansion

:: Configuration
set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
set "PLUGINS_DIR=%SCRIPT_DIR%\plugins"
set "BUILD_DIR=%SCRIPT_DIR%\build"
set "ARTIFACTS_DIR=%SCRIPT_DIR%\artifacts"
set "JUCE_WRAPPER_DIR=%SCRIPT_DIR%\JuceWrapper"
set "ID_FILE=%ARTIFACTS_DIR%\plugin_ids.txt"
set "VST3_INSTALL_DIR=%LOCALAPPDATA%\Programs\Common\VST3"

:: Create directories if they don't exist
if not exist "%ARTIFACTS_DIR%" mkdir "%ARTIFACTS_DIR%"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: Load existing plugin IDs into an array
set "ID_COUNT=0"
if exist "%ID_FILE%" (
    for /f "tokens=1,2 delims=: " %%a in ('type "%ID_FILE%"') do (
        set "PLUGIN_NAME_!ID_COUNT!=%%a"
        set "PLUGIN_ID_!ID_COUNT!=%%b"
        set /a "ID_COUNT+=1"
    )
)

:: Process each plugin
for /r "%PLUGINS_DIR%" %%f in (*.h) do (
    set "plugin_name=%%~nf"
    set "plugin_id="
    set "found=0"

    :: Check if we have a stored ID for this plugin
    for /l %%i in (0,1,%ID_COUNT%) do (
        if "!PLUGIN_NAME_%%i!"=="!plugin_name!" (
            set "plugin_id=!PLUGIN_ID_%%i!"
            set "found=1"
        )
    )

    echo Processing plugin: !plugin_name! with ID: !plugin_id!

    :: Run cmake
    if "!plugin_id!"=="" (
        cmake -B "%BUILD_DIR%" -S "%JUCE_WRAPPER_DIR%" -DNTFX_PLUGIN=!plugin_name! 2> cmake_output.txt
    ) else (
        cmake -B "%BUILD_DIR%" -S "%JUCE_WRAPPER_DIR%" -DNTFX_PLUGIN=!plugin_name! -DNTFX_ID=!plugin_id! 2> cmake_output.txt
    )

    :: Extract new plugin ID if not previously stored
    if "!found!"=="0" (
        for /f "tokens=5 delims= " %%i in ('findstr "Generated new plugin id:" cmake_output.txt') do (
            set "plugin_id=%%i"
            echo Found new plugin ID: !plugin_id!
            echo !plugin_name!: !plugin_id!>>"%ID_FILE%"
            set "PLUGIN_NAME_!ID_COUNT!=!plugin_name!"
            set "PLUGIN_ID_!ID_COUNT!=!plugin_id!"
            set /a "ID_COUNT+=1"
        )
    )

    :: Build the project
    cmake --build "%BUILD_DIR%" --config Release

    :: Copy artifacts
    set "source=%BUILD_DIR%\!plugin_name!_artefacts"
    set "dest=%ARTIFACTS_DIR%\!plugin_name!_artefacts"

    if exist "!source!" (
        echo Copying artifacts from !source! to !dest!
        xcopy "!source!" "!dest!" /E /I /Y /Q
    ) else (
        echo Warning: Could not find artifacts directory for !plugin_name!
    )
)

echo All plugins processed.
echo Artifacts are in %ARTIFACTS_DIR%
echo Plugin IDs are stored in %ID_FILE%

:: Installation prompt
set /p install=Do you want to install the VST3 plugins? (y/n):
if /i "%install%"=="y" (
    if not exist "%VST3_INSTALL_DIR%" mkdir "%VST3_INSTALL_DIR%"

    for /r "%ARTIFACTS_DIR%" %%d in (*_artefacts\VST3) do (
        set "plugin_name=%%~nd"
        set "plugin_name=!plugin_name:~0,-8!"
        set "vst3_file=%%d\!plugin_name!.vst3"

        if exist "!vst3_file!" (
            echo Installing !plugin_name!.vst3 to %VST3_INSTALL_DIR%
            copy "!vst3_file!" "%VST3_INSTALL_DIR%" /Y
        ) else (
            echo Warning: Could not find !plugin_name!.vst3
        )
    )
    echo Installation complete.
)
endlocal


@REM @echo off
@REM setlocal enabledelayedexpansion

@REM :: Configuration
@REM set "SCRIPT_DIR=%~dp0"
@REM set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
@REM set "PLUGINS_DIR=%SCRIPT_DIR%\plugins"
@REM set "BUILD_DIR=%SCRIPT_DIR%\build"
@REM set "ARTIFACTS_DIR=%SCRIPT_DIR%\artifacts"
@REM set "JUCE_WRAPPER_DIR=%SCRIPT_DIR%\JuceWrapper"
@REM set "ID_FILE=%ARTIFACTS_DIR%\plugin_ids.txt"
@REM set "VST3_INSTALL_DIR=%ARTIFACTS_DIR%\VST3"

@REM :: Create directories if they don't exist
@REM if not exist "%ARTIFACTS_DIR%" mkdir "%ARTIFACTS_DIR%"
@REM if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

@REM :: Load existing plugin IDs
@REM set "ID_MAP="
@REM if exist "%ID_FILE%" (
@REM     for /f "tokens=1,2 delims=: " %%a in ('type "%ID_FILE%"') do (
@REM         set "ID_MAP=!ID_MAP!%%a=%%b "
@REM     )
@REM )

@REM :: Process each plugin
@REM for /r "%PLUGINS_DIR%" %%f in (*.h) do (
@REM     set "plugin_name=%%~nf"
@REM     set "plugin_id="

@REM     :: Check if we have a stored ID for this plugin
@REM     for %%i in (!ID_MAP!) do (
@REM         if "%%i"=="!plugin_name!" (
@REM             set "plugin_id=%%j"
@REM         )
@REM     )

@REM     echo Processing plugin: !plugin_name! with ID: !plugin_id!

@REM     :: Run cmake
@REM     if "!plugin_id!"=="" (
@REM         cmake -B "%BUILD_DIR%" -S "%JUCE_WRAPPER_DIR%" -DNTFX_PLUGIN=!plugin_name! 2> cmake_output.txt
@REM     ) else (
@REM         cmake -B "%BUILD_DIR%" -S "%JUCE_WRAPPER_DIR%" -DNTFX_PLUGIN=!plugin_name! -DNTFX_ID=!plugin_id! 2> cmake_output.txt
@REM     )

@REM     :: Extract new plugin ID if not previously stored
@REM     if "!plugin_id!"=="" (
@REM         for /f "tokens=5 delims= " %%i in ('findstr "Generated new plugin id:" cmake_output.txt') do (
@REM             set "plugin_id=%%i"
@REM             echo Found new plugin ID: !plugin_id!
@REM             echo !plugin_name!: !plugin_id!>>"%ID_FILE%"
@REM         )
@REM     )

@REM     :: Build the project
@REM     cmake --build "%BUILD_DIR%" --config Release

@REM     :: Copy artifacts
@REM     set "source=%BUILD_DIR%\!plugin_name!_artefacts"
@REM     set "dest=%ARTIFACTS_DIR%\!plugin_name!_artefacts"

@REM     if exist "!source!" (
@REM         echo Copying artifacts from !source! to !dest!
@REM         xcopy "!source!" "!dest!" /E /I /Y /Q
@REM     ) else (
@REM         echo Warning: Could not find artifacts directory for !plugin_name!
@REM     )
@REM )

@REM echo All plugins processed.
@REM echo Artifacts are in %ARTIFACTS_DIR%
@REM echo Plugin IDs are stored in %ID_FILE%


@REM if not exist "%VST3_INSTALL_DIR%" mkdir "%VST3_INSTALL_DIR%"

@REM for /r "%ARTIFACTS_DIR%" %%d in (*_artefacts\release\VST3) do (
@REM     set "plugin_name=%%~nd"
@REM     set "plugin_name=!plugin_name:~0,-8!"
@REM     set "vst3_file=%%d\!plugin_name!.vst3"

@REM     if exist "!vst3_file!" (
@REM         echo Installing !plugin_name!.vst3 to %VST3_INSTALL_DIR%
@REM         copy "!vst3_file!" "%VST3_INSTALL_DIR%" /Y
@REM     ) else (
@REM         echo Warning: Could not find !plugin_name!.vst3
@REM     )
@REM )
@REM echo Installation complete.

@REM endlocal

@REM @echo off
@REM :: Copyright (C) 2026 Niels Thøgersen, NTlyd
@REM ::
@REM :: This program is free software: you can redistribute it and/or modify it under
@REM :: the terms of the GNU Affero General Public License as published by the Free
@REM :: Software Foundation, either version 3 of the License, or (at your option) any
@REM :: later version.
@REM ::
@REM :: This program is distributed in this hope that it will be useful, but WITHOUT
@REM :: ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
@REM :: FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
@REM :: details.
@REM ::
@REM :: You should have received a copy of the GNU Affero General Public License
@REM :: along with this program. If not, see <https://www.gnu.org/licenses/>.
@REM ::
@REM :: You are free to download, build and use this code for commercial
@REM :: purposes. Just don't resell it or a build of it, modified or otherwise.
@REM ::
@REM :: @brief This script will build all plugins in the 'plugins' directory
@REM :: in release mode and install them on the system if possible.
@REM ::

@REM :: Configuration
@REM set PLUGINS_DIR=plugins
@REM set BUILD_DIR=build
@REM set ARTIFACTS_DIR=artifacts
@REM set JUCE_WRAPPER_DIR=JuceWrapper
@REM set ID_FILE=%ARTIFACTS_DIR%\plugin_ids.txt
@REM set TEST_SCRIPT_DIR=testWrapper
@REM set TEST_DIR=test
@REM set VST3_INSTALL_DIR=%ARTIFACTS_DIR%\VST3

@REM :: Create artifacts directory if it doesn't exist
@REM if not exist "%ARTIFACTS_DIR%" mkdir "%ARTIFACTS_DIR%"
@REM if not exist "%VST3_INSTALL_DIR%" mkdir "%VST3_INSTALL_DIR%"

@REM :: Initialize ID map from existing file if it exists
@REM setlocal enabledelayedexpansion
@REM if exist "%ID_FILE%" (
@REM     for /f "tokens=1,2 delims=: " %%a in (%ID_FILE%) do (
@REM         set "plugin_ids[%%a]=%%b"
@REM     )
@REM )

@REM :: Process all plugin files
@REM for /r "%PLUGINS_DIR%" %%f in (*.h) do (
@REM     :: Extract plugin name (filename without .h)
@REM     set "plugin_name=%%~nf"
@REM     echo Processing plugin: !plugin_name!

@REM     :: Check if we have a stored ID for this plugin
@REM     set "plugin_id="
@REM     if defined plugin_ids[%plugin_name%] (
@REM         set "plugin_id=!plugin_ids[%plugin_name%]!"
@REM         echo Reusing existing plugin ID: !plugin_id!
@REM     )

@REM     :: Step 1: Run cmake with the plugin name and captured output
@REM     if not "!plugin_id!"=="" (
@REM         echo Running cmake for !plugin_name! using existing ID !plugin_id! ...
@REM         cmake -B "%BUILD_DIR%" -S "%JUCE_WRAPPER_DIR%" -DNTFX_PLUGIN=!plugin_name! -DNTFX_ID=!plugin_id! -A x64 --fresh > cmake_output.txt
@REM     ) else (
@REM         echo Running cmake for !plugin_name! without ID...
@REM         cmake -B "%BUILD_DIR%" -S "%JUCE_WRAPPER_DIR%" -DNTFX_PLUGIN=!plugin_name! -A x64 --fresh > cmake_output.txt
@REM     )
@REM     set /p cmake_output=<cmake_output.txt

@REM     :: Extract the plugin ID from cmake output if we didn't reuse one
@REM     if "!plugin_id!"=="" (
@REM         for /f "tokens=6 delims= " %%i in ('findstr "Generated new plugin id:" cmake_output.txt') do (
@REM             set "plugin_id=%%i"
@REM             echo Found new plugin ID: !plugin_id!
@REM         )
@REM         if "!plugin_id!"=="" (
@REM             echo Warning: Could not find plugin ID for !plugin_name!
@REM         )
@REM     )

@REM     :: Update the ID file if we have a new ID
@REM     if defined plugin_id if not defined plugin_ids[%plugin_name%] (
@REM         echo !plugin_name!: !plugin_id!>>"%ID_FILE%"
@REM         set "plugin_ids[%plugin_name%]=!plugin_id!"
@REM     )

@REM     :: Step 2: Build the project
@REM     echo Building !plugin_name!...
@REM     cmake --build "%BUILD_DIR%" --config Release

@REM     :: Step 3: Find the plugin-specific artifacts directory
@REM     set "plugin_artefacts_dir=%BUILD_DIR%\!plugin_name!_artefacts"
@REM     echo Artefact dir: !plugin_artefacts_dir!

@REM     if exist "!plugin_artefacts_dir!" (
@REM         :: Step 4: Copy the entire plugin artifacts directory to the final location
@REM         echo Copying artifacts for !plugin_name! from %plugin_artefacts_dir% to %ARTIFACTS_DIR%...
@REM         xcopy "%plugin_artefacts_dir%" "%ARTIFACTS_DIR%" /E /I /Y
@REM         xcopy "%plugin_artefacts_dir%\VST3\Release\VST3\%plugin_name%.vst3" "%VST3_INSTALL_DIR%\%plugin_name%.vst3" /E /I /Y

@REM         echo Finished processing !plugin_name!
@REM         echo ---------------------------------
@REM     ) else (
@REM         echo Warning: Could not find artifacts directory for '!plugin_name!' at path '!plugin_artefacts_dir!'.
@REM     )
@REM )

@REM rm cmake_output.txt
@REM echo All plugins processed.
@REM echo Artifacts are in %ARTIFACTS_DIR%
@REM echo Plugin IDs are stored in %ID_FILE%
@REM echo Add '%~f1%VST3_INSTALL_DIR%' to you DAW search path or copy the content to your favorite VST3 pligin location in order to use the plugins.

@REM endlocal