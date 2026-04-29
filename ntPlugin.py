#! ./.venv/bin/python

"""
@file ntPlugin.py
@author Niels Thøgersen (niels.thoegersen@gmail.com)
@brief Top level CLI for working with the NTplugin framework.
@version 0.1

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
details.
You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
"""

import sys
import os
import argparse
import subprocess
import re
import time
import shutil
from multiprocessing import cpu_count
from testWrapper import test

PLUGINS_DIR = "plugins"
BUILD_DIR = "build"
ARTIFACTS_DIR = "artifacts"
JUCE_WRAPPER_DIR = "JuceWrapper"
ID_FILE = f"{ARTIFACTS_DIR}/plugin_ids.txt"
TEST_SCRIPT_DIR = "testWrapper"
TEST_DIR = "test"
FILE_DIR = os.path.dirname(__file__)

ID = 0
VST3_CAT = 1
AAX_CAT = 2

CATEGORY_MAP = {
    """
    Maps from AAX format categories to VST3 format. AU doesn't make sense since 
    the only applicable fromat there in 'Effect'.
    Keys are AAX format, values are VST3 format.
    """
    "Effect": "Fx",
    "EQ": "EQ",
    "Dynamics": "Dynamics",
    "Reverb": "Reverb",
    "Delay": "Delay",
    "PitchShift": "Pitch Shift",
    "Modulation": "Modulation",
    "Harmonic": "Distortion",
    "NioseReduction": "Restoration",
    "SoundField": "Spacial",
}


def readPluginIds() -> dict[str, list[str]]:
    """
    Reads cached plugin ids along with categories from file and returns as a
    dict with plugin name as keys and list of strings as values. Order of values
    is ID, VST3 category, AAX category.
    """
    if not os.path.exists(ID_FILE):
        return {}
    pluginIds = {}
    with open(ID_FILE, "r", encoding="utf-8") as f:
        lines = f.readlines()
        for line in lines:
            kv = line.split(":")
            if len(kv) < 2:
                continue
            pluginIds[kv[0]] = kv[1:]
    return pluginIds


def readPlugins() -> list[str]:
    """
    Returns a list of all plugins found in folder 'plugins'.
    """
    files = os.listdir(PLUGINS_DIR)
    plugins: list[str] = []
    for file in files:
        if file.endswith(".h") and " " not in file:
            plugins += [file.replace(".h", "")]
    return plugins


def configure(
    plugin: str,
    pluginIds: dict[str, list[str]],
    category: str = "",
) -> bool:
    """
    Configures Cmake for build

    Args:
        plugin (str): Name of plugin to build.
        pluginIds (dict[str, list[str]]): Information about plugins. See 'readPluginIds' for more.
        category (str): AAX format category for plugin. Overrides categories in 'pluginIds' if set.

    Returns:
        bool: True on success.
    """
    if os.path.exists("build/CMakeCache.txt"):
        os.remove("build/CMakeCache.txt")
    args = [
        "cmake",
        "-B",
        BUILD_DIR,
        "-S",
        JUCE_WRAPPER_DIR,
        f"-DNTFX_PLUGIN={plugin}",
        "-DCMAKE_BUILD_TYPE=Release",
    ]
    if category:
        args += [f"-DNTFX_AAX_CATEGORY={category}"]
        args += [f"-DNTFX_VST3_CATEGORY={CATEGORY_MAP[category]}"]
    if plugin in pluginIds:
        info = pluginIds[plugin]
        args += [f"-DNTFX_ID={info[ID].strip()}"]
        print(f"Reusing existing plugin id for {plugin}: {info[ID].strip()}.")
        if not category and len(info) > 2:
            args += [f"-DNTFX_VST3_CATEGORY={info[VST3_CAT].strip()}"]
            print(f"Reusing VST3 category: {info[VST3_CAT].strip()}.")
        if not category and len(info) > 3:
            args += [f"-DNTFX_AAX_CATEGORY={info[AAX_CAT].strip()}"]
            print(f"Reusing AAX category: {info[AAX_CAT].strip()}.")

    print(f"Running cmake for plugin {plugin}.")
    res = subprocess.run(
        args, check=False, capture_output=True, env=os.environ.copy()
    )
    cmakeOut = res.stdout.decode()
    print(cmakeOut)
    if res.returncode:
        print("\033[31m", end="")
        print(f"Cmake config failed: {res.stderr}")
        print("\033[0m", end="")
        return False
    if plugin not in pluginIds:
        addNewPluginId(plugin, cmakeOut, category)
    return True


