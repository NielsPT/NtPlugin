#!/bin/bash
# Configuration
PLUGINS_DIR="plugins"                   # Directory containing plugin .h files
BUILD_DIR="build"                       # Build directory
ARTIFACTS_DIR="artifacts"               # Directory to store final artifacts
JUCE_WRAPPER_DIR="JuceWrapper"          # Path to JuceWrapper directory
ID_FILE="$ARTIFACTS_DIR/plugin_ids.txt" # File to store plugin IDs

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
  local plugin_id=""
  if [ -n "${plugin_ids[$plugin_name]}" ]; then
    plugin_id="${plugin_ids[$plugin_name]}"
    echo "Reusing existing plugin ID: $plugin_id"
  fi

  # Step 1: Run cmake with the plugin name and captured output
  echo "Running cmake for $plugin_name..."
  cmake_output=$(cmake -B "$BUILD_DIR" -S "$JUCE_WRAPPER_DIR" -DNTFX_PLUGIN="$plugin_name" \
    ${plugin_id:+"-DNTFX_ID=$plugin_id"} --fresh 2>&1)
  echo $cmake_output

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

  # Step 2: Build the project
  echo "Building $plugin_name..."
  cmake --build "$BUILD_DIR" --config release

  # Step 3: Find the plugin-specific artifacts directory
  plugin_artefacts_dir="$BUILD_DIR/${plugin_name}_artefacts"

  if [ -d "$plugin_artefacts_dir" ]; then
    # Step 4: Copy the entire plugin artifacts directory to the final location
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

# #!/bin/bash

# # Configuration
# PLUGINS_DIR="plugins"          # Directory containing plugin .h files
# BUILD_DIR="build"              # Build directory
# ARTIFACTS_DIR="artefacts"      # Directory to store final artifacts
# JUCE_WRAPPER_DIR="JuceWrapper" # Path to JuceWrapper directory
# ID_FILE="$ARTIFACTS_DIR/plugin_ids.txt"       # File to store plugin IDs

# # Create artifacts directory if it doesn't exist
# mkdir -p "$ARTIFACTS_DIR"

# # Initialize ID map from existing file if it exists
# declare -A plugin_ids
# if [ -f "$ID_FILE" ]; then
#     while IFS=': ' read -r plugin_name id; do
#         plugin_ids["$plugin_name"]="$id"
#     done < "$ID_FILE"
# fi

# # Process all plugin files
# while IFS= read -r header_file; do
#     # Extract plugin name (filename without .h)
#     plugin_name=$(basename "$header_file" ".h")
#     echo "Processing plugin: $plugin_name"

#     # Check if we have a stored ID for this plugin
#     local plugin_id=""
#     if [ -n "${plugin_ids[$plugin_name]}" ]; then
#         plugin_id="${plugin_ids[$plugin_name]}"
#         echo "Reusing existing plugin ID: $plugin_id"
#     fi

#     # Step 1: Run cmake with the plugin name and captured output
#     echo "Running cmake for $plugin_name..."
#     cmake_output=$(cmake -B "$BUILD_DIR" -S "$JUCE_WRAPPER_DIR" -DNTFX_PLUGIN="$plugin_name" \
#         ${plugin_id:+"-DNTFX_ID=$plugin_id"} --fresh 2>&1)

#     # Extract the plugin ID from cmake output if we didn't reuse one
#     if [ -z "$plugin_id" ]; then
#         plugin_id=$(echo "$cmake_output" | grep -oP 'Generated new plugin id: \K[^ ]+')
#         if [ -n "$plugin_id" ]; then
#             echo "Found new plugin ID: $plugin_id"
#         else
#             echo "Warning: Could not find plugin ID for $plugin_name"
#         fi
#     fi

#     # Update the ID file if we have a new ID
#     if [ -n "$plugin_id" ] && [ -z "${plugin_ids[$plugin_name]}" ]; then
#         echo "$plugin_name: $plugin_id" >> "$ID_FILE"
#         plugin_ids["$plugin_name"]="$plugin_id"
#     fi

#     # Step 2: Build the project
#     echo "Building $plugin_name..."
#     cmake --build "$BUILD_DIR"

#     # Step 3: Find the plugin-specific artifacts directory
#     plugin_artefacts_dir="$BUILD_DIR/${plugin_name}_artefacts"

