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
import time
import json
from testWrapper import test
from JuceWrapper import package
from JuceWrapper.package import RED, YELLOW, BLACK, PLUGINS_DIR
from JuceWrapper import build

TEST_SCRIPT_DIR = os.path.realpath(f"{build.REPO_BASE_DIR}/testWrapper")


def _openInVscode(path: str) -> bool:
    try:
        subprocess.run(["code", path], check=False)
        return True
    except FileNotFoundError:
        return False


def _switchVsCodeSettings(plugin: str) -> bool:
    settingsFilePath = os.path.realpath(
        f"{build.REPO_BASE_DIR}/.vscode/settings.json"
    )
    if not os.path.exists(settingsFilePath):
        print(f"{RED}No Vscode settings found.{BLACK}")
        return False

    with open(settingsFilePath, "r", encoding="utf-8") as f:
        settings = json.loads(f.read())
    cmakeConfArgsKey = "cmake.configureArgs"
    if cmakeConfArgsKey not in settings:
        print(f"{RED}No Cmake config setting in Vscode settings.{BLACK}")
        return False
    configArgs: list[str] = settings[cmakeConfArgsKey]
    if not isinstance(configArgs, list):
        print(f"{RED}No Cmake config settings bad datatype.{BLACK}")
        return False

    for line in configArgs:
        if not line.startswith("-DNTFX_PLUGIN="):
            continue
        selectedPlugin = line.replace("-DNTFX_PLUGIN=", "")
        print(f"Currently selected plugin: {selectedPlugin}")
        if selectedPlugin == plugin or plugin == "":
            return True

    pluginPath = f"{PLUGINS_DIR}/{plugin}.h"
    if not os.path.exists(pluginPath):
        print(f"{RED}Plugin '{plugin}' not found.{BLACK}")
        return False
    pluginIds = build.readPluginIds()
    cat = ""
    if plugin not in pluginIds:
        print(f"{YELLOW}'{plugin}' not found in plugin ID file.")
    else:
        cat = pluginIds[plugin][build.AAX_CAT]
    newConfigArgs = [f"-DNTFX_PLUGIN={plugin}"]
    if cat:
        newConfigArgs += [
            f"-DNTFX_AAX_CATEGORY={cat}",
            f"-DNTFX_VST3_CATEGORY={build.CATEGORY_MAP[cat]}",
        ]
    settings[cmakeConfArgsKey] = newConfigArgs
    newSettingJson = json.dumps(settings, indent=2)
    with open(settingsFilePath, "w", encoding="utf-8") as f:
        f.write(newSettingJson)
    if not build.updatePluginHeader([plugin]):
        return False
    build.configure(plugin, pluginIds, cat, debug=True)
    _openInVscode(pluginPath)
    return True


def _writeFile(path: str, content: str) -> bool:
    if os.path.exists(path):
        print(f"{YELLOW}'{path}' already exists.{BLACK}")
        return False
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    return True


def newPlugin(plugin: str, cat: str = "") -> bool:
    """
    Creates a new plugin.  If Vscode is installed, opens the file.

    Args:
        name: Name of new plugin.
        cat: Optional category.

    Returns:
        bool: True on success.
    """
    template = f"""#pragma once

#include "lib/Plugin.h"
#include "lib/Audio.h"


struct {plugin} final : public NtFx::Plugin {{
  bool bypassEnable {{ false }};

  {plugin}() {{
    this->primaryKnobs = {{
    }};
    this->toggles = {{
      {{ .p_val = &this->bypassEnable, .name = "Bypass" }},
    }};
    this->updateDefaults();
  }}

  Audio process(Audio x) noexcept override {{
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {{
      this->updatePeakLevel(1, x);
      return x;
    }}
    Audio y = {{ 0, 0 }};
    this->updatePeakLevel(1, y);
    return y;
  }}

  void update() noexcept override {{
  }}

  void reset(signal_t fs) noexcept override {{
    this->_fs = fs;
    this->update();
  }}
}};
"""
    if cat:
        build.updatePluginId(plugin, cat)
    path = f"{build.REPO_BASE_DIR}/plugins/{plugin}.h"
    if not _writeFile(path, template):
        return False
    if not build.updatePluginHeader([plugin]):
        return False
    _switchVsCodeSettings(plugin)
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
    template = f"""
#include "lib/ComponentTest.h"
#include "plugins/{name}.h"
#include <memory>

int main() {{
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypass_ = std::make_unique<{name}>();
  auto& bypass = *bypass_;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaults_ = std::make_unique<{name}>();
  auto& defaults = *defaults_;
  NTFX_ADD_TEST(set, defaults, "impulse");
  return set.runAllTests();
}}
"""
    path = f"{build.REPO_BASE_DIR}/testWrapper/tests/{name}_test.cpp"
    if not _writeFile(path, template):
        return False
    _openInVscode(path)
    return True


def process(args: dict) -> bool:
    """
    Configures, builds and tests all plugins.

    Args:
        args (dict): CLI args.

    Returns:
        bool: True on success.
    """
    t = time.time()
    if args["test"]:
        if not test.run(
            {"files": args["plugins"], "fs": 48e3, "no_plot": True}
        ):
            return False
    if not build.main(args):
        return False
    print(f"Time elapsed: {time.time() - t:.2f} seconds.")
    return True


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
    subParsers.add_parser(
        "build",
        help="Build plugins.",
        parents=[build.createParser()],
        add_help=False,
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
    build.addCatArg(newParser)
    switchParser = subParsers.add_parser(
        "switch",
        help="Switch VsCode context to a different plugin.",
    )
    switchParser.add_argument("name", help="Name of plugin")
    whichParser = subParsers.add_parser(
        "which",
        help="Display currently selected plugin in Vscode settings.",
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
        return newPlugin(args["name"], args["category"])
    if args["task"] == "package":
        return package.main(args)
    if args["task"] == "switch":
        return _switchVsCodeSettings(args["name"])
    if args["task"] == "which":
        return _switchVsCodeSettings("")
    print(f"{RED}Unknown command: {args["task"]}{BLACK}.")
    return False


if __name__ == "__main__":
    sys.exit(not main(createParser().parse_args().__dict__))
