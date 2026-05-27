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
from JuceWrapper import package
from JuceWrapper.package import RED, YELLOW, GREEN, BLUE, BLACK

REPO_BASE_DIR = os.path.realpath(os.path.dirname(__file__))
BUILD_DIR = os.path.realpath(f"{REPO_BASE_DIR}/build")
ARTIFACTS_DIR = os.path.realpath(f"{REPO_BASE_DIR}/artifacts")
JUCE_WRAPPER_DIR = os.path.realpath(f"{REPO_BASE_DIR}/JuceWrapper")
TEST_SCRIPT_DIR = os.path.realpath(f"{REPO_BASE_DIR}/testWrapper")
ID_FILE = os.path.realpath(f"{JUCE_WRAPPER_DIR}/pluginIds.txt")


SECRETS = ["devId", "email", "password", "team", "installerId", "company"]
ID = 0
VST3_CAT = 1
AAX_CAT = 2

"""
Maps from AAX format categories to VST3 format. AU doesn't make sense since 
the only applicable fromat there in 'Effect'.
Keys are AAX format, values are VST3 format.
"""
CATEGORY_MAP = {
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
#include "lib/Audio.h"

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

  NtFx::Audio<signal_t> process(NtFx::Audio<signal_t> x) noexcept override {{
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
    path = f"{REPO_BASE_DIR}/plugins/{name}.h"
    if not _writeFile(path, template):
        return False
    _openInVscode(path)
    return True


def newPluginMono(name: str) -> bool:
    template = f"""#pragma once

#define NTFX_MONO

#include "{name}.h"

template <typename signal_t>
using {name}Mono = {name}<signal_t>;

    """
    path = f"{REPO_BASE_DIR}/plugins/{name}Mono.h"
    if not _writeFile(path, template):
        return False
    _openInVscode(path)
    return True


def _writeFile(path: str, content: str) -> bool:
    if os.path.exists(path):
        print(f"{YELLOW}'{path}' already exists.{BLACK}")
        return False
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
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
    path = f"{REPO_BASE_DIR}/testWrapper/tests/{name}_test.cpp"
    if os.path.exists(path):
        print(f"{YELLOW}'{path}' already exists.{BLACK}")
        return False
    with open(path, "w", encoding="utf-8") as f:
        f.write(template)
    _openInVscode(path)
    return True


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


def configure(
    plugin: str,
    pluginIds: dict[str, list[str]],
    category: str = "",
    version: str = "",
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
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE",
    ]
    if sys.platform == "win32":
        args += ["-A", "x64"]
    if version:
        args += [f"-DNTFX_VERSION={version}"]
    if category:
        args += [f"-DNTFX_AAX_CATEGORY={category}"]
        args += [f"-DNTFX_VST3_CATEGORY={CATEGORY_MAP[category]}"]
    if plugin in pluginIds:
        info = pluginIds[plugin]
        args += [f"-DNTFX_ID={info[ID].strip()}"]
        print(
            f"{BLUE}Reusing existing plugin id for {plugin}: "
            f"{info[ID].strip()}.{BLACK}"
        )
        if not category and len(info) > 2:
            args += [f"-DNTFX_VST3_CATEGORY={info[VST3_CAT].strip()}"]
            print(
                f"{BLUE}Reusing VST3 category: {info[VST3_CAT].strip()}.{BLACK}"
            )
        if not category and len(info) > 3:
            args += [f"-DNTFX_AAX_CATEGORY={info[AAX_CAT].strip()}"]
            print(
                f"{BLUE}Reusing AAX category: {info[AAX_CAT].strip()}.{BLACK}"
            )

    print(f"{BLUE}Running cmake for plugin {plugin}.{BLACK}")
    res = subprocess.run(
        args, check=False, capture_output=True, env=os.environ.copy()
    )
    cmakeOut = res.stdout.decode()
    print(cmakeOut)
    if res.returncode:
        print(f"{RED}Cmake config failed.{BLACK}")
        return False
    if plugin not in pluginIds:
        # TODO: updatePluginId for adding category.
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
        print(f"{RED}Failed to get new plugin ID from cmake output.{BLACK}")
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
    print(f"{BLUE}Building.{BLACK}")
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
        print(f"{RED}Build failed.{BLACK}")
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
        print(f"{GREEN}Storing target {target} for plugin {plugin}.{BLACK}")
        if sys.platform == "win32" and target == "VST3":
            print(
                f"{BLUE}Add '{os.path.abspath(outDir)}' to you host/DAW plugin "
                "path or copy content to your plugin folder in order to use "
                f"plugins.{BLACK}"
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
        print(f"{RED}Auval utility not found.{BLACK}")
        return False
    return not bool(res.returncode)


def process(args: dict) -> bool:
    """
    Configures, builds and tests all plugins.

    Args:
        args (dict): CLI args.

    Returns:
        bool: True on success.
    """
    plugins = args["plugins"] if "plugins" in args else []
    category = args["category"] if "category" in args else ""
    doTest = args["test"] if "test" in args else False
    doPackage = args["package"] if "package" in args else False
    secrets = package.loadSecrets()
    for secret in SECRETS:
        if secret in args and args[secret]:
            secrets[secret] = args[secret].replace("/n", "")
    if not secrets:
        print(f"{RED}Faild to get credentials.{BLACK}")
        return False
    version = package.findVersion(args, secrets)
    if not version:
        print(f"{RED}Version is not provided. Aborting.{BLACK}")
        return False
    secrets["version"] = version
    if doTest:
        if not test.run({"files": plugins, "fs": 48e3}):
            return False
    pluginIds = readPluginIds()
    allPlugins = package.readPlugins()
    if not plugins or plugins == ["all"]:
        plugins = allPlugins
    for plugin in plugins:
        if plugin not in allPlugins:
            print(f"{RED}Plugin '{plugin}' does not exist.{BLACK}")
            return False
        if not configure(plugin, pluginIds, category, version):
            return False
        if not build():
            return False
        if doTest:
            for _ in range(30):
                if _artefactExists(plugin, "AU"):
                    if _artefactExists(plugin, "VST3"):
                        break
                print(f"{RED}Test files not found.{BLACK}")
                time.sleep(1)
            if not runCtest():
                return False
        if not storeArtifacts(plugin):
            return False
    if doTest and sys.platform == "darwin":
        if not runAuVal():
            return False
    if doPackage:
        if not package.main(args):
            return False
    if not package.storeSecrets(secrets):
        return False
    return True


def _artefactExists(plugin: str, target: str) -> bool:
    if os.path.exists(
        f"{BUILD_DIR}/{plugin}_artefacts/Release/{target}/"
        f"{plugin}.{package.TARGET_EXT_MAP[target]}"
    ):
        return True
    return False


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
        "host plugin list. Setting is cached, so it's only needed once per "
        "plugin, and never when building all plugins.",
    )
    buildParser.add_argument(
        "--package",
        "-p",
        action="store_true",
        help="For MacOS, package plugins as an installer. Requires 'package' "
        "command to have been run with full credentials and at least once, "
        "which will cache the credentials.",
    )
    buildParser.add_argument(
        "--version",
        help="Version to be used for all plugins and installer. Cached and "
        "minor minor number auto-incremented if not given.",
    )
    parser.add_argument(
        "--company",
        help="Company/vendor of plugins.",
        default="NTfx",
    )
    subParsers.add_parser(
        "test",
        help="Runs unit tests.",
        parents=[test.createParser()],
        add_help=False,
    )
    subParsers.add_parser(
        "package",
        help="Package plugins as zip or installer.",
        parents=[package.createParser()],
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
    newParser.add_argument(
        "--mono",
        "--add-mono",
        "-m",
        action="store_true",
        help="Create a mono stub for the plugin. Builds a mono version of the "
        "plugin. New mono-plugin is named [plugin name]Mono",
    )
    return parser


def main(args: dict) -> bool:
    """
    Main function for ntPlugin CLI.

    Returns:
        bool: True on success.
    """
    if args["task"] == "build":
        return process(args)
    if args["task"] == "test":
        return test.main(args)
    if args["task"] == "new":
        if "test" in args and args["test"]:
            newPluginTest(args["name"])
        if "mono" in args and args["mono"]:
            newPluginMono(args["name"])
        return newPlugin(args["name"])
    if args["task"] == "package":
        return package.main(args)
    print(f"{RED}Unknown command: {args["task"]}{BLACK}.")
    return False


if __name__ == "__main__":
    sys.exit(not main(createParser().parse_args().__dict__))
