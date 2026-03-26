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
SEPARATOR = "."

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
    outputPath = f"{FILE_DIR}/in/"
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
        lineStyle = "-"
        for i, v in enumerate(x):
            # TODO: DRY
            lineWidth = 1
            if i > 1:
                lineStyle = "--"
            if i > 3:
                lineStyle = "-"
                lineWidth = 1.5
            if i > 5:
                lineStyle = "--"
            if i > 7:
                lineStyle = ":"
            p.semilogx(fAx, v, linestyle=lineStyle, linewidth=lineWidth)
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
        lineStyle = "-"
        for i, v in enumerate(xDb):
            lineWidth = 1.7
            if i > 1:
                lineStyle = "--"
            if i > 3:
                lineStyle = "-"
                lineWidth = 1.2
            if i > 5:
                lineStyle = "--"
            if i > 7:
                lineStyle = ":"
            p.plot(tAx, v, linestyle=lineStyle, linewidth=lineWidth)
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
    p.savefig(filename, dpi=300, bbox_inches="tight")
    p.clf()


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


def plotTestResults(componentName: str, fs):
    # TODO: plot multiple tests of the same component in one image.
    print("Plotting results")
    inFiles = os.listdir(f"{FILE_DIR}/in")
    outFiles = os.listdir(f"{FILE_DIR}/out")
    resultFiles: list[str] = []
    for file in outFiles:
        if file.endswith(f"{SEPARATOR}result.txt"):
            resultFiles += [file]
    exceptedFiles: list[str] = []
    for file in inFiles:
        if file.endswith(f"{SEPARATOR}expected.txt"):
            info = file.split(SEPARATOR)
            if len(info) != 4:
                print(f"Bad filename: {file}")
                continue
            exceptedFiles += [info[0]]

    (
        impPlots,
        linSwPlots,
        syncSwPlots,
        dynPlots,
        impLegends,
        linSwLegends,
        syncSwLegends,
        dynLegends,
    ) = parseFiles(resultFiles, exceptedFiles)
    os.makedirs(f"{FILE_DIR}/img", exist_ok=True)
    if impPlots:
        plotImpulse(
            np.concatenate(impPlots),
            fs,
            f"{FILE_DIR}/img/{componentName}{SEPARATOR}frequency.png",
            impLegends,
        )
    if linSwPlots:
        plotSpectrum(
            np.concatenate(linSwPlots),
            fs,
            f"{FILE_DIR}/img/{componentName}{SEPARATOR}linearSweep.png",
        )
    if syncSwPlots:
        plotSpectrum(
            np.concatenate(syncSwPlots),
            fs,
            f"{FILE_DIR}/img/{componentName}{SEPARATOR}syncSweep.png",
        )
    if dynPlots:
        for i in range(3):
            plotDynamic(
                np.concatenate(dynPlots),
                fs,
                f"{FILE_DIR}/img/{componentName}{SEPARATOR}dynamic{SEPARATOR}{i}.png",
                dynLegends,
                i,
            )


