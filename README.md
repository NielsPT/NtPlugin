# NtPlugin

A framework for fast and easy audio plugin development. Uses JUCE under the hood.
Since plugins are written in platform-agnostic code, targeting more platforms in the
future is possible, and the user doesn't need to learn the JUCE framework to write
cross-platform audio plugins.

## Features

- Write audio plugins in plain C++ without the need for any JUCE-specific code.
- Automatic layout of UI including metering.
- Oversampling and UI scaling available by default.
- Possibility for wrapping plugins for other targets.

## Installation

- Clone with `--recurse-submodules´.
- Install Cmake. The brew-version does not work, so if you're on Mac, download it from
  the website.
- Install Visual Studio (Windows) or XCode (Mac). <!-- TODO: Linux. -->

## Usage

- Create your plugin as `plugins/[name of your plugin].h`.
- Write a class that inherits from NtFx::Plugin. The name of your class must be
  the same as the file name.
- The base class requires you to implement
  the methods `processSample`, `updateCoeffs` and `reset`. `processSample` runs for every
  sample, `updateCoeffs` is called every time a parameter changes and `reset`
  (re)initializes the plugin.
- The base class contains 3 empty vectors, that the user can add parameters to in the
  constructor of the plugin.
  `primaryKnobs` contains specifications for the main knobs, which are laid out in a grid
  automatically, `secondaryKnobs` can be used to add a single row of smaller knobs below
  main grid for fine tuning or utility controls.
  `toggles` contain the boolean parameters.
- Similarly, the base class contain a spec for the UI named `guiSpec`, which can be  
  modified for customization of the UI.

- Use the following commands to configure and build your project:

```sh
cmake -B build -S JuceWrapper -DNTFX_PLUGIN=[name of your plugin]
cmake --build build
```

Have fun coding.