def addNewPluginId(plugin: str, cmakeOut: str, category: str = "") -> bool:
    """
    Adds a new plugin ID along with AAX and VST3 categories to plugin ID file.

    Args:
        plugin (str): Name of plugin.
        cmakeOut (str): Stdout from Cmake configure command. Used to deduce plugin ID.
        category (str): Category to add to record in ID file in AAX format.

    Returns:
        bool: True on success.
    """
    match = re.search("-- Generated new plugin id: ([^ ]*)\n", cmakeOut)
    if not match:
        print("Failed to get new plugin ID from cmake output.")
        return False
    newPluginId = match.group(1)
    if not newPluginId:
        return False
    os.makedirs(ARTIFACTS_DIR, exist_ok=True)
    with open(ID_FILE, "a", encoding="utf-8") as f:
        st = f"{plugin}:{newPluginId}"
        if category:
            st += f":{CATEGORY_MAP[category]}:{category}"
        f.write(f"{st}\n")
    return True


def build() -> bool:
    """
    Builds plugin.

    Returns:
        bool: True on success.
    """
    print("Building")
    args = [
        "cmake",
        "--build",
        BUILD_DIR,
        f"-j{cpu_count()}",
        "--config",
        "Release",
    ]
    res = subprocess.run(args, check=False)
    if res.returncode:
        print(f"Build failed: {res.stderr}")
        return False
    return True


def runCtest() -> bool:
    """
    Runs Ctest.

    Returns:
        bool: True on success.
    """
    args = ["ctest", "--test-dir", BUILD_DIR]
    res = subprocess.run(args, check=False)
    return not bool(res.returncode)


def storeArtifacts(plugin: str) -> bool:
    """
    Stores output from build to 'artifacts' folder. Sorts according to plugin
    type, that is 'AAX', 'VST3', 'AU' and 'Standalone'.

    Args:
        plugin (str): Name of plugin to find and store artifacts for.

    Returns:
        bool: True on success.
    """
    art = f"{BUILD_DIR}{os.sep}{plugin}_artefacts{os.sep}{"Release"}{os.sep}"
    if not os.path.exists(art):
        return False
    targets = []
    for e in os.scandir(art):
        if e.is_dir():
            targets += [e.path.replace(art, "")]
    for target in targets:
        outDir = f"{ARTIFACTS_DIR}{os.sep}{target}"
        os.makedirs(outDir, exist_ok=True)
        shutil.copytree(f"{art}{target}", outDir, dirs_exist_ok=True)
        print(f"Storing target {target} for plugin {plugin}.")
        if sys.platform == "win32" and target == "VST3":
            print(
                f"Add '{os.path.abspath(outDir)}' to you host/DAW plugin path "
                "or copy content to your plugin folder in order to use plugins."
            )
    return True


def runAuVal() -> bool:
    """
    Runs Apple's AU validation utility for all plugins made by the 'NTfx' vendor.

    Returns:
        bool: True on success.
    """
    try:
        res = subprocess.run(["auval", "-vt", "aufx", "NTfx"], check=False)
    except FileNotFoundError:
        print("Auval utility not found.")
        return False
    return not bool(res.returncode)


def process(
    plugins: list[str],
    preformTests: bool,
    category: str,
) -> bool:
    """
    Configures, builds and tests all plugins.

    Args:
        plugins (list[str]): List of names of plugins to process.
        preformTests (bool): If True, perform unittest before and validation after build.
        category (str): Category to put plugin in in DAW. AAX format.

    Returns:
        bool: True on success.
    """
    if preformTests:
        if not test.run({"files": plugins, "fs": 48e3}):
            return False
    pluginIds = readPluginIds()
    if not plugins or plugins == ["all"]:
        plugins = readPlugins()
    for plugin in plugins:
        if not configure(plugin, pluginIds, category):
            return False
        if not build():
            return False
        if preformTests:
            time.sleep(1)
            if sys.platform == "darwin":
                if not runAuVal():
                    return False
            if not runCtest():
                return False
        if not storeArtifacts(plugin):
            return False
    return True


