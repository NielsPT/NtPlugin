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

FILE_DIR = os.path.dirname(__file__)
PLUGINS_DIR = f"{FILE_DIR}/plugins"
BUILD_DIR = f"{FILE_DIR}/build"
ARTIFACTS_DIR = f"{FILE_DIR}/artifacts"
JUCE_WRAPPER_DIR = f"{FILE_DIR}/JuceWrapper"
TEST_SCRIPT_DIR = f"{FILE_DIR}/testWrapper"
ID_FILE = f"{JUCE_WRAPPER_DIR}/pluginIds.txt"
SECRETS_FILE = f"{FILE_DIR}/.secrets.txt"
PACKAGING_DIR = f"{BUILD_DIR}/packaging"
PACKAGE_ARTIFACT = "ntPlugin"

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

"""
Maps from folders containing resulting artifacts to extensions of those 
artifacts.
"""
TARGET_EXT_MAP = {
    "VST3": "vst3",
    "AU": "component",
    "AAX": "aaxplugin",
    "Standalone": "app",
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
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE",
    ]
    if sys.platform == "win32":
        args += ["-A", "x64"]
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


def _storeSecrets(secrets: dict[str, str]) -> bool:
    for secret in secrets:
        if not secrets[secret] or secret not in SECRETS:
            return False
    with open(SECRETS_FILE, "w", encoding="utf-8") as f:
        for k, v in secrets.items():
            f.write(f"{k}:{v}\n")
    print(f"Stored secrets to file '{SECRETS_FILE}'.")
    res = subprocess.run(
        [
            "xcrun",
            "notraytool",
            "store-credentials",
            "ntPlugin-credentials",
            "--apple-id",
            secrets["email"],
            "--team-id",
            secrets["team"],
            "--password",
            secrets["password"],
        ],
        check=False,
    )
    if res.returncode:
        return False
    return True


def _loadSecrets() -> dict[str, str]:
    if not os.path.exists(SECRETS_FILE):
        print("Secrets file not found.")
        return {}
    with open(SECRETS_FILE, "r", encoding="utf-8") as f:
        lines = f.readlines()
    if not lines:
        return {}
    secrets = {}
    for line in lines:
        d = line.split(":", 1)
        secrets[d[0]] = d[1].replace("\n", "")
    return secrets


def sign(plugins: list[str], devId: str) -> bool:
    """
    Applies code signing on Mac.

    Args:
        plugins (list[str]): Names of plugins to sign.
        devId (str): Developer Application ID as generated at
        developer.apple.com or using Xcode.

    Returns:
        bool: True on success.
    """
    for target, extension in TARGET_EXT_MAP.items():
        for plugin in plugins:
            file = f"{ARTIFACTS_DIR}/{target}/{plugin}.{extension}"
            print(f"Running codesign for '{file}'.")
            if not os.path.exists(file):
                print(f"Artifact '{file}' not found.")
                continue
            res = subprocess.run(
                [
                    "codesign",
                    "--force",
                    "-s",
                    f"{devId}",
                    f"{ARTIFACTS_DIR}/{target}/{plugin}.{extension}",
                    "-v",
                    "--deep",
                    "--strict",
                    "--options=runtime",
                    "--timestamp",
                ],
                check=False,
            )
            if res.returncode:
                print(f"Codesign failed for '{plugin}'.")
                return False
            print(f"'{plugin}' signed succesfully.")
    return True


def notarizePlugins(
    plugins: list[str],
    email: str,
    password: str,
    teamId: str,
) -> bool:
    """
    Notarizes plugins.

    Args:
        plugins (list[str]): Names of plugins to notarize.
        email (str): Email address/user name for Apple ID.
        password (str): Password for Apple Developer ID. Must be app-specific.
        teamId (str): Team ID from Apple Developer.

    Returns:
        bool: True on success.
    """
    for target, extension in TARGET_EXT_MAP.items():
        for plugin in plugins:
            file = f"{ARTIFACTS_DIR}/{target}/{plugin}.{extension}"
            print(f"Running codesign for '{file}'.")
            if not os.path.exists(file):
                print(f"Artifact '{file}' not found.")
                continue
            tmpZipFile = f"{BUILD_DIR}/{plugin}_{target}_unnotarized.zip"
            os.chdir(f"{ARTIFACTS_DIR}/{target}")
            res = subprocess.run(
                [
                    "zip",
                    f"../../{tmpZipFile}",
                    f"{plugin}.{extension}",
                    "-r",
                ],
                check=False,
            )
            os.chdir(FILE_DIR)
            if res.returncode:
                print(f"Failed to compress '{file}'.")
                return False
            if not _notarize(tmpZipFile, email, password, teamId):
                print(f"Failed to notarize '{plugin}'.")
                return False
            print(f"'{plugin}' notarized succesfully.")
    return True


def _notarize(
    file: str,
    email: str,
    password: str,
    teamId: str,
) -> bool:
    res = subprocess.run(
        [
            "xcrun",
            "notarytool",
            "submit",
            file,
            "--apple-id",
            email,
            "--password",
            password,
            "--team-id",
            teamId,
            "--wait",
        ],
        check=False,
    )
    if res.returncode:
        return False
    return True


