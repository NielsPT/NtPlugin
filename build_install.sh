#!/bin/bash

# Copyright (C) 2026 Niels Thøgersen, NTlyd
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU Affero General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
# You are free to download, build and use this code for commercial
# purposes. Just don't resell it or a build of it, modified or otherwise.
#

#
# @brief This script will build all plugins in the 'plugins' directory
# in release mode and install them on the system if possible. Add "-t" as arg to
# run tests on lib and all plugins.
#

# Configuration
PLUGINS_DIR="plugins"                   # Directory containing plugin .h files
BUILD_DIR="build"                       # Build directory
ARTIFACTS_DIR="artifacts"               # Directory to store final artifacts
JUCE_WRAPPER_DIR="JuceWrapper"          # Path to JuceWrapper directory
ID_FILE="$ARTIFACTS_DIR/plugin_ids.txt" # File to store plugin IDs
TEST_SCRIPT_DIR="testWrapper"
TEST_DIR="test"

if [[ $# -gt 0 ]] && [[ "$1" == "-t" ]]; then
  if [ ! -d .venv ]; then
    python3 -m venv .venv;
    . ./.venv/bin/activate;
    pip install -r ${TEST_SCRIPT_DIR}/requirements.txt;
  else
    . ./.venv/bin/activate;
  fi
  python ./${TEST_SCRIPT_DIR}/test.py run all || exit 1
fi

# Create artifacts directory if it doesn't exist
mkdir -p "$ARTIFACTS_DIR"

# Initialize ID map from existing file if it exists
declare -A plugin_ids
if [ -f "$ID_FILE" ]; then
  while IFS=': ' read -r plugin_name id; do
    plugin_ids[$plugin_name]="$id"
  done <"$ID_FILE"
fi

# Process all plugin files
while IFS= read -r header_file; do
  # Extract plugin name (filename without .h)
  plugin_name=$(basename "$header_file" .h)
  echo "Processing plugin: $plugin_name, id: ${plugin_ids[$plugin_name]}"

  # Check if we have a stored ID for this plugin
  plugin_id=""
  if [ -n "${plugin_ids[$plugin_name]}" ]; then
    plugin_id="${plugin_ids[$plugin_name]}"
    echo "Reusing existing plugin ID: $plugin_id"
  fi

  # Run cmake with the plugin name and captured output
  echo "Running cmake for $plugin_name..."
  cmake_output=$(cmake -B "$BUILD_DIR" -S "$JUCE_WRAPPER_DIR" -DNTFX_PLUGIN="$plugin_name" \
    ${plugin_id:+"-DNTFX_ID=$plugin_id"} -DCMAKE_BUILD_TYPE=Release 2>&1)
  echo $cmake_output
  if [ $? -ne 0 ]; then echo "Exit code: $? Aborting"; exit 1; fi

  # Extract the plugin ID from cmake output if we didn't reuse one
  if [ -z "$plugin_id" ]; then
    plugin_id=$(echo "$cmake_output" | grep -o 'Generated new plugin id: [^ ]*' | cut -d' ' -f5)
    if [ -n "$plugin_id" ]; then
      echo "Found new plugin ID: $plugin_id"
    else
      echo "Warning: Could not find plugin ID for $plugin_name"
    fi
  fi

  # Update the ID file if we have a new ID
  if [ -n "$plugin_id" ] && [ -z "${plugin_ids[$plugin_name]}" ]; then
    echo "$plugin_name: $plugin_id" >>"$ID_FILE"
    plugin_ids["$plugin_name"]="$plugin_id"
  fi

  # Build the project
  echo "Building $plugin_name..."
  cmake --build "$BUILD_DIR" -j$(nproc --all) || exit 1

  if [[ $# -gt 0 ]] && [[ "$1" == "-t" ]]; then
    # A short sleep to make sure the plugin exists before validation.
    sleep 0.1
  
    # Test the plugin with pluginval
    ctest --test-dir "$BUILD_DIR" || exit 1
  fi

  # Find the plugin-specific artifacts directory
  plugin_artefacts_dir="$BUILD_DIR/${plugin_name}_artefacts"

  if [ -d "$plugin_artefacts_dir" ]; then
    # Copy the entire plugin artifacts directory to the final location
    echo "Copying artifacts for $plugin_name..."
    cp -r "$plugin_artefacts_dir" "$ARTIFACTS_DIR/"

    echo "Finished processing $plugin_name"
    echo "---------------------------------"
  else
    echo "Warning: Could not find artifacts directory for $plugin_name"
  fi
done < <(find "$PLUGINS_DIR" -name "*.h")

echo "All plugins processed."
echo "Artifacts are in $ARTIFACTS_DIR"z
echo "Plugin IDs are stored in $ID_FILE"

if [[ "$(uname)" == "Darwin" ]]; then
  echo "Testing AU plugins"
  auval -vt aufx NtFx
fi
