:: @echo off
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
set VST3_INSTALL_DIR=%ARTIFACTS_DIR%\VST3

:: Create artifacts directory if it doesn't exist
if not exist "%ARTIFACTS_DIR%" mkdir "%ARTIFACTS_DIR%"
if not exist "%VST3_INSTALL_DIR%" mkdir "%VST3_INSTALL_DIR%"

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
    echo Processing plugin: !plugin_name!

    :: Check if we have a stored ID for this plugin
    set "plugin_id="
    if defined plugin_ids[%plugin_name%] (
        set "plugin_id=!plugin_ids[%plugin_name%]!"
        echo Reusing existing plugin ID: !plugin_id!
    )

    :: Step 1: Run cmake with the plugin name and captured output
    if not "!plugin_id!"=="" (
        echo Running cmake for !plugin_name! using existing ID !plugin_id! ...
        cmake -B "%BUILD_DIR%" -S "%JUCE_WRAPPER_DIR%" -DNTFX_PLUGIN=!plugin_name! -DNTFX_ID=!plugin_id! -A x64 --fresh > cmake_output.txt
    ) else (
        echo Running cmake for !plugin_name! without ID...
        cmake -B "%BUILD_DIR%" -S "%JUCE_WRAPPER_DIR%" -DNTFX_PLUGIN=!plugin_name! -A x64 --fresh > cmake_output.txt
    )
    set /p cmake_output=<cmake_output.txt

    :: Extract the plugin ID from cmake output if we didn't reuse one
    if "!plugin_id!"=="" (
        for /f "tokens=6 delims= " %%i in ('findstr "Generated new plugin id:" cmake_output.txt') do (
            set "plugin_id=%%i"
            echo Found new plugin ID: !plugin_id!
        )
        if "!plugin_id!"=="" (
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
    set "plugin_artefacts_dir=%~f1%BUILD_DIR%\!plugin_name!_artefacts"
    echo Artefact dir: !plugin_artefacts_dir!
    pause

    if exist "!plugin_artefacts_dir!" (
        :: Step 4: Copy the entire plugin artifacts directory to the final location
        echo Copying artifacts for !plugin_name!...
        xcopy "%plugin_artefacts_dir%" "%ARTIFACTS_DIR%" /E /I /Y
        xcopy "%plugin_artefacts_dir%/VST3/Release/VST3/%plugin_name%.vst3" "%~f1%VST3_INSTALL_DIR%/%plugin_name%.vst3" /E /I /Y

        echo Finished processing !plugin_name!
        echo ---------------------------------
    ) else (
        echo Warning: Could not find artifacts directory for '!plugin_name!' at path '!plugin_artefacts_dir!'.
    )
)

rm cmake_output.txt
echo All plugins processed.
echo Artifacts are in %ARTIFACTS_DIR%
echo Plugin IDs are stored in %ID_FILE%
echo Add '%~f1%VST3_INSTALL_DIR%' to you DAW search path or copy the content to your favorite VST3 pligin location in order to use the plugins.

endlocal