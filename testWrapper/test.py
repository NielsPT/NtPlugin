# Copyright (C) 2026 Niels Thøgersen, NTlyd
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU Affero General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
# You are free to download, build and use this code for commercial
# purposes. Just don't resell it or a build of it, modified or otherwise.

"""
Genarates test vectors for unittest of NTplugin components and analyzes and plots
results.
"""

import os
import subprocess as sp
import sys
import argparse
import shutil
import numpy as np
from matplotlib import pyplot as p
from scipy import signal as s

# TODO: Make the files comma separated.
# TODO: CSV files.
SEPARATOR = "."
EXPECTED_DIR = "in"
TMP_DIR = "out"

FILE_DIR = os.path.dirname(__file__)


def generateImpulse(n: int) -> np.ndarray:
    x = np.zeros(n)
    x[0] = 1
    return x


def generateSteppedSine(
    fs: float,
    t: float,
    f: float = 10e3,
    nBursts: int = 2,
    lowLevel: float = 0.25,
    highLevel: float = 1,
):
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
    f_start: float = 10, f_stop: float = 24e3, fs: float = 48e3, k: int = 2
) -> np.ndarray:
    n = int(np.ceil(np.log(f_stop / f_start) / f_start * k * fs))
    t_ax = np.array(range(n)) / fs
    x = np.sin(2 * np.pi * f_start * (k / f_start) * np.exp(t_ax * f_start / k))
    if n % 2 != 0:
        x = np.append(x, 0)
    return x


def generateLinearSweep(fs: float, t: float) -> np.ndarray:
    n = int(fs * t)
    tAx = np.arange(n) * t / n
    return s.chirp(
        tAx,
        20,
        t,
        20e3,
    )


def storeMonoTestVectorAsStereo(x: np.ndarray, filename: str):
    with open(filename, "w", encoding="utf8") as f:
        for sample in x:
            f.write(str(sample) + " " + str(sample) + "\n")


def storeStereoTestVector(x: np.ndarray, filename: str):
    with open(filename, "w", encoding="utf8") as f:
        xL = x[0, :]
        xR = x[1, :]
        for i, _ in enumerate(xL):
            f.write(str(xL[i]) + " " + str(xR[i]) + "\n")


def generateTestVectors(t: float, fs: float):
    n = int(t * fs)
    outputPath = f"{FILE_DIR}/{EXPECTED_DIR}/"
    _impulse = generateImpulse(n)
    impulse = np.vstack((_impulse, _impulse))
    storeStereoTestVector(impulse, outputPath + "impulse.txt")
    steppedSineL = generateSteppedSine(fs, t, 10e3, 2, 0.25, 1)
    steppedSineR = generateSteppedSine(fs, t, 10e3, 2, 1, 0.25)
    dynamic = np.array([steppedSineL, steppedSineR])
    storeStereoTestVector(dynamic, outputPath + "dynamic.txt")
    _syncSweep = generateSyncSweep()
    syncSweep = np.array([_syncSweep, _syncSweep])
    storeStereoTestVector(syncSweep, outputPath + "syncSweep.txt")
    _linearSweep = generateLinearSweep(fs, 1)
    linearSweep = np.array([_linearSweep, _linearSweep])
    storeStereoTestVector(linearSweep, outputPath + "linearSweep.txt")
    return (impulse, linearSweep, syncSweep, dynamic)


def buildTestProg(cppPath: str):
    os.makedirs(f"{FILE_DIR}/{TMP_DIR}", exist_ok=True)
    res = sp.run(
        [
            "clang++",
            cppPath,
            "-o",
            f"{FILE_DIR}/{TMP_DIR}/main",
            f"-I{os.path.abspath(FILE_DIR)}/..",
            f"-I{os.path.abspath(FILE_DIR)}/../lib/gcem/include",
            "-I/opt/homebrew/include",
            "--std=c++20",
            "-DNTFX_FS=48e3f",
        ],
        check=False,
    )
    if res.returncode:
        print(f"Build failed: {res.stdout}, {res.stderr}")
        return False
    return True


