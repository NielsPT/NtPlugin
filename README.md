# NtPlugin

A framework for fast and easy audio plugin development.

## Features

## Installation

- Clone with `--recurse-submodules´.
- Install Cmake. The brew-version does not work, so if you're on Mac, download it from
  the website.
- Install Visual Studio (Windows) or XCode (Mac).

## Usage

- Write your plugin and store it as `plugins/[name of your plugin].h`.
- Use the following commands to configure and build your project.

```sh
cmake -B build -S JuceWrapper -DNTFX_PLUGIN=[name of your plugin]
cmake --build build
```

- Have fun coding.
