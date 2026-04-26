"""
@file ntPlugin.py
@author Niels Thøgersen (niels.thoegersen@gmail.com)
@brief Genarates test vectors for unittest of NTplugin components and analyzes
and plots results.
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

import os
import subprocess as sp
import sys
import argparse
import shutil
import platform
import numpy as np
from matplotlib import pyplot as p
from scipy import signal as s

SEPARATOR = "."
EXPECTED_DIR = "in"
TMP_DIR = "out"
STIMULI = [
    "impulse",
    "linearSweep",
    "syncSweep",
    "dynamic_alternating",
    "dynamic_matched",
]
FILE_DIR = os.path.dirname(__file__)


def generateImpulse(n: int) -> np.ndarray:
    """
    Generates an impulse stimulus.

    Args:
        n (int): Length of result

    Returns:
        np.ndarray: Impulse.
    """
    x = np.zeros(n)
    x[0] = 1
    return x


def generateBurstSine(
    fs: float,
    t: float,
    f: float = 10e3,
    nBursts: int = 2,
    lowLevel: float = 0.25,
    highLevel: float = 1,
) -> np.ndarray:
    """
    Generates a sine ware a 'f' Hz, who's level steps between 'lowLevel' and
    'highLevel'.

    Args:
        fs (float): Sample rate.
        t (float): Length of result is seconds.
        f (float, optional): Frequency of result. Defaults to 10e3.
        nBursts (int, optional): Number of bursts. Defaults to 2.
        lowLevel (float, optional): Low level between bursts. Defaults to 0.25.
        highLevel (float, optional): Level during burst. Defaults to 1.

    Returns:
        np.ndarray: Generated, bursting sine wave.
    """
    n = int(t * fs)
    nAx = np.array(range(n))
    y = np.sin(2 * np.pi * f * nAx / fs)
    nBlocks = 2 * nBursts + 1
    nPrBlock = int(n / nBlocks)
    iSample = 0
    for iBlock in range(nBlocks):
        if np.fmod(iBlock, 2) == 0:
            y[iSample : (iSample + nPrBlock)] *= lowLevel
        else:
            y[iSample : (iSample + nPrBlock)] *= highLevel
        iSample += nPrBlock
    return y


def generateSyncSweep(
    f_start: float = 10,
    f_stop: float = 24e3,
    fs: float = 48e3,
    k: int = 2,
) -> np.ndarray:
    """
    Generates a synchronized sine sweep.

    Args:
        f_start (float, optional): Starting frequency. Defaults to 10.
        f_stop (float, optional): Ending frequency. Defaults to 24e3.
        fs (float, optional): Sample rate. Defaults to 48e3.
        k (int, optional): Konstant the length is derived from. Defaults to 2.

    Returns:
        np.ndarray: Synchronized chirp.
    """
    n = int(np.ceil(np.log(f_stop / f_start) / f_start * k * fs))
    t_ax = np.array(range(n)) / fs
    x = np.sin(2 * np.pi * f_start * (k / f_start) * np.exp(t_ax * f_start / k))
    if n % 2 != 0:
        x = np.append(x, 0)
    return x


def generateLinearSweep(fs: float, t: float) -> np.ndarray:
    """
    Generates a linear sweep.

    Args:
        fs (float): Sample rate.
        t (float): Length in seconds.

    Returns:
        np.ndarray: Linear sweep.
    """
    n = int(fs * t)
    tAx = np.arange(n) * t / n
    return s.chirp(tAx, 20, t, 20e3)


def _storeMonoTestVectorAsStereo(x: np.ndarray, filename: str):
    with open(filename, "w", encoding="utf8") as f:
        for sample in x:
            f.write(str(sample) + " " + str(sample) + "\n")


def _storeStereoTestVector(x: np.ndarray, filename: str):
    with open(filename, "w", encoding="utf8") as f:
        xL = x[0, :]
        xR = x[1, :]
        for i, _ in enumerate(xL):
            f.write(f"{xL[i]:.16f} {xR[i]:.16f}\n")


def generateTestVectors(
    t: float, fs: float
) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    """
    Generates and stores all the test vectors.

    Args:
        t (float): Length of results.
        fs (float): Sample rate.

    Returns:
        tuple: The generated test vectors.
    """
    os.makedirs(f"{FILE_DIR}/{EXPECTED_DIR}", exist_ok=True)
    os.makedirs(f"{FILE_DIR}/{TMP_DIR}", exist_ok=True)
    n = int(t * fs)
    outputPath = f"{FILE_DIR}/{EXPECTED_DIR}/"
    _impulse = generateImpulse(n)
    impulse = np.vstack((_impulse, _impulse))
    _storeStereoTestVector(impulse, outputPath + "impulse.txt")
    steppedSineL = generateBurstSine(fs, t, 10e3, 2, 0.25, 1)
    steppedSineR = generateBurstSine(fs, t, 10e3, 2, 1, 0.25)
    dynamic_alternating = np.array([steppedSineL, steppedSineR])
    _storeStereoTestVector(
        dynamic_alternating, outputPath + "dynamic_alternating.txt"
    )
    dynamic_matched = np.array([steppedSineL, steppedSineL])
    _storeStereoTestVector(dynamic_matched, outputPath + "dynamic_matched.txt")
    _syncSweep = generateSyncSweep()
    syncSweep = np.array([_syncSweep, _syncSweep])
    _storeStereoTestVector(syncSweep, outputPath + "syncSweep.txt")
    _linearSweep = generateLinearSweep(fs, 1)
    linearSweep = np.array([_linearSweep, _linearSweep])
    _storeStereoTestVector(linearSweep, outputPath + "linearSweep.txt")
    return (impulse, linearSweep, syncSweep, dynamic_alternating)


def _buildTestProg(cppPath: str) -> bool:
    os.makedirs(f"{FILE_DIR}/{TMP_DIR}", exist_ok=True)
    args = ["g++"]
    if platform.system() == "macOS":
        args = ["clang++"]
    args += [
        cppPath,
        "-o",
        f"{FILE_DIR}/{TMP_DIR}/main",
        f"-I{os.path.abspath(FILE_DIR)}/..",
        f"-I{os.path.abspath(FILE_DIR)}/../lib/gcem/include",
        "--std=c++20",
        "-DNTFX_FS=48e3f",
        "-O0",
    ]
    if platform.system() == "Windows":
        args = [
            "cl.exe",
            cppPath,
            f"/Fe{FILE_DIR}{os.sep}{TMP_DIR}{os.sep}main.exe",
            f"/I{os.path.abspath(FILE_DIR)}{os.sep}..{os.sep}",
            f"/I{os.path.abspath(FILE_DIR)}{os.sep}..{os.sep}lib{os.sep}gcem{os.sep}include",
            "/std:c++20",
            "/DNTFX_FS=48e3f",
            "/EHsc",
        ]
    res = sp.run(
        args,
        check=False,
    )
    if res.returncode:
        print(f"Build failed: {res.stdout}, {res.stderr}")
        return False
    return True


def _runTestProg() -> int:
    res = sp.run(
        [f"{FILE_DIR}/{TMP_DIR}/main"],
        check=False,
    )
    return res.returncode


def _findAllTests() -> list[str]:
    paths = []
    allFiles = os.listdir(f"{FILE_DIR}/tests")
    for file in allFiles:
        if file.endswith("_test.cpp"):
            paths += [f"{FILE_DIR}/tests/" + file]
    return paths


def _idxToLineStyle(i: int) -> str:
    if (i // 2) % 2:
        return ":"
    return "-"


def plotImpulse(
    x: np.ndarray,
    fs: float,
    filename: str,
    legends: list | None = None,
):
    """
    Plots the results of an impulse response an frequency amplitude and phase
    responses.

    Args:
        x (np.ndarray): Data to plot.
        fs (float): Sample rate.
        filename (str): Name of output file.
        legends (list | None, optional): Legends of plots. Defaults to None.
    """
    xFft = np.fft.fft(x)
    xAbs = np.abs(xFft)
    xPhase = np.angle(xFft)
    xDb = 20 * np.log10(xAbs + 1e-8)
    plotFrequencyDomain(xDb, fs, filename, legends)
    plotFrequencyDomain(
        xPhase,
        fs,
        filename.replace(f"{SEPARATOR}frequency.png", f"{SEPARATOR}phase.png"),
        legends,
        [-np.pi, np.pi],
        "Phase / radians",
    )


def plotSpectrum(
    x: np.ndarray,
    fs: float,
    filename: str,
):
    """
    Plots a spectrum of a sweep result.

    Args:
        x (np.ndarray): Sweep to plot.
        fs (float): Sample rate.
        filename (str): Path to store at.
    """
    _x = x
    if x.shape[0] < 5:
        if x.shape[0] > 2:
            _x = x[3, :]
        else:
            _x = x[0, :]
    p.specgram(_x, Fs=fs)
    p.grid(True)
    p.xlabel("Time / s")
    p.ylabel("Frequency / Hz")
    p.title(
        os.path.basename(filename).replace(".png", "").replace(SEPARATOR, " ")
        + " spectrum"
    )
    p.savefig(filename, dpi=300, bbox_inches="tight")
    p.clf()


def plotFrequencyDomain(
    x: np.ndarray,
    fs: float,
    filename: str,
    legends: list | None = None,
    ylim: list | None = None,
    ylabel: str | None = None,
):
    """
    Plots signal in frequency domain.

    Args:
        x (np.ndarray): Data to plot.
        fs (float): Sample rate.
        filename (str): Output file name.
        legends (list | None, optional): Legends to add to plot. Defaults to None.
        ylim (list | None, optional): Y axis limits. Defaults to None.
        ylabel (str | None, optional): Lable for Y axis. Defaults to None.
    """
    try:
        n = x.shape[1]
        fAx = np.linspace(0, fs - fs / n, n)
        for i, v in enumerate(x):
            p.semilogx(fAx, v, linestyle=_idxToLineStyle(i))
    except IndexError:
        n = x.shape[0]
        fAx = np.linspace(0, fs - fs / n, n)
        p.semilogx(fAx, x)
    p.xlim([20, 20e3])
    _ylim = [-60, 10]
    if ylim:
        _ylim = ylim
    else:
        if x.min() > -60:
            _ylim[0] = x.min() * 1.1
        if x.max() > 10:
            _ylim[1] = x.max() * 1.1
    p.ylim(_ylim)
    p.grid(True)
    if legends:
        p.legend(legends, loc=(1.04, 0))
    p.xlabel("Frequency / Hz")
    p.ylabel("Amplitude / dB")
    if ylabel:
        p.ylabel(ylabel)
    p.title(
        os.path.basename(filename).replace(".png", "").replace(SEPARATOR, " ")
        + " response"
    )
    p.savefig(filename, dpi=300, bbox_inches="tight")
    p.clf()


def plotDynamic(
    x: np.ndarray,
    fs: float,
    filename: str,
    legends: list | None = None,
    zoom: int = 1,
):
    """
    Plots a dynamic response.

    Args:
        x (np.ndarray): Data to plot.
        fs (float): Sample rate.
        filename (str): Output file name.
        legends (list | None, optional): Legends of plots. Defaults to None.
        zoom (int, optional): Zoom level. 1 for whole plot. Defaults to 1.
    """
    xDb = 20 * np.log10(np.abs(x) + 1e-8)
    try:
        nSamples = xDb.shape[1]
        tAx = np.array(range(nSamples)) / fs
        for i, v in enumerate(xDb):
            p.plot(tAx, v, linestyle=_idxToLineStyle(i))
    except IndexError:
        nSamples = xDb.shape[0]
        tAx = np.array(range(nSamples)) / fs
        p.plot(tAx, xDb)
    if zoom == 1:
        p.xlim([0.9 * nSamples / (fs * 5), 3 * nSamples / (fs * 5)])
    p.ylim([-14, 2])
    if zoom == 2:
        p.xlim([0.9 * 2 * nSamples / (fs * 5), 1.1 * 2 * nSamples / (fs * 5)])
        p.ylim([-1, 1])
    if legends:
        p.legend(legends, loc=(1.04, 0))
    p.grid(True)
    p.xlabel("Time / s")
    p.ylabel("Amplitude / dB")
    p.title(
        os.path.basename(filename).replace(".png", "").replace(SEPARATOR, " ")
        + " response"
    )
    p.savefig(filename, dpi=300, bbox_inches="tight")
    p.clf()


def plotSweeps(
    results: list[np.ndarray],
    legends: dict[str, list[str]],
    testFileName: str,
    fs: float,
    objectName: str,
):
    """
    Plots a spectrum for each sweep in 'results'. File name will be genarated
    forom legends and objectName.

    Args:
        results (dict[str, list[np.ndarray]]): Data to plot.
        legends (dict[str, list[str]]): Legends to add.
        testFileName (str): Name of the test program the data came from.
        fs (float): Sample rate.
        objectName (str): Name of the specific test.
    """
    for i, sweep in enumerate(results):
        plotSpectrum(
            sweep,
            fs,
            f"{FILE_DIR}/img/{testFileName}{SEPARATOR}"
            + f"{legends[objectName][i*2].split(" ")[0]}{SEPARATOR}"
            + objectName
            + ".png",
        )


def _parseFiles(files: list[str], exceptedFiles: list[str]):
    results_dict = {
        "impulse": [],
        "linearSweep": [],
        "syncSweep": [],
        "dynamic_alternating": [],
        "dynamic_matched": [],
    }
    legends_dict = {
        "impulse": [],
        "linearSweep": [],
        "syncSweep": [],
        "dynamic_alternating": [],
        "dynamic_matched": [],
    }
    legends = ["result left", "result right"]
    for file in files:
        info = file.split(SEPARATOR)
        if len(info) != 5:
            print(f"Bad filename: '{file}'.")
            continue
        result = _readResult(f"{FILE_DIR}/{TMP_DIR}/" + file)
        if result is None:
            print(f"'{file}' not found.")
            continue
        legends = ["result left", "result right"]
        if info[0] + SEPARATOR + info[1] in exceptedFiles:
            excepted = _readResult(
                f"{FILE_DIR}/{EXPECTED_DIR}/{file.replace("result", "expected")}"
            )
            if excepted is not None:
                legends = [
                    "expected left",
                    "expected right",
                    "result left",
                    "reslut right",
                ]
                result = [excepted, result]
        else:
            result = [result]
        legends = [info[1] + " " + legend for legend in legends]
        stimulus = info[2]
        if stimulus not in STIMULI:
            print(f"Unknown stimulus: {stimulus}")
            continue
        results_dict[stimulus] += result
        legends_dict[stimulus] += legends
    return results_dict, legends_dict


def _readResult(path: str) -> np.ndarray | None:
    if not os.path.exists(path):
        print(f"Bad path: {path}")
        return None
    with open(path, "r", encoding="utf8") as f:
        lines = f.readlines()
    y = np.zeros([2, len(lines)])
    for i, line in enumerate(lines):
        vs = line.split(" ")
        y[0, i] = vs[0]
        y[1, i] = vs[1]
    return y


def _plotResults(
    results: dict[str, list[np.ndarray]],
    legends: dict[str, list[str]],
    testFileName: str,
    fs: float,
):
    os.makedirs(f"{FILE_DIR}/img", exist_ok=True)
    if "impulse" in results and results["impulse"]:
        plotImpulse(
            np.concatenate(results["impulse"]),
            fs,
            f"{FILE_DIR}/img/{testFileName}{SEPARATOR}frequency.png",
            legends["impulse"],
        )
    if "linearSweep" in results and results["linearSweep"]:
        plotSweeps(
            results["linearSweep"], legends, testFileName, fs, "linearSweep"
        )
    if "syncSweep" in results and results["syncSweep"]:
        plotSweeps(results["syncSweep"], legends, testFileName, fs, "syncSweep")
    _plotDynamic(results, "dynamic_alternating", testFileName, legends, fs)
    _plotDynamic(results, "dynamic_matched", testFileName, legends, fs)


def _plotDynamic(
    results: dict[str, list[np.ndarray]],
    name: str,
    testFileName: str,
    legends: dict[str, list[str]],
    fs: float,
):
    if name in results and results[name]:
        for i in range(3):
            plotDynamic(
                np.concatenate(results[name]),
                fs,
                f"{FILE_DIR}/img/{testFileName}{SEPARATOR}{name}{SEPARATOR}{i}.png",
                legends[name],
                i,
            )


def _readAndPlotTestResults(testFileName: str, fs: float):
    resultFiles: list[str] = []
    outFiles = os.listdir(f"{FILE_DIR}/{TMP_DIR}")
    for file in outFiles:
        if file.startswith(testFileName) and file.endswith(
            f"{SEPARATOR}result.txt"
        ):
            resultFiles += [file]
    expectedFiles: list[str] = []
    inFiles = os.listdir(f"{FILE_DIR}/{EXPECTED_DIR}")
    for file in inFiles:
        if file.endswith(f"{SEPARATOR}expected.txt"):
            info = file.split(SEPARATOR)
            if len(info) != 5:
                print(f"Bad filename: {file}")
                continue
            expectedFiles += [info[0] + SEPARATOR + info[1]]
    results, legends = _parseFiles(resultFiles, expectedFiles)
    _plotResults(results, legends, testFileName, fs)


def acceptLatestResult(files: list[str], objects: list[str]) -> bool:
    """
    Approves the latest results that match a specific test program file and
    objects therein. Both args accept ["all"], which will match all files or all
    objects within a file.

    Args:
        files (list[str]): Test program files. Typically named after class under test.
        objects (list[str]): Names of objects under test.

    Returns:
        bool: True on success.
    """
    resultFiles = os.listdir(f"{FILE_DIR}/{TMP_DIR}")
    filesToCopy: list[str] = []
    for file in resultFiles:
        if file.endswith(f"{SEPARATOR}result.txt"):
            info = file.split(SEPARATOR)
            if len(info) != 5:
                print(f"Bad filename: {file}")
                continue
            if files and files != ["all"] and info[0] not in files:
                continue
            if objects and objects != ["all"] and info[1] not in objects:
                continue
            filesToCopy += [file]
    if not filesToCopy:
        print("No result files found.")
        return False
    for file in filesToCopy:
        storeFile = file.replace("result", "expected")
        with open(f"{FILE_DIR}/{TMP_DIR}/" + file, "r", encoding="utf8") as x:
            with open(
                f"{FILE_DIR}/{EXPECTED_DIR}/" + storeFile, "w", encoding="utf8"
            ) as y:
                y.write(x.read())
    print(f"Copied {len(filesToCopy)} files with expected results to `in`.")
    return True


def clean() -> bool:
    """
    Cleans all previous test outputs.

    Returns:
        bool: True on success.
    """
    shutil.rmtree(f"{FILE_DIR}/{TMP_DIR}")
    return True


def runTests(path: str, fs: float) -> bool:
    """
    Runs tests for a specific test program. Build the program, runs it and
    collects results.

    Args:
        path (str): _description_
        fs (float): _description_

    Returns:
        bool: _description_
    """
    testFileName = (
        os.path.basename(path).replace("_test", "").replace(".cpp", "")
    )
    print(f"Testing '{testFileName}'")
    if not _buildTestProg(path):
        return False
    returncode = _runTestProg()
    _readAndPlotTestResults(testFileName, fs)
    return returncode == 0


def _readAggregateResults() -> dict[str, int]:
    results = {}
    path = f"{FILE_DIR}/{TMP_DIR}/results.txt"
    if not os.path.exists(path):
        return {}
    with open(path, encoding="utf8") as f:
        for line in f.readlines():
            vals = line.split(",")
            if len(vals) != 4:
                print("Bad line in results.")
                continue
            if vals[0] in results:
                print(f"Multiple results for '{vals[0]}'. Using latest.")
            results[vals[0]] = {
                "nTests": vals[1],
                "nObjects": vals[2],
                "nSuccessful": vals[3],
            }
    sums = {
        "nFiles": 0,
        "nTests": 0,
        "nObjects": 0,
        "nSuccessful": 0,
    }
    for file in results.values():
        assert isinstance(file, dict)
        sums["nFiles"] += 1
        for test, val in file.items():
            sums[test] += int(val)
    return sums


def run(args: dict):
    """
    Runs test application.

    Args:
        args (dict): Used to get 'files' and 'fs' args from command line.

    Returns:
        bool: True on success.
    """
    os.makedirs(f"{FILE_DIR}/{EXPECTED_DIR}", exist_ok=True)
    os.makedirs(f"{FILE_DIR}/{TMP_DIR}", exist_ok=True)
    clean()
    success = True
    files = args["files"]
    if not files or files == ["all"]:
        files = _findAllTests()
    for file in files:
        if not file.endswith(".cpp"):
            if not file.endswith("_test"):
                file = f"{file}_test"
            file = f"{FILE_DIR}/tests/{file}.cpp"
        if not os.path.exists(file):
            print(f"File '{file}' not found. Skipping test.")
            continue
        success &= runTests(file, args["fs"])
        print()
    results = _readAggregateResults()
    if not results:
        return True
    print(
        f"Ran {results["nTests"]} tests on {results["nObjects"]} objects "
        f"in {results["nFiles"]} test files. "
        f"{results["nSuccessful"]} succeeded. "
        f"({100.0 * results["nSuccessful"] /  results["nTests"]:.2f} %)"
    )
    if success:
        print("\033[32m", end="")
        st = "PASSED"
    else:
        print("\033[31m", end="")
        st = "FAILED"
    print(f"TESTS {st}")
    print("\033[0m", end="")
    return success


def createParser() -> argparse.ArgumentParser:
    """
    Creates an argument parser for test program.

    Returns:
        argparse.ArgumentParser: New argument parser.
    """
    parser = argparse.ArgumentParser(
        description="Runs test on NTfx Components. Usage: 'test.py run "
        "[test file name(s)]'. '_test.cpp' can be omitted."
    )
    subparsers = parser.add_subparsers(dest="test_task")
    runParser = subparsers.add_parser(
        "run",
        help="Run tests. A list of names can be added to select specific tests.",
    )
    runParser.add_argument(
        "files",
        nargs="*",
        type=str,
        help="Cpp-files to run. If 'all' or nothing, dir 'test' will be searched"
        " for files ending with '_test.cpp' and those will be used.",
    )
    generateParser = subparsers.add_parser(
        "generate", help="Generate needed input files."
    )
    subparsers.add_parser(
        "clean", help="Cleans all outputs from previous tests."
    )
    approveParser = subparsers.add_parser(
        "approve", help="Set selected results in output dir as new expected."
    )
    approveParser.add_argument(
        "file",
        nargs=1,
        help="File containing tests to approve. '_test' and '.cpp' are ignored "
        "and can be omitted",
    )
    approveParser.add_argument(
        "objects",
        nargs="+",
        help="Objects to accept results for.",
    )
    parser.add_argument(
        "--fs",
        type=float,
        default=48e3,
        help="Sample rate to test at.",
    )
    generateParser.add_argument(
        "--duration",
        "-t",
        type=float,
        default=0.1,
        help="Duration of test in seconds. Defaults to 0.1",
    )
    return parser


def main(args: dict) -> bool:
    """
    Main function for test program.

    Args:
        args (dict): Arguments as passed in from the command line.

    Returns:
        bool: True on success.
    """
    fs = args["fs"]
    if args["test_task"] == "generate":
        t = args["duration"]
        return generateTestVectors(t, fs) is not None
    if args["test_task"] == "run":
        return run(args)
    if args["test_task"] == "clean":
        return clean()
    if args["test_task"] == "approve":
        return acceptLatestResult(args["file"], args["objects"])
    print(f"Unknown command: '{args['task']}'.")
    return False


if __name__ == "__main__":
    sys.exit(not main(createParser().parse_args().__dict__))