def runTestProg() -> int:
    # TODO: PWD
    res = sp.run(
        [f"{FILE_DIR}/../testWrapper/{TMP_DIR}/main"],
        check=False,
        # capture_output=True,
    )
    # print(f"{res.stdout.decode()}")
    return res.returncode


def findAllTests() -> list[str]:
    paths = []
    allFiles = os.listdir(f"{FILE_DIR}/../test")
    for file in allFiles:
        if file.endswith("_test.cpp"):
            paths += ["test/" + file]
    return paths


def idxToLineStyle(i: int) -> str:
    if (i // 2) % 2:
        return ":"
    return "-"


def plotImpulse(
    x: np.ndarray,
    fs: float,
    filename: str,
    legends: list | None = None,
):
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
    try:
        n = x.shape[1]
        fAx = np.linspace(0, fs - fs / n, n)
        for i, v in enumerate(x):
            p.semilogx(fAx, v, linestyle=idxToLineStyle(i))
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
    xDb = 20 * np.log10(np.abs(x) + 1e-8)
    try:
        nSamples = xDb.shape[1]
        tAx = np.array(range(nSamples)) / fs
        for i, v in enumerate(xDb):
            p.plot(tAx, v, linestyle=idxToLineStyle(i))
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
    results: dict[str, list[np.ndarray]],
    legends: dict[str, list[str]],
    testFileName: str,
    fs: float,
    objectName: str,
):
    for i, sweep in enumerate(results[objectName]):
        plotSpectrum(
            sweep,
            fs,
            f"{FILE_DIR}/img/{testFileName}{SEPARATOR}"
            + f"{legends[objectName][i*2].split(" ")[0]}{SEPARATOR}"
            + objectName
            + ".png",
        )


