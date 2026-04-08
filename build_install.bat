@echo off
:: Copyright (C) 2026 Niels Thøgersen, NTlyd
::
:: This program is free software: you can redistribute it and/or modify it under
:: the terms of the GNU Affero General Public License as published by the Free
:: Software Foundation, either version 3 of the License, or (at your option) any
:: later version.
::
:: This program is distributed in this hope that it will be useful, but WITHOUT
:: ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
:: FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
:: details.
::
:: You should have received a copy of the GNU Affero General Public License
:: along with this program. If not, see <https://www.gnu.org/licenses/>.
::
:: You are free to download, build and use this code for commercial
:: purposes. Just don't resell it or a build of it, modified or otherwise.
::
:: @brief This script will build all plugins in the 'plugins' directory
:: in release mode and install them on the system if possible.
::

:: Configuration
set PLUGINS_DIR=plugins
set BUILD_DIR=build
set ARTIFACTS_DIR=artifacts
set JUCE_WRAPPER_DIR=JuceWrapper
set ID_FILE=%ARTIFACTS_DIR%\plugin_ids.txt
set TEST_SCRIPT_DIR=testWrapper
set TEST_DIR=test
set VST3_INSTALL_DIR=%LOCALAPPDATA%\Programs\Common\VST3

@REM :: Check if running as administrator
@REM net session >nul 2>&1
@REM if %errorLevel% neq 0 (
@REM     echo This script needs to be run as Administrator to install VST3 plugins.
@REM     echo Please right-click and select "Run as administrator".
@REM     pause
@REM     exit /b
@REM )

:: Create artifacts directory if it doesn't exist
if not exist "%ARTIFACTS_DIR%" mkdir "%ARTIFACTS_DIR%"

:: Initialize ID map from existing file if it exists
setlocal enabledelayedexpansion
if exist "%ID_FILE%" (
    for /f "tokens=1,2 delims=: " %%a in (%ID_FILE%) do (
        set "plugin_ids[%%a]=%%b"
    )
)

:: Process all plugin files
for /r "%PLUGINS_DIR%" %%f in (*.h) do (
    :: Extract plugin name (filename without .h)
    set "plugin_name=%%~nf"
    echo Processing plugin: !plugin_name!, id: !plugin_ids[%plugin_name%]!

    :: Check if we have a stored ID for this plugin
    set "plugin_id="
    if defined plugin_ids[%plugin_name%] (
        set "plugin_id=!plugin_ids[%plugin_name%]!"
        echo Reusing existing plugin ID: !plugin_id!
    )

    :: Step 1: Run cmake with the plugin name and captured output
    echo Running cmake for !plugin_name!...
    cmake -B "%BUILD_DIR%" -S "%JUCE_WRAPPER_DIR%" -DNTFX_PLUGIN=!plugin_name! %plugin_id:-DNTFX_ID=% 2> cmake_output.txt
    set /p cmake_output=<cmake_output.txt

    :: Extract the plugin ID from cmake output if we didn't reuse one
    if not defined plugin_id (
        for /f "tokens=5 delims= " %%i in ('findstr "Generated new plugin id:" cmake_output.txt') do (
            set "plugin_id=%%i"
            echo Found new plugin ID: !plugin_id!
        )
        if not defined plugin_id (
            echo Warning: Could not find plugin ID for !plugin_name!
        )
    )

    :: Update the ID file if we have a new ID
    if defined plugin_id if not defined plugin_ids[%plugin_name%] (
        echo !plugin_name!: !plugin_id!>>"%ID_FILE%"
        set "plugin_ids[%plugin_name%]=!plugin_id!"
    )

    :: Step 2: Build the project
    echo Building !plugin_name!...
    cmake --build "%BUILD_DIR%" --config Release

    :: Step 3: Find the plugin-specific artifacts directory
    set "plugin_artefacts_dir=%BUILD_DIR%\!plugin_name!_artefacts"

    if exist "%plugin_artefacts_dir%" (
        :: Step 4: Copy the entire plugin artifacts directory to the final location
        echo Copying artifacts for !plugin_name!...
        xcopy "%plugin_artefacts_dir%" "%ARTIFACTS_DIR%" /E /I /Y

        echo Finished processing !plugin_name!
        echo ---------------------------------
    ) else (
        echo Warning: Could not find artifacts directory for !plugin_name!
    )
)

echo All plugins processed.
echo Artifacts are in %ARTIFACTS_DIR%
echo Plugin IDs are stored in %ID_FILE%

:: Ask for permission to install artifacts
set /p install=Do you want to install the VST3 plugins? (y/n):
if /i "%install%"=="y" (
    echo Installing VST3 plugins to %VST3_INSTALL_DIR%...
    if not exist "%VST3_INSTALL_DIR%" mkdir "%VST3_INSTALL_DIR%"

    :: Install each plugin
    for /r "%ARTIFACTS_DIR%" %%d in (*_artefacts\VST3) do (
        set "plugin_name=%%~nd"
        set "plugin_name=!plugin_name:~0,-8!"  :: Remove "_artefacts" from name

        if exist "%%d\!plugin_name!.vst3" (
            echo Installing !plugin_name!.vst3...
            xcopy "%%d\!plugin_name!.vst3" "%VST3_INSTALL_DIR%" /Y
        ) else (
            echo Warning: Could not find !plugin_name!.vst3 in %%d
        )
    )

    echo VST3 plugins installed successfully to %VST3_INSTALL_DIR%
) else (
    echo Installation cancelled.
)

endlocal