def parseFiles(
    files: list[str],
    exceptedFiles: list[str],
):
    impPlots = []
    linSwPlots = []
    syncSwPlots = []
    dynPlots = []
    impLegends = []
    linSwLegends = []
    syncSwLegends = []
    dynLegends = []
    legends = ["result left", "result right"]
    for file in files:
        info = file.split(SEPARATOR)
        if len(info) != 4:
            print(f"Bad filename: {file}")
            continue
        # print(f"Analyzing component '{info[0]}', test '{info[1]}'.")
        result = readResult(f"{FILE_DIR}/out/" + file)
        if result is None:
            continue
        legends = ["result left", "result right"]
        if info[0] in exceptedFiles:
            excepted = readResult(
                f"{FILE_DIR}/in/"
                + info[0]
                + SEPARATOR
                + info[1]
                + SEPARATOR
                + "expected.txt"
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
        legends = [info[0] + " " + legend for legend in legends]
        if info[1] == "impulse":
            impPlots += result
            impLegends += legends
        elif info[1] == "linearSweep":
            linSwPlots += result
            linSwLegends += legends
        elif info[1] == "syncSweep":
            syncSwPlots += result
            syncSwLegends += legends
        elif info[1] == "dynamic":
            dynPlots += result
            dynLegends += legends
        else:
            print(f"Bad input name: {info[1]}")
    return (
        impPlots,
        linSwPlots,
        syncSwPlots,
        dynPlots,
        impLegends,
        linSwLegends,
        syncSwLegends,
        dynLegends,
    )


def buildTestProg(cppPath: str):
    print("Building test program.")
    os.makedirs(f"{FILE_DIR}/out", exist_ok=True)
    res = sp.run(
        [
            "clang++",
            cppPath,
            "-o",
            f"{FILE_DIR}/out/main",
            f"-I{os.path.abspath(FILE_DIR)}/..",
            f"-I{os.path.abspath(FILE_DIR)}/../lib/gcem/include",
            "--std=c++20",
        ],
        check=False,
    )
    if res.returncode:
        print(f"Build failed: {res.stdout}, {res.stderr}")
        return False
    return True


def runTestProg() -> int:
    print("Running test program.")
    res = sp.run(["./testWrapper/out/main"], check=False)
    return res.returncode


def acceptLatestResult(components: list[str]):
    print(f"Accepting results {components}.")
    files = os.listdir(f"{FILE_DIR}/out")
    filesToCopy: list[str] = []
    for file in files:
        if file.endswith(f"{SEPARATOR}result.txt"):
            info = file.split(SEPARATOR)
            if len(info) != 4:
                print(f"Bad filename: {file}")
                continue
            if info[0] in components:
                filesToCopy += [file]
    for file in filesToCopy:
        storeFile = file.replace("result", "expected")
        print(f"Storing {file} as {storeFile}.")
        with open(f"{FILE_DIR}/out/" + file, "r", encoding="utf8") as x:
            with open(f"{FILE_DIR}/in/" + storeFile, "w", encoding="utf8") as y:
                y.write(x.read())


def clean():
    shutil.rmtree(f"{FILE_DIR}/out")
    # os.removedirs(f"{testWrapperDir}/out")
    return 0


def run():
    parser = argparse.ArgumentParser(
        description="Runs test on NtFx Components."
    )
    subparsers = parser.add_subparsers(dest="task")
    runParser = subparsers.add_parser("run")
    runParser.add_argument(
        "file",
        nargs=1,
        type=str,
        help="Cpp-file to run.",
    )
    # runParser.add_argument(
    #     "--tests",
    #     choices=["impulse", "syncSweep", "linearSweep", "dynamic"],
    #     help="Select specific test inputs to process with component.",
    # )
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
        "components",
        nargs="*",
        default=["all"],
        help="Components to accept results for. 'all' for all previously tested "
        "components.",
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
    os.makedirs(f"{FILE_DIR}/in", exist_ok=True)
    os.makedirs(f"{FILE_DIR}/out", exist_ok=True)
    if args["task"] == "generate":
        return generateTestVectors(t, fs) is None
    elif args["task"] == "run":
        clean()
        if not buildTestProg(args["file"][0]):
            return 1
        returncode = runTestProg()
        plotTestResults(
            os.path.basename(args["file"][0])
            .replace("_test", "")
            .replace(".cpp", ""),
            fs,
        )
        return returncode
    elif args["task"] == "clean":
        return clean()
    elif args["task"] == "approve":
        components = args["components"]
        if "all" in components:
            # TODO: Make this list from files found in 'test', 'in' or 'out'.
            components = [
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
                "rmsSensor",
                "softClip3",
                "softClip5",
            ]
        acceptLatestResult(components)


if __name__ == "__main__":
    sys.exit(run())