def newPlugin(name: str) -> bool:
    """
    Creates a new plugin.  If Vscode is installed, opens the file.

    Args:
        name: Name of new plugin.

    Returns:
        bool: True on success.
    """
    template = f"""#pragma once

#include "lib/Plugin.h"
#include "lib/Stereo.h"

template <typename signal_t>
struct {name} : NtFx::NtPlugin<signal_t> {{
  bool bypassEnable {{ false }};
  // TODO: Create some variables.

  {name}() {{
    this->primaryKnobs = {{
      // TODO: Create some knobs.
    }};
    this->toggles = {{
      {{ .p_val = &this->bypassEnable, .name = "Bypass" }},
    }};
    this->updateDefaults();
  }}

  NtFx::Stereo<signal_t> process(NtFx::Stereo<signal_t> x) noexcept override {{
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {{
      this->template updatePeakLevel<1>(x);
      return x;
    }}
    auto y = {{ 0, 0 }};
    // TODO: processing.
    this->template updatePeakLevel<1>(y);
    return y;
  }}

  void update() noexcept override {{
    // TODO: Update coeffs.
  }}

  void reset(float fs) noexcept override {{
    this->fs = fs;
    // TODO: Allocate and reset.
    this->update();
  }}
}};
"""
    path = f"{FILE_DIR}/plugins/{name}.h"
    if os.path.exists(path):
        print(f"'{path}' already exists.")
        return False
    with open(path, "w", encoding="utf-8") as f:
        f.write(template)
    _openInVscode(path)
    return True


def newPluginTest(name: str):
    """
    Creates a new test file for a plugin plugin. If Vscode is installed, opens
    the file.

    Args:
        name: Name of new plugin.

    Returns:
        bool: True on success.
    """
    template = f"""#include "lib/ComponentTest.h"
#include "plugins/{name}.h"

NTFX_TEST_BEGIN

NTFX_TEST() {{
  auto bypass = {name}<double>();
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(bypass, "impulse");
  auto defaults = {name}<double>();
  NTFX_ADD_TEST(defaults, "impulse");
  // TODO: Add more tests.
  return NTFX_RUN_TESTS();
}}
"""
    path = f"{FILE_DIR}/testWrapper/tests/{name}_test.cpp"
    if os.path.exists(path):
        print(f"'{path}' already exists.")
        return False
    with open(path, "w", encoding="utf-8") as f:
        f.write(template)
    _openInVscode(path)
    return True


def _openInVscode(path: str) -> None:
    try:
        subprocess.run(["code", path], check=False)
    except FileNotFoundError:
        pass


def createParser() -> argparse.ArgumentParser:
    """
    Creates argument parser for ntPligin CLI.

    Returns:
        argparse.ArgumentParser: New parser.
    """
    parser = argparse.ArgumentParser(
        description="Builds and tests all plugins."
    )
    subParsers = parser.add_subparsers(dest="task")
    buildParser = subParsers.add_parser(
        "build", help="Build and install plugins."
    )
    buildParser.add_argument(
        "plugins",
        nargs="*",
        type=str,
        help="Plugins to build, placed in 'plugins' dir. Defaults to 'all', "
        "which builds all plugins.",
        default=["all"],
    )
    buildParser.add_argument(
        "--test",
        "-t",
        action="store_true",
        help="Apply uint tests before and validation after build.",
    )
    buildParser.add_argument(
        "--category",
        "-c",
        type=str,
        default="",
        choices=CATEGORY_MAP.keys(),
        help="Set the category you wish the plugin to be shown under in the "
        "host plugin list.",
    )
    subParsers.add_parser(
        "test",
        help="Runs unit tests.",
        parents=[test.createParser()],
        add_help=False,
    )
    newParser = subParsers.add_parser("new", help="Create a new plugin.")
    newParser.add_argument("name", help="Name of plugin.")
    newParser.add_argument(
        "--test",
        "-t",
        action="store_true",
        help="Add test file to 'testWrapper/tests'.",
    )
    return parser


def main() -> bool:
    """
    Main function for ntPlugin CLI.

    Returns:
        bool: True on success.
    """
    args = createParser().parse_args().__dict__
    if args["task"] == "build":
        return process(args["plugins"], args["test"], args["category"])
    if args["task"] == "test":
        return test.main(args)
    if args["task"] == "new":
        if "test" in args and args["test"]:
            newPluginTest(args["name"])
        return newPlugin(args["name"])
    print(f"Unknown command: {args["task"]}")
    return False


if __name__ == "__main__":
    sys.exit(not main())