def _validateAppSigning(plugins: list[str]) -> bool:
    for plugin in plugins:
        path = f"{ARTIFACTS_DIR}/Standalone/{plugin}.app"
        if not os.path.exists(path):
            print(f"'{path}' does not exist.")
            return False
        res = subprocess.run(
            ["spctl", "-vvv", "--asses", "--type", "exec", path],
            check=False,
        )
        if res.returncode:
            return False
    return True


def _validateInsataller(path: str) -> bool:
    res = subprocess.run(
        [
            "spctl",
            "-vvv",
            "--asses",
            "--type",
            "install",
            path,
        ],
        check=False,
    )
    if res.returncode:
        return False
    return True


def staple(plugins: list[str]) -> bool:
    """
    Staple plugins with approval. Depends on plugins being notarized.

    Args:
        plugins (list[str]): Plugins to staple.

    Returns:
        bool: Trie on success.
    """
    for target, extension in TARGET_EXT_MAP.items():
        for plugin in plugins:
            path = f"{ARTIFACTS_DIR}/{target}/{plugin}.{extension}"
            if not _staple(path):
                return False
    return True


def _staple(path: str) -> bool:
    if not os.path.exists(path):
        print(f"'{path}' does not exist.")
        return False
    res = subprocess.run(
        ["xcrun", "stapler", "staple", path],
        check=False,
    )
    if res.returncode:
        print(f"Failed to staple '{path}'.")
        return False
    return True


def _zipPackage(plugins: list[str]) -> bool:
    targetDirs = [f"{ARTIFACTS_DIR}/{d}" for d in TARGET_EXT_MAP]
    files = []
    for target in targetDirs:
        _plugins = os.listdir(target)
        for _plugin in _plugins:
            if _plugin.split(".")[0] in plugins:
                files += [f"{target}/{_plugin}"]
    zipFileName = PACKAGE_ARTIFACT
    if len(plugins) == 1:
        zipFileName = plugins[0]
    outPath = f"{ARTIFACTS_DIR}/{zipFileName}.zip"
    res = subprocess.run(
        ["zip", "-r", outPath] + files,
        check=False,
    )
    if res.returncode:
        return False
    print(f"Package stored to '{outPath}'.")
    return True


def _makeDistributionXml(
    plugins: list[str],
    company: str,
    version: str = "0.1.0",
) -> bool:
    xml = '<?xml version="1.0" encoding="utf-8" standalone="no"?>\n'
    xml += '<installer-gui-script minSpecVersion="2">\n'
    xml += '<options customize="always" require-scripts="false" '
    xml += 'hostArchitectures="arm64"/>\n'
    xml += '<choices-outline>\n'
    for plugin in plugins:
        for target in TARGET_EXT_MAP:
            xml += f'  <line choice="{plugin}.{target}" />\n'
    xml += '</choices-outline>\n'
    for plugin in plugins:
        for target, extension in TARGET_EXT_MAP.items():
            xml += f'<choice id="{plugin}.{target}" visible="true" '
            xml += f'start_selected="true" title="{plugin} {target}">\n'
            xml += '  <pkg-ref '
            xml += f'id="com.{company}.{plugin}.{target.lower()}.pkg" '
            # TODO: What to do about version?
            xml += f'version="{version}" onConclusion="none">\n'
            xml += f"    {plugin}.{target}.pkg\n"
            xml += '  </pkg-ref>\n'
            xml += '</choice>\n'
    xml += '</installer-gui-script>\n'
    with open(f"{PACKAGING_DIR}/dist.xml", "w", encoding="utf-8") as f:
        f.write(xml)
    return True


def _makeInstallerPkg(installerId: str) -> bool:
    os.chdir(PACKAGING_DIR)
    res = subprocess.run(
        [
            "productbuild",
            "--resources",
            f"{PACKAGING_DIR}",
            "--distribution",
            f"{PACKAGING_DIR}/dist.xml",
            "--sign",
            installerId,
            "--timestamp",
            f"{ARTIFACTS_DIR}/{PACKAGE_ARTIFACT}.pkg",
        ],
        check=False,
        capture_output=True,
    )
    os.chdir(FILE_DIR)
    print(res.stdout.decode())
    if "warning" in res.stdout.decode():
        return False
    if res.returncode:
        return False
    return True


def _makePluginPkgs(plugins: list[str], company: str) -> bool:
    for plugin in plugins:
        # TODO: Targets arg. Everywhere.
        for target, extension in TARGET_EXT_MAP.items():
            installLocation = "/Library/Audio/Plug-Ins/VST3"
            if target == "AU":
                installLocation = "/Library/Audio/Plug-Ins/Components"
            if target == "Standalone":
                installLocation = "/Applications"
            if target == "AAX":
                installLocation = (
                    "/Library/Application Support/Avid/Audio/Plug-Ins"
                )
            os.chdir(PACKAGING_DIR)
            res = subprocess.run(
                [
                    "pkgbuild",
                    "--identifier",
                    f"com.{company}.{plugin}.{target.lower()}.pkg",
                    "--component",
                    f"{ARTIFACTS_DIR}/{target}/{plugin}.{extension}",
                    "--install-location",
                    installLocation,
                    f"{plugin}.{target}.pkg",
                ],
                check=False,
            )
            if res.returncode:
                return False
            os.chdir(FILE_DIR)
    return True


