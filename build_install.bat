@echo off
setlocal enabledelayedexpansion

:: =========================================================================
:: Configuration
:: =========================================================================
SET "PLUGINS_DIR=plugins"                   :: Directory containing plugin .h files
SET "BUILD_DIR=build"                       :: Build directory
SET "ARTIFACTS_DIR=artifacts"               :: Directory to store final artifacts
SET "JUCE_WRAPPER_DIR=JuceWrapper"          :: Path to JuceWrapper directory
SET "ID_FILE=artifacts\plugin_ids.txt"       :: File to store plugin IDs
SET "TEST_SCRIPT_DIR=testWrapper"
SET "TEST_DIR=test"

echo Copyright (C) 2026 Niels Thøgersen, NTlyd
echo.
echo This script will build all plugins in the 'plugins' directory
echo in release mode and install them on the system if possible.
echo.

:: =========================================================================
:: Setup Python Virtual Environment and Run Tests
:: =========================================================================
SET "VENV_DIR=.venv"

IF NOT EXIST "%VENV_DIR%" (
    echo [SETUP] Creating Python virtual environment...
    python -m venv "%VENV_DIR%"
)

:: Activate virtual environment using full path (required for reliable scripting)
SET "PYTHON_EXECUTABLE=%VENV_DIR%\Scripts\python.exe"

IF NOT EXIST "%PYTHON_EXECUTABLE%" (
    echo ERROR: Could not find the Python executable at %PYTHON_EXECUTABLE%
    echo Please ensure Python and the venv creation was successful.
    goto :end
)

:: Activate the environment (best practice for invoking scripts)
CALL "%VENV_DIR%\Scripts\activate.bat"

:: Install requirements
echo [SETUP] Installing Python dependencies from requirements.txt...
pip install -r %TEST_SCRIPT_DIR%\requirements.txt

:: Run tests
echo [SETUP] Running unit tests...
"%PYTHON_EXECUTABLE%" %TEST_SCRIPT_DIR%\test.py run all
IF ERRORLEVEL 1 (
    echo.
    echo ERROR: Unit tests failed. Aborting build process.
    goto :end
)
echo [SETUP] Tests completed successfully.

:: =========================================================================
:: Main Build Logic
:: =========================================================================

:: Create artifacts directory if it doesn't exist
IF NOT EXIST "%ARTIFACTS_DIR%" (
    mkdir "%ARTIFACTS_DIR%"
    echo Created artifacts directory: %ARTIFACTS_DIR%
)

:: Initialize ID map (plugin_ids) from existing file
SET "plugin_ids="
IF EXIST "%ID_FILE%" (
    echo [CONFIG] Loading existing plugin IDs from %ID_FILE%
    FOR /F "tokens=1* delims=:" %%A IN ('type "%ID_FILE%"') DO (
        SET "plugin_name=%%A"
        SET "plugin_id=%%B"
        :: Use a global/persistent method to store IDs, or pass them to a function
        :: For simplicity here, we just rely on the file existing.
        :: If we needed internal tracking of all IDs, a temporary file structure would be needed.
        echo Found ID entry: !plugin_name! = !plugin_id!
    )
)

echo.
echo =========================================================================
echo STARTING PLUGIN BUILD PROCESS
echo =========================================================================

:: Use FOR /R to iterate through all *.h files in PLUGINS_DIR
:: This replaces the find "$PLUGINS_DIR" -name "*.h" loop
FOR /R "%PLUGINS_DIR%" %%F IN (*.h) DO (
    SET "header_file=%%F"
    
    :: Extract plugin name (filename without .h)
    SET "plugin_name=%%~nF"
    
    echo.
    echo Processing plugin: !plugin_name!
    
    SET "plugin_id="
    
    :: Check for stored ID (Using FINDSTR on the ID file)
    FOR /F "tokens=2 delims=:" %%B IN ('findstr /b /i "!plugin_name!:" "%ID_FILE%"') DO (
        SET "plugin_id=%%B"
    )

    IF DEFINED plugin_id (
        echo Reusing existing plugin ID: !plugin_id!
    )

    :: Step 1: Run cmake
    echo Running cmake for !plugin_name!...
    
    SET "cmake_args=cmake -B %BUILD_DIR% -S %JUCE_WRAPPER_DIR% -DNTFX_PLUGIN=%plugin_name%"
    
    IF DEFINED plugin_id (
        SET "cmake_args=%cmake_args% -DNTFX_ID=!plugin_id!"
    )

    :: Execute cmake and capture output
    CALL %cmake_args%
    SET "cmake_output=%ERRORLEVEL%"  :: Note: Capturing output reliably is extremely difficult in Batch
    
    :: --- Simplified ID Extraction (Assuming the grep pattern holds) ---
    SET "new_plugin_id="
    FOR /F "tokens=5 delims=' ' " %%I IN ('echo !cmake_output! ^| findstr /i "Generated new plugin id: [^ ]*";') DO (
        SET "new_plugin_id=%%I"
    )
    
    IF NOT DEFINED plugin_id (
        IF DEFINED new_plugin_id (
            SET "plugin_id=!new_plugin_id!"
            echo Found new plugin ID: !plugin_id!
        ) ELSE (
            echo Warning: Could not find plugin ID for !plugin_name!
        )
    )
    
    :: Update the ID file
    IF NOT DEFINED plugin_id (
        echo Warning: Skipping ID update for !plugin_name!
    ) ELSE (
        IF NOT DEFINED plugin_id_initial (
            echo !plugin_name!: !plugin_id! >> "%ID_FILE%"
            echo New ID recorded for !plugin_name!: !plugin_id!
        )
    )

    :: Step 2: Build the project
    echo Building !plugin_name!...
    cmake --build "!BUILD_DIR!" --config release

    :: Step 3 & 4: Copy artifacts
    SET "plugin_artefacts_dir=!BUILD_DIR!\!plugin_name!_artefacts"
    
    IF EXIST "!plugin_artefacts_dir!" (
        echo Copying artifacts for !plugin_name!...
        xcopy /E /I "!plugin_artefacts_dir!" "%ARTIFACTS_DIR%"
        echo Finished processing !plugin_name!
    ) ELSE (
        echo Warning: Could not find artifacts directory for !plugin_name!
    )
)

echo.
echo =========================================================================
echo All plugins processed.
echo Artifacts are in %ARTIFACTS_DIR%
echo Plugin IDs are stored in %ID_FILE%
echo =========================================================================

:end
echo.
echo Build process complete.
endlocal