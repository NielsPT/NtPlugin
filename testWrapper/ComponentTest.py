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

import numpy as np
from matplotlib import pyplot as p
import os
import subprocess as sp


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
    outputPath = "testWrapper/in/"
    _impulse = generateImpulse(n)
    impulse = np.vstack((_impulse, _impulse))
    if not os.path.exists(outputPath + "impulse.txt"):
        storeStereoTestVector(impulse, outputPath + "impulse.txt")
    steppedSineL = generateSteppedSine(fs, t, 10e3, 2, 0.25, 1)
    steppedSineR = generateSteppedSine(fs, t, 10e3, 2, 1, 0.25)
    dynamicSine = np.array([steppedSineL, steppedSineR])
    if not os.path.exists(outputPath + "dynamicSine.txt"):
        storeStereoTestVector(dynamicSine, outputPath + "dynamicSine.txt")
    _syncSweep = generateSyncSweep()
    syncSweep = np.concatenate((_syncSweep, _syncSweep))
    if not os.path.exists(outputPath + "syncSweep.txt"):
        storeStereoTestVector(syncSweep, outputPath + "syncSweep.txt")
    return (impulse, syncSweep, dynamicSine)


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
        filename.replace(".png", "_phase.png"),
        legends,
        [-np.pi, np.pi],
        "Phase / radians",
    )


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
        for v in x:
            p.semilogx(fAx, v)
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
        p.legend(legends)
    p.xlabel("Frequency / Hz")
    p.ylabel("Amplitude / dB")
    if ylabel:
        p.ylabel(ylabel)
    p.savefig(filename)
    p.clf()


def plotDynamic(
    x: np.ndarray,
    fs: float,
    filename: str,
    legends: list | None = None,
    zoom: bool = False,
):
    _x = 20 * np.log10(np.abs(x) + 1e-8)
    try:
        nSamples = _x.shape[1]
        tAx = np.array(range(nSamples)) / fs
        for v in _x:
            p.plot(tAx, v)
    except IndexError:
        nSamples = _x.shape[0]
        tAx = np.array(range(nSamples)) / fs
        p.plot(tAx, _x)
    p.xlim([0.9 * nSamples / (fs * 5), 3 * nSamples / (fs * 5)])
    p.ylim([-14, 2])
    if zoom:
        p.xlim([0.9 * 2 * nSamples / (fs * 5), 1.1 * 2 * nSamples / (fs * 5)])
        p.ylim([-1, 1])
    if legends:
        p.legend(legends)
    p.grid(True)
    p.xlabel("Time / s")
    p.ylabel("Amplitude / dB")
    p.savefig(filename)
    p.clf()


def readResult(path: str) -> np.ndarray:
    with open(path, "r", encoding="utf8") as f:
        lines = f.readlines()
    y = np.zeros([2, len(lines)])
    for i, line in enumerate(lines):
        vs = line.split(" ")
        y[0, i] = vs[0]
        y[1, i] = vs[1]
    return y


def analyzeTestResults(impulse, syncSweep, steppedSine, fs):

    outFiles = os.listdir("testWrapper/out")
    resultFiles: list[str] = []
    for file in outFiles:
        if file.endswith("_result.txt"):
            resultFiles += [file]
    for file in resultFiles:
        info = file.split("_")
        if len(info) != 3:
            print(f"Bad filename: {file}")
            continue
        print(f"Analyzing component '{info[0]}', test '{info[1]}'.")
        result = readResult("testWrapper/out/" + file)
        if info[1] == "impulse":
            plotImpulse(
                np.concatenate((impulse, result)),
                fs,
                "testWrapper/out/" + info[0] + "_frequency.png",
            )
        elif info[1] == "syncSweep":
            print("'syncSweep' not implemented.")
            # TODO: Sync sweep analysis
        elif info[1] == "dynamicSine":
            plotDynamic(
                np.concatenate((steppedSine, result)),
                fs,
                "testWrapper/out/" + info[0] + "_dynamic.png",
            )
        else:
            print(f"Bad input name: {info[1]}")


def buildTestProg():
    print("Building test program.")
    res = sp.run(
        [
            "clang++",
            "testWrapper/test.cpp",
            "-o",
            "testWrapper/out/main",
            f"-I{os.path.abspath(os.getcwd())}",
            f"-I{os.path.abspath(os.getcwd())}/lib/gcem/include",
            "--std=c++20",
        ],
        check=False,
    )
    if res.returncode:
        print(f"Build failed: {res.stdout}, {res.stderr}")
        return False
    return True


def runTestProg():
    sp.run(["./testWrapper/out/main"], check=False)


def run():
    fs = 48e3
    t = 0.1
    impulse, syncSweep, steppedSine = generateTestVectors(t, fs)
    if not buildTestProg():
        return
    runTestProg()
    analyzeTestResults(impulse, syncSweep, steppedSine, fs)


if __name__ == "__main__":
    run()