def package(args: dict) -> bool:
    """
    Packages selected plugins for MacOS.

    Args:
        args (dict): CLI arguments.

    Returns:
        bool: True on success.
    """
    if sys.platform != "darwin":
        print("Packaging only available for MacOS.")
        return False
    os.makedirs(PACKAGING_DIR, exist_ok=True)
    plugins = args["plugins"]
    if not plugins or plugins == ["all"]:
        plugins = readPlugins()
    secrets = _loadSecrets()
    for secret in SECRETS:
        if args[secret]:
            secrets[secret] = args[secret].replace("/n", "")
    if secrets and args["store_secrets"]:
        if not _storeSecrets(secrets):
            return False
    if not secrets:
        print("Faild to get credentials.")
        return False
    if not args["no_sign"]:
        if not sign(plugins, secrets["devId"]):
            return False
        if not _validateAppSigning(plugins):
            return False
    if args["zip"]:
        if not args["no_notarize"]:
            if not notarizePlugins(
                plugins,
                secrets["email"],
                secrets["password"],
                secrets["team"],
            ):
                return False
        if not args["no_staple"]:
            if not staple(plugins):
                return False
            if not _zipPackage(plugins):
                return False
        return True
    if not _makePluginPkgs(plugins, secrets["company"]):
        return False
    if not _makeDistributionXml(plugins, secrets["company"]):
        return False
    if not _makeInstallerPkg(secrets["installerId"]):
        return False
    if not args["no_notarize"]:
        if not _notarize(
            f"{ARTIFACTS_DIR}/{PACKAGE_ARTIFACT}.pkg",
            secrets["email"],
            secrets["password"],
            secrets["team"],
        ):
            return False
    if not args["no_staple"]:
        if not _staple(f"{ARTIFACTS_DIR}/{PACKAGE_ARTIFACT}.pkg"):
            return False
    if not _validateInsataller(f"{ARTIFACTS_DIR}/{PACKAGE_ARTIFACT}.pkg"):
        return False
    return True


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
    if doTest:
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
        if doTest:
            time.sleep(1)
            if sys.platform == "darwin":
                if not runAuVal():
                    return False
            if not runCtest():
                return False
        if not storeArtifacts(plugin):
            return False
    if doPackage:
        if not package(args):
            return False
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
    buildParser.add_argument(
        "--package",
        action="store_true",
        help="For MacOS, package plugins as an installer. Requires 'package' "
        "command to have been run with full credentials and 'store_secrets' "
        "flag at least once.",
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
    packageParser = subParsers.add_parser(
        "package",
        help="Sign, notarize, staple and bundle for MacOS. Requires Apple ID, "
        "Application developer ID and Apple Developer team ID. Once set once, "
        "credentials are cached in '.secrets'.",
    )
    packageParser.add_argument("plugins", help="Plugins to package.", nargs="*")
    packageParser.add_argument(
        "--devId",
        "-d",
        help="Application Developer ID Application as exported from Xcode.",
        type=str,
    )
    packageParser.add_argument("--email", help="Apple ID email address.")
    packageParser.add_argument(
        "--password",
        "-p",
        help="Password for Apple ID. Must be an 'app-specific password',",
    )
    packageParser.add_argument(
        "--team",
        "-t",
        help="Team ID. Found in the paranthesis in the Developer ID.",
    )
    packageParser.add_argument(
        "--installerId",
        "-i",
        help="Developer ID Installer from Apple.",
    )
    packageParser.add_argument(
        "--store_secrets",
        "--store-secrets",
        "-s",
        action="store_true",
        help="Store IDs and password to file.",
    )
    packageParser.add_argument(
        "--no_notarize",
        "--no-notarize",
        action="store_true",
        help="Don't notarize artifacts.",
    )
    packageParser.add_argument(
        "--no_sign",
        "--no-sign",
        action="store_true",
        help="Don't sign artifacts.",
    )
    packageParser.add_argument(
        "--no_staple",
        "--no-staple",
        action="store_true",
        help="Don't staple artifacts.",
    )
    packageParser.add_argument(
        "--zip",
        "-z",
        action="store_true",
        help="Instead of creating an installer, create a zip file with "
        "notarized plugins.",
    )
    # TODO: Set company in Cmake from this.
    packageParser.add_argument(
        "--company",
        help="Company/vendor of plugin.",
        default="NTfx",
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
        return process(args)
    if args["task"] == "test":
        return test.main(args)
    if args["task"] == "new":
        if "test" in args and args["test"]:
            newPluginTest(args["name"])
        return newPlugin(args["name"])
    if args["task"] == "package":
        return package(args)
    print(f"Unknown command: {args["task"]}")
    return False


if __name__ == "__main__":
    sys.exit(not main())
