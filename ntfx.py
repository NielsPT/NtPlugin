import sys
import os
import argparse
import subprocess
import re
import shutil
from multiprocessing import cpu_count
from testWrapper import test

PLUGINS_DIR = "plugins"  # Directory containing plugin .h files
BUILD_DIR = "build"  # Build directory
ARTIFACTS_DIR = "artifacts"  # Directory to store final artifacts
JUCE_WRAPPER_DIR = "JuceWrapper"  # Path to JuceWrapper directory
ID_FILE = f"{ARTIFACTS_DIR}/plugin_ids.txt"  # File to store plugin IDs
TEST_SCRIPT_DIR = "testWrapper"
TEST_DIR = "test"

ID = 0
VST3_CAT = 1
AAX_CAT = 2


def readPluginIds() -> dict[str, str]:
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
    files = os.listdir(PLUGINS_DIR)
    plugins: list[str] = []
    for file in files:
        if file.endswith(".h") and " " not in file:
            plugins += [file.replace(".h", "")]
    return plugins


def configure(plugin: str, pluginIds: dict[str, str]) -> bool:
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
    if plugin in pluginIds:
        info = pluginIds[plugin]
        args += [f"-DNTFX_ID={info[ID].strip()}"]
        print(f"Reusing existing plugin id for {plugin}: {info[ID].strip()}.")
        if len(info) > 2:
            args += [f"-DNTFX_VST3_CATEGORY={info[VST3_CAT].strip()}"]
            print(f"Reusing VST3 category: {info[VST3_CAT].strip()}.")
        if len(info) > 3:
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
        print("Cmake config failed.")
        print("\033[0m", end="")
        return False
    if plugin not in pluginIds:
        addNewPluginId(plugin, cmakeOut)
    return True


def addNewPluginId(plugin, cmakeOut):
    m = re.search("-- Generated new plugin id: ([^ ]*)\n", cmakeOut)
    if not m:
        print("Failed to get new plugin ID from cmake output.")
        return False
    newPluginId = m.group(1)
    if not newPluginId:
        return False
    os.makedirs(ARTIFACTS_DIR, exist_ok=True)
    with open(ID_FILE, "a", encoding="utf-8") as f:
        f.write(f"{plugin}: {newPluginId}")
    return True


def build():
    print("Building")
    args = ["cmake", "--build", BUILD_DIR, f"-j{cpu_count()}"]
    res = subprocess.run(args, check=False)
    if res.returncode:
        return False
    return True


def runCtest() -> bool:
    args = ["ctest", "--test-dir", BUILD_DIR]
    res = subprocess.run(args, check=False)
    return not bool(res.returncode)


def storeArtifacts(plugin) -> bool:
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
                f"Add {os.path.abspath(outDir)} to you host/DAW plugin path "
                "or copy content to your plugin folder in order to use plugins."
            )
    return True


def runAuVal():
    res = subprocess.run(["auval", "-vt", "aufx", "NtFx"], check=False)
    return not bool(res.returncode)


def process(plugins: list[str], preformTests: bool) -> bool:
    if preformTests:
        if not test.run({"files": plugins, "fs": 48e3}):
            return False
    pluginIds = readPluginIds()
    if not plugins or plugins == ["all"]:
        plugins = readPlugins()
    for plugin in plugins:
        if not configure(plugin, pluginIds):
            return False
        if not build():
            return False
        if preformTests:
            if not runCtest():
                return False
            if sys.platform == "darwin":
                if not runAuVal():
                    return False
        if not storeArtifacts(plugin):
            return False
    return True


def run() -> bool:
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
    args = parser.parse_args().__dict__
    if args["task"] == "build":
        return process(args["plugins"], args["test"])
    print(f"Unknown command: {args["task"]}")
    return False


if __name__ == "__main__":
    sys.exit(not run())
