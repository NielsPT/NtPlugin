#! ./.venv/bin/python

"""
@file package.py
@author Niels Thøgersen (niels.thoegersen@gmail.com)
@brief Packaging of plugins for MacOS.
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

import subprocess
import os
import argparse
import sys

BLACK = "\033[0m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"

REPO_BASE_DIR = os.path.realpath(f"{os.path.dirname(__file__)}/..")
PLUGINS_DIR = os.path.realpath(f"{REPO_BASE_DIR}/plugins")
BUILD_DIR = os.path.realpath(f"{REPO_BASE_DIR}/build")
ARTIFACTS_DIR = os.path.realpath(f"{REPO_BASE_DIR}/artifacts")
JUCE_WRAPPER_DIR = os.path.realpath(f"{REPO_BASE_DIR}/JuceWrapper")
TEST_SCRIPT_DIR = os.path.realpath(f"{REPO_BASE_DIR}/testWrapper")
SECRETS_FILE = os.path.realpath(f"{JUCE_WRAPPER_DIR}/.secrets.txt")
PACKAGING_DIR = os.path.realpath(f"{BUILD_DIR}/packaging")
PACKAGE_ARTIFACT = "ntPlugin"

SECRETS = [
    "devId",
    "email",
    "password",
    "team",
    "installerId",
    "company",
    "version",
]

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


def _pluginPath(plugin: str, target: str):
    return f"{ARTIFACTS_DIR}/{target}/{plugin}.{TARGET_EXT_MAP[target]}"


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


def storeSecrets(secrets: dict[str, str]) -> bool:
    for secret in secrets:
        if not secrets[secret] or secret not in SECRETS:
            return False
    with open(SECRETS_FILE, "w", encoding="utf-8") as f:
        for k, v in secrets.items():
            f.write(f"{k}:{v}\n")
    print(f"{GREEN}Stored secrets to file '{SECRETS_FILE}'.{BLACK}")
    return True


def loadSecrets() -> dict[str, str]:
    if not os.path.exists(SECRETS_FILE):
        print(f"{YELLOW}Secrets file not found.{BLACK}")
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


def sign(plugins: list[str], targets: list[str], devId: str) -> bool:
    """
    Applies code signing on Mac.

    Args:
        plugins (list[str]): Names of plugins to sign.
        targets (list[str]): Names of targeted plugin formats.
        devId (str): Developer Application ID as generated at
        developer.apple.com or using Xcode.

    Returns:
        bool: True on success.
    """
    for target in targets:
        extension = TARGET_EXT_MAP[target]
        for plugin in plugins:
            file = f"{ARTIFACTS_DIR}/{target}/{plugin}.{extension}"
            print(f"{BLUE}Running codesign for '{file}'.{BLACK}")
            if not os.path.exists(file):
                print(f"{YELLOW}Artifact '{file}' not found. Skipping.{BLACK}")
                continue
            res = subprocess.run(
                [
                    "codesign",
                    "--force",
                    "-s",
                    f"{devId}",
                    file,
                    "-v",
                    "--deep",
                    "--strict",
                    "--options=runtime",
                    "--timestamp",
                ],
                check=False,
            )
            if res.returncode:
                print(f"{RED}Codesign failed for '{plugin}'.{BLACK}")
                return False
            print(f"{GREEN}'{plugin}' signed succesfully.{BLACK}")
    return True


def notarizePlugins(
    plugins: list[str],
    targets: list[str],
    email: str,
    password: str,
    teamId: str,
) -> bool:
    """
    Notarizes plugins.

    Args:
        plugins (list[str]): Names of plugins to notarize.
        targets (list[str]): Names of targeted plugin formats.
        email (str): Email address/user name for Apple ID.
        password (str): Password for Apple Developer ID. Must be app-specific.
        teamId (str): Team ID from Apple Developer.

    Returns:
        bool: True on success.
    """
    for target in targets:
        extension = TARGET_EXT_MAP[target]
        for plugin in plugins:
            file = f"{ARTIFACTS_DIR}/{target}/{plugin}.{extension}"
            print(f"{BLUE}Running codesign for '{file}'.{BLACK}")
            if not os.path.exists(file):
                print(f"{RED}Artifact '{file}' not found.{BLACK}")
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
            os.chdir(REPO_BASE_DIR)
            if res.returncode:
                print(f"{RED}Failed to compress '{file}'.{BLACK}")
                return False
            if not notarize(tmpZipFile, email, password, teamId):
                print(f"{RED}Failed to notarize '{plugin}'.{BLACK}")
                return False
            print(f"{GREEN}'{plugin}' notarized succesfully.{BLACK}")
    return True


def notarize(
    path: str,
    email: str,
    password: str,
    teamId: str,
) -> bool:
    """
    Notarizes an inastaller or plugin with Apple's servers.

    Args:
        path (str): Path of file to me notarized.
        email (str): E-mail address of Apple Developer Account.
        password (str): Password of Apple Developer Account.
        teamId (str): Apple Developer team ID.

    Returns:
        bool: True on success.
    """
    res = subprocess.run(
        [
            "xcrun",
            "notarytool",
            "submit",
            path,
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


def validateInstaller(path: str) -> bool:
    """
    Returns True if installer is signed, notarized and stapled.

    Args:
        path (str): Path of installer.

    Returns:
        bool: True if valid.
    """
    print(f"{BLUE}Validating installer '{path}'.{BLACK}")
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
        print(f"{RED}Installer validation failed.{BLACK}")
        return False
    print(f"{GREEN}Installer validation passed.{BLACK}")
    return True


def staplePlugins(plugins: list[str], targets: list[str]) -> bool:
    """
    Staple plugins with approval. Depends on plugins being notarized.

    Args:
        plugins (list[str]): Plugins to staple.

    Returns:
        bool: Trie on success.
    """
    print(f"{BLUE}Stapling plugins.{BLACK}")
    for target in targets:
        extension = TARGET_EXT_MAP[target]
        for plugin in plugins:
            path = f"{ARTIFACTS_DIR}/{target}/{plugin}.{extension}"
            if not staple(path):
                return False
    return True


def staple(path: str) -> bool:
    """
    Staples selected file with signature from Apple.

    Args:
        path (str): Path to file to staple.

    Returns:
        bool: True on success.
    """
    print(f"{BLUE}Stapling '{path}'.{BLACK}")
    if not os.path.exists(path):
        print(f"{RED}'{path}' does not exist.{BLACK}")
        return False
    res = subprocess.run(
        ["xcrun", "stapler", "staple", path],
        check=False,
    )
    if res.returncode:
        print(f"{RED}Failed to staple '{path}'.{BLACK}")
        return False
    return True


def zipPackage(plugins: list[str], targets: list[str], version: str) -> bool:
    """
    Zips selected plugins in a single package for 'manual install' distribution.

    Args:
        plugins (list[str]): Plugins to package.
        targets (list[str]): Target formats to package.

    Returns:
        bool: True on success.
    """
    targetDirs = [f"{ARTIFACTS_DIR}/{d}" for d in targets]
    files = []
    for target in targetDirs:
        _plugins = os.listdir(target)
        for _plugin in _plugins:
            if _plugin.split(".")[0] in plugins:
                files += [f"{target}/{_plugin}"]
    zipFileName = PACKAGE_ARTIFACT
    if len(plugins) == 1:
        zipFileName = plugins[0]
    outPath = f"{ARTIFACTS_DIR}/{zipFileName}_{version}.zip"
    res = subprocess.run(
        ["zip", "-r", outPath] + files,
        check=False,
    )
    if res.returncode:
        print(f"{RED}Failed to zip.{BLACK}")
        return False
    print(f"{GREEN}Package stored to '{outPath}'.{BLACK}")
    return True


def makeDistributionXml(
    plugins: list[str],
    targets: list[str],
    company: str,
    version: str = "0.1.0",
) -> bool:
    """
    Generates a distribution XML file.

    Args:
        plugins (list[str]): Names of plugins to add to installer.
        company (str): Vendor/company name.
        version (str, optional): Version string. Defaults to "0.1.0".

    Returns:
        bool: True on success.
    """
    xml = '<?xml version="1.0" encoding="utf-8" standalone="no"?>\n'
    xml += '<installer-gui-script minSpecVersion="2">\n'
    xml += '<options customize="always" require-scripts="false" '
    xml += 'hostArchitectures="arm64"/>\n'
    xml += '<choices-outline>\n'
    for plugin in plugins:
        for target in targets:
            if not os.path.exists(_pluginPath(plugin, target)):
                continue
            xml += f'  <line choice="{plugin}.{target}" />\n'
    xml += '</choices-outline>\n'
    for plugin in plugins:
        for target in targets:
            if not os.path.exists(_pluginPath(plugin, target)):
                continue
            xml += f'<choice id="{plugin}.{target}" visible="true" '
            xml += f'start_selected="true" title="{plugin} {target}">\n'
            xml += '  <pkg-ref '
            xml += f'id="com.{company}.{plugin}.{target.lower()}.pkg" '
            xml += f'version="{version}" onConclusion="none">\n'
            xml += f"    {plugin}.{target}.pkg\n"
            xml += '  </pkg-ref>\n'
            xml += '</choice>\n'
    xml += '</installer-gui-script>\n'
    with open(f"{PACKAGING_DIR}/dist.xml", "w", encoding="utf-8") as f:
        f.write(xml)
    return True


def makeInstallerPkg(installerId: str, path: str) -> bool:
    """
    Makes an installer package from individual plugin packages.

    Args:
        installerId (str): Apple Developer ID Installer to sign with.
        path (str): Output path.

    Returns:
        bool: True on success.
    """
    print(f"{BLUE}Making installer.{BLACK}")
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
            path,
        ],
        check=False,
        capture_output=True,
    )
    os.chdir(REPO_BASE_DIR)
    print(res.stdout.decode())
    if "warning" in res.stdout.decode():
        print(f"{RED}Warnings in productbuild. Aborting.{BLACK}")
        return False
    if res.returncode:
        print(f"{RED}Failed to make installer.{BLACK}")
        return False
    return True


def makePluginPkgs(
    plugins: list[str],
    targets: list[str],
    company: str,
) -> bool:
    """
    Makes a pkg file for each plugin.

    Args:
        plugins (list[str]): List of names of plugins.
        targets (list[str]): List of target formats (AU, VST3 ect.).
        company (str): Name of vendor/company.

    Returns:
        bool: True on success.
    """
    for plugin in plugins:
        for target in targets:
            if not makePluginPkg(plugin, target, company):
                return False
    return True


def makePluginPkg(plugin: str, target: str, company: str) -> bool:
    """
    Makes a package file for a single plugin.

    Args:
        plugin (str): Name of plugin.
        target (str): Target format.
        company (str): Company/vendor.

    Returns:
        bool: True on success.
    """
    os.chdir(PACKAGING_DIR)
    pluginPath = _pluginPath(plugin, target)
    print(f"{BLUE}Packaging '{plugin}' {target}.{BLACK}")
    if not os.path.exists(pluginPath):
        print(f"{YELLOW}'{pluginPath}' not found. Skipping.{BLACK}")
        return True
    targetPath = f"{plugin}.{target}.pkg"
    installLocation = "/Library/Audio/Plug-Ins/VST3"
    if target == "AU":
        installLocation = "/Library/Audio/Plug-Ins/Components"
    if target == "Standalone":
        installLocation = "/Applications"
    if target == "AAX":
        installLocation = "/Library/Application Support/Avid/Audio/Plug-Ins"
    res = subprocess.run(
        [
            "pkgbuild",
            "--identifier",
            f"com.{company}.{plugin}.{target.lower()}.pkg",
            "--component",
            pluginPath,
            "--install-location",
            installLocation,
            targetPath,
        ],
        check=False,
    )
    os.chdir(REPO_BASE_DIR)
    if res.returncode:
        return False
    return True


def secretsAreValid(secrets: dict) -> bool:
    valid = True
    for s in SECRETS:
        if s not in secrets.keys():
            print(f"{RED}'{s}' is missing.")
            valid = False
    return valid


def verisonIsNewer(new: str, old: str) -> bool:
    try:
        newInts = [int(v) for v in new.split(".")]
        oldInts = [int(v) for v in old.split(".")]
    except ValueError:
        return False
    if len(newInts) != 3:
        return False
    if len(oldInts) != 3:
        return False
    if newInts[0] > oldInts[0]:
        return True
    if newInts[1] > oldInts[1]:
        return True
    if newInts[2] > oldInts[2]:
        return True
    return False


def incrementMinorMonirVersion(old: str) -> str:
    ints = [int(v) for v in old.split(".")]
    version = f"{ints[0]}.{ints[1]}.{ints[2] + 1}"
    print(f"{BLUE}Incremented version: {version}{BLACK}")
    return version


def findVersion(args: dict, secrets: dict) -> str:
    version = ""
    if args["version"]:
        if "version" in secrets and secrets["version"]:
            if not verisonIsNewer(args["version"], secrets["version"]):
                if args["version"] == secrets["version"]:
                    version = incrementMinorMonirVersion(args["version"])
                else:
                    print(f"{RED}New version is lower than last build.{BLACK}")
                    return ""
        version = args["version"]
        secrets["version"] = version
    elif "version" in secrets and secrets["version"]:
        version = incrementMinorMonirVersion(secrets["version"])
    return version


def main(args: dict) -> bool:
    """
    Packages selected plugins for MacOS.

    Args:
        args (dict): CLI arguments.

    Returns:
        bool: True on success.
    """
    # print(f"args: {json.dumps(args, indent=2)}")
    if sys.platform != "darwin":
        print(f"{RED}Packaging only available for MacOS.{BLACK}")
        return False

    os.makedirs(PACKAGING_DIR, exist_ok=True)
    plugins = args["plugins"]
    targets = list(TARGET_EXT_MAP.keys())
    if "targets" in args and args["targets"]:
        targets = args["targets"]
    if not plugins or plugins == ["all"]:
        plugins = readPlugins()
    secrets = loadSecrets()
    for secret in SECRETS:
        if secret in args and args[secret]:
            secrets[secret] = args[secret].replace("/n", "")
    if not secretsAreValid(secrets):
        print(f"{RED}Faild to get credentials.{BLACK}")
        return False
    version = findVersion(args, secrets)
    if not version:
        print(f"{RED}Version is not provided. Aborting.{BLACK}")
        return False
    secrets["version"] = version
    if not storeSecrets(secrets):
        return False
    if "no_sign" not in args or not args["no_sign"]:
        if not sign(plugins, targets, secrets["devId"]):
            return False
    if "zip" in args and args["zip"]:
        if not args["no_notarize"]:
            if not notarizePlugins(
                plugins,
                targets,
                secrets["email"],
                secrets["password"],
                secrets["team"],
            ):
                return False
        if "no_staple" not in args or not args["no_staple"]:
            if not staplePlugins(plugins, targets):
                return False
        if not zipPackage(plugins, targets, version):
            return False
        return True
    if not makePluginPkgs(plugins, targets, secrets["company"]):
        return False
    if not makeDistributionXml(plugins, targets, secrets["company"]):
        return False
    path = f"{ARTIFACTS_DIR}/{PACKAGE_ARTIFACT}_{version}.pkg"
    if len(plugins) == 1:
        path = f"{ARTIFACTS_DIR}/{plugins[0]}_{version}.pkg"
    if not makeInstallerPkg(secrets["installerId"], path):
        return False
    if "no_notarize" not in args or not args["no_notarize"]:
        if not notarize(
            path,
            secrets["email"],
            secrets["password"],
            secrets["team"],
        ):
            return False
    if "no_staple" not in args or not args["no_staple"]:
        if not staple(path):
            return False
    if not validateInstaller(path):
        return False
    return True


def createParser() -> argparse.ArgumentParser:
    """
    Creates an argument parser for packaging CLI features.

    Returns:
        argparse.ArgumentParser: Parser.
    """
    parser = argparse.ArgumentParser(
        description="Sign, notarize, staple and bundle for MacOS. Requires Apple ID, "
        "Application developer ID and Apple Developer team ID. Once set once, "
        "credentials are cached in '.secrets'."
    )
    parser.add_argument(
        "plugins",
        help="Plugins to package. 'all' for all plugins.",
        nargs="*",
    )
    parser.add_argument(
        "--targets",
        help="Plugin formats to target. Defaults to all available targets.",
        choices=TARGET_EXT_MAP.keys(),
    )
    parser.add_argument(
        "--devId",
        help="Application Developer ID Application as exported from Xcode.",
        type=str,
    )
    parser.add_argument("--email", help="Apple ID email address.")
    parser.add_argument(
        "--password",
        help="Password for Apple ID. Must be an 'app-specific password',",
    )
    parser.add_argument(
        "--team",
        help="Team ID. Found in the paranthesis in the Developer ID.",
    )
    parser.add_argument(
        "--installerId",
        help="Developer ID Installer from Apple.",
    )
    parser.add_argument(
        "--no_notarize",
        "--no-notarize",
        action="store_true",
        help="Don't notarize artifacts.",
    )
    parser.add_argument(
        "--no_sign",
        "--no-sign",
        action="store_true",
        help="Don't sign artifacts.",
    )
    parser.add_argument(
        "--no_staple",
        "--no-staple",
        action="store_true",
        help="Don't staple artifacts.",
    )
    parser.add_argument(
        "--zip",
        "-z",
        action="store_true",
        help="Instead of creating an installer, create a zip file with "
        "notarized plugins.",
    )
    parser.add_argument(
        "--company",
        help="Company/vendor of plugins.",
        default="NTfx",
    )
    parser.add_argument(
        "--version",
        help="Version to be used for all plugins and installer.",
    )
    return parser


if __name__ == "__main__":
    sys.exit(not main(createParser().parse_args().__dict__))
