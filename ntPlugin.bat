:: Copyright (C) 2026 Niels Thøgersen, NTlyd
::
:: This program is free software: you can redistribute it and/or modify it under
:: the terms of the GNU Affero General Public License as published by the Free
:: Software Foundation, either version 3 of the License, or (at your option) any
:: later version.
::
:: This program is distributed in the hope that it will be useful, but WITHOUT
:: ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
:: FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
:: details.
::
:: You should have received a copy of the GNU Affero General Public License
:: along with this program. If not, see <https://www.gnu.org/licenses/>.
::
:: You are free to download, build and use this code for commercial
:: purposes. Just don't resell it or a build of it, modified or otherwise.

:: @brief This script will build all plugins in the 'plugins' directory
:: in release mode and install them on the system if possible. Add "-t" as arg to
:: run tests on lib and all plugins.

@echo off
setlocal enabledelayedexpansion
SET "VENV_DIR=.venv"
IF NOT EXIST "%VENV_DIR%" (
  echo Creating Python virtual environment...
  python -m venv "%VENV_DIR%"
  CALL "%VENV_DIR%\Scripts\activate.bat"
  pip install -r requirements.txt
) else (
  CALL "%VENV_DIR%\Scripts\activate.bat"
)
python ntPlugin.py %*
endlocal