#     if [ -d "$plugin_artefacts_dir" ]; then
#         # Step 4: Copy the entire plugin artifacts directory to the final location
#         echo "Copying artifacts for $plugin_name..."
#         cp -r "$plugin_artefacts_dir" "$ARTIFACTS_DIR/"

#         echo "Finished processing $plugin_name"
#         echo "---------------------------------"
#     else
#         echo "Warning: Could not find artifacts directory for $plugin_name"
#     fi
# done < <(find "$PLUGINS_DIR" -name "*.h")

# echo "All plugins processed."
# echo "Artifacts are in $ARTIFACTS_DIR"
# echo "Plugin IDs are stored in $ID_FILE"
# ```

# #!/bin/bash

# # Configuration
# PLUGINS_DIR="plugins"          # Directory containing plugin .h files
# BUILD_DIR="build"              # Build directory
# ARTIFACTS_DIR="artefacts"      # Directory to store final artifacts
# JUCE_WRAPPER_DIR="JuceWrapper" # Path to JuceWrapper directory

# # Create artifacts directory if it doesn't exist
# mkdir -p "$ARTIFACTS_DIR"

# # Find all .h files in the plugins directory
# find "$PLUGINS_DIR" -name "*.h" | while read -r header_file; do
#     # Extract plugin name (filename without .h)
#     plugin_name=$(basename "$header_file" .h)
#     echo "Processing plugin: $plugin_name"

#     # Step 1: Run cmake with the plugin name
#     echo "Running cmake for $plugin_name..."
#     cmake -B "$BUILD_DIR" -S "$JUCE_WRAPPER_DIR" -DNTFX_PLUGIN="$plugin_name" --fresh

#     # Step 2: Build the project
#     echo "Building $plugin_name..."
#     cmake --build "$BUILD_DIR"

#     # Step 3: Find the plugin-specific artifacts directory
#     # JUCE typically creates a subdirectory with the plugin name
#     plugin_artefacts_dir="$BUILD_DIR/${plugin_name}_artefacts"

#     if [ -d "$plugin_artefacts_dir" ]; then
#         # Step 4: Copy the entire plugin artifacts directory to the final location
#         echo "Copying artifacts for $plugin_name..."
#         cp -r "$plugin_artefacts_dir" "$ARTIFACTS_DIR/"

#         echo "Finished processing $plugin_name"
#         echo "---------------------------------"
#     else
#         echo "Warning: Could not find artifacts directory for $plugin_name"
#     fi
# done

# echo "All plugins processed. Artifacts are in $ARTIFACTS_DIR"

# #!/bin/bash
# # Please be warned: This is pure slop.
# # Configuration
# PLUGINS_DIR="plugins"          # Directory containing plugin .h files
# BUILD_DIR="build"              # Build directory
# ARTIFACTS_DIR="artefacts"      # Directory to store final artifacts
# JUCE_WRAPPER_DIR="JuceWrapper" # Path to JuceWrapper directory

# # Create artifacts directory if it doesn't exist
# mkdir -p "$ARTIFACTS_DIR"

# # Find all .h files in the plugins directory
# find "$PLUGINS_DIR" -name "*.h" | while read -r header_file; do
#     # Extract plugin name (filename without .h)
#     plugin_name=$(basename "$header_file" .h)
#     echo "Processing plugin: $plugin_name"

#     # Step 1: Run cmake with the plugin name
#     echo "Running cmake for $plugin_name..."
#     cmake -B "$BUILD_DIR" -S "$JUCE_WRAPPER_DIR" -DNTFX_PLUGIN="$plugin_name" --fresh

#     # Step 2: Build the project
#     echo "Building $plugin_name..."
#     cmake --build "$BUILD_DIR"

#     # Step 3: Copy artifacts to artefacts directory
#     echo "Copying artifacts for $plugin_name..."

#     # Find and copy the relevant artifacts
#     find "$BUILD_DIR" -type f \( \
#         -name "*.aaxplugin" -o \
#         -name "*.au" -o \
#         -name "*.vst*" -o \
#         -name "*.exe" -o \
#         -name "*.dylib" -o \
#         -name "*.so" \
#     \) -exec cp {} "$ARTIFACTS_DIR"/ \;

#     echo "Finished processing $plugin_name"
#     echo "---------------------------------"
# done

# echo "All plugins processed. Artifacts are in $ARTIFACTS_DIR"