def parseFiles(
    files: list[str],
    exceptedFiles: list[str],
):
    results_dict = {
        "impulse": [],
        "linearSweep": [],
        "syncSweep": [],
        "dynamic": [],
    }
    legends_dict = {
        "impulse": [],
        "linearSweep": [],
        "syncSweep": [],
        "dynamic": [],
    }
    legends = ["result left", "result right"]
    for file in files:
        info = file.split(SEPARATOR)
        if len(info) != 5:
            print(f"Bad filename: '{file}'.")
            continue
        result = readResult(f"{FILE_DIR}/{TMP_DIR}/" + file)
        if result is None:
            print(f"'{file}' not found.")
            continue
        legends = ["result left", "result right"]
        if info[0] + SEPARATOR + info[1] in exceptedFiles:
            excepted = readResult(
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
        # TODO: loop
        # TODO: 2? magic number. How long did that take to find? Come on.
        try:
            if info[2] == "impulse":
                results_dict["impulse"] += result
                legends_dict["impulse"] += legends
            elif info[2] == "linearSweep":
                results_dict["linearSweep"] += result
                legends_dict["linearSweep"] += legends
            elif info[2] == "syncSweep":
                results_dict["syncSweep"] += result
                legends_dict["syncSweep"] += legends
            elif info[2] == "dynamic":
                results_dict["dynamic"] += result
                legends_dict["dynamic"] += legends
        except ValueError as e:
            print(f"Unexpected result file found: {e}")
    return results_dict, legends_dict


def readResult(path: str) -> np.ndarray | None:
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


def readAndPlotTestResults(testFileName: str, fs: float):
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
    results, legends = parseFiles(resultFiles, expectedFiles)
    plotResults(results, legends, testFileName, fs)


def plotResults(
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
        plotSweeps(results, legends, testFileName, fs, "linearSweep")
    if "syncSweep" in results and results["syncSweep"]:
        plotSweeps(results, legends, testFileName, fs, "syncSweep")
    if "dynamic" in results and results["dynamic"]:
        for i in range(3):
            plotDynamic(
                np.concatenate(results["dynamic"]),
                fs,
                f"{FILE_DIR}/img/{testFileName}{SEPARATOR}dynamic{SEPARATOR}{i}.png",
                legends["dynamic"],
                i,
            )


def acceptLatestResult(objects: list[str]) -> bool:
    files = os.listdir(f"{FILE_DIR}/{TMP_DIR}")
    filesToCopy: list[str] = []
    for file in files:
        if file.endswith(f"{SEPARATOR}result.txt"):
            info = file.split(SEPARATOR)
            if len(info) != 5:
                print(f"Bad filename: {file}")
                continue
            if info[1] in objects:
                filesToCopy += [file]
    if not filesToCopy:
        return False
    for file in filesToCopy:
        storeFile = file.replace("result", "expected")
        # print(f"Storing {file} as {storeFile}.")
        with open(f"{FILE_DIR}/{TMP_DIR}/" + file, "r", encoding="utf8") as x:
            with open(
                f"{FILE_DIR}/{EXPECTED_DIR}/" + storeFile, "w", encoding="utf8"
            ) as y:
                y.write(x.read())
    print(f"Copied {len(filesToCopy)} files with expected results to `in`.")
    return True


def clean() -> bool:
    shutil.rmtree(f"{FILE_DIR}/{TMP_DIR}")
    # os.removedirs(f"{testWrapperDir}/{TMP_DIR}")
    return True


def runTests(path: str, fs: float) -> bool:
    testFileName = (
        os.path.basename(path).replace("_test", "").replace(".cpp", "")
    )
    print(f"Testing '{testFileName}'")
    if not buildTestProg(path):
        return False
    returncode = runTestProg()
    readAndPlotTestResults(testFileName, fs)
    return returncode == 0


def readAggregateResults() -> dict[str, int]:
    results = {}
    with open(f"{FILE_DIR}/{TMP_DIR}/results.txt", encoding="utf8") as f:
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


def run() -> bool:
    parser = argparse.ArgumentParser(
        description="Runs test on NTfx Components. Usage: 'test.py run "
        "[test file name(s)]'. '_test.cpp' can be omitted."
    )
    subparsers = parser.add_subparsers(dest="task")
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
    cleanParser = subparsers.add_parser(
        "clean", help="Cleans all outputs from previous tests."
    )
    approveParser = subparsers.add_parser(
        "approve", help="Set selected results in output dir as new expected."
    )
    approveParser.add_argument(
        "objects",
        nargs="*",
        default=["all"],
        help="Objects to accept results for. 'all' for all previously tested "
        "objects.",
    )
    parser.add_argument(
        "--fs",
        type=float,
        default=48e3,
        help="Sample rate to test at.",
    )
    parser.add_argument(
        "--duration",
        "-t",
        type=float,
        default=0.1,
        help="Duration of test in seconds. Defaults to 0.1",
    )
    args = parser.parse_args().__dict__
    fs = args["fs"]
    t = args["duration"]
    os.makedirs(f"{FILE_DIR}/{EXPECTED_DIR}", exist_ok=True)
    os.makedirs(f"{FILE_DIR}/{TMP_DIR}", exist_ok=True)
    if args["task"] == "generate":
        return generateTestVectors(t, fs) is not None
    if args["task"] == "run":
        success = True
        files = args["files"]
        if not files or files == ["all"]:
            clean()
            files = findAllTests()
        for file in files:
            if not file.endswith(".cpp"):
                if not file.endswith("_test"):
                    file = f"{file}_test"
                file = f"test/{file}.cpp"
            success &= runTests(file, fs)
            print()
        results = readAggregateResults()
        print(
            f"Ran {results["nTests"]} tests on {results["nObjects"]} objects "
            f"in {results["nFiles"]} test files. "
            f"{results["nSuccessful"]} succeeded. "
            f"({100.0 * results["nSuccessful"] /  results["nTests"]} %)"
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
    if args["task"] == "clean":
        return clean()
    if args["task"] == "approve":
        objects = args["objects"]
        if "all" in objects:
            # TODO: Make this list from files found in 'test', 'in' or 'out'.
            objects = [
                "bqBell",
                "bqHfp",
                "bqHiShelf",
                "bqLpf",
                "bqLoShelf",
                "firstOrderHpf",
                "firstOrderLpf",
                "firstOrderLpfWithZero",
                "peakSensor",
                "peakHoldSensor",
                "peakDbSc",
                "peakDbScLink",
                "rmsSensor",
                "softClip3",
                "softClip5",
            ]
        return acceptLatestResult(objects)
    print(f"Unknown command: '{args['task']}'.")
    return False


if __name__ == "__main__":
    sys.exit(not run())
