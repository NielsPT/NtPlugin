# NtPlugin

A framework for fast and easy audio plugin development.

## Target audience

The target audience are hobbyists, students and professionals who needs to try
out an idea quickly, without having to build a custom GUI for it. A student or
hobbyist will be able to focus on the algorithms and not all the details of
using JUCE or another framework.

## Features

- Write audio plugins in plain C++ without the need for knowledge of the JUCE
  framework.
- Automatic layout of UI including metering and UI scaling. Just list your
parametes, and the framework takes care of the rest.
<!-- TODO: - Oversampling available by default. -->
- Possibility for wrapping plugins for other targets.

## Getting started

### Install needed software

- Install [Cmake](https://cmake.org/download/). The brew-version does not work,
  so if you're on Mac, download it from the website.
- Install [Visual Studio](https://visualstudio.microsoft.com/downloads/)
  (Windows) or [XCode](https://developer.apple.com/xcode/) (Mac).
  <!-- TODO: Linux. -->
- Clone with `--recurse-submodules´ flag. This will include the JUCE framework.

### Try it out

- To test the install, type

```sh
cmake -B build -S JuceWrapper -DNTFX_PLUGIN=gainExample
cmake --build build
```

in the terminal. This should build the simple gain knob example plugin.

## Usage

- Create your plugin as `plugins/[name of your plugin].h` in the project
  directory.
- Write a class that inherits from `NtFx::NtPlugin`. The name of your class must
  be the same as the file name.
- The base class requires you to implement the methods `processSample`,
  `updateCoeffs` and `reset`. `processSample` runs for every sample,
  `updateCoeffs` is called every time a parameter changes and `reset`
  (re)initializes the plugin and sets the samplerate. `reset` should always call
  `updateCoeffs`.
- The base class contains 3 empty vectors, that the user can add parameters to
  in the constructor of the plugin. `primaryKnobs` contains specifications for
  the main knobs, which are laid out in a grid automatically, `secondaryKnobs`
  can be used to add a single row of smaller knobs below main gird for fine
  tuning or utility controls. `toggles` contain the boolean parameters. The
  constructor should call `updateDefaults` before returning.
- Similarly, the base class contain a spec for the UI named `guiSpec`, which can
  be modified for customization of the UI.
- Use the following commands to configure and build your project:

```sh
cmake -B build -S JuceWrapper -DNTFX_PLUGIN=[name of your plugin]
cmake --build build
```

- Once the project is configured, only the build-command is needed for
  incremental builds.

Have fun coding.

### Caveats

As with all code projects, there are secrets to know.

- If you want to make AXX plugins for Pro Tools, you need to get the AAX SDK
  from avid and put it in the JuceWrapper directory.
- Since Cmake for Mac doesn't have hashing support, plugin IDs are selected at
  random. This means that when ever Cmake is reconfigured, it's a new plugin and
  you'll need to reinsert it in the DAW.
- On windows, the system won't be able to install outputs, so the easiest way to
  test your plugin is to add
  `/path/to/repo/build/[your plugin]\_artefacts/Debug/VST3` to the list of VST
  search paths in your DAW.

### Making it work with VsCode

<!--TODO: VsCode instructions. -->

## The UI

<!--TODO: UI -->

## Collaborations

Collaborators are most welcome. Feel free to make bug reports, feature requests
and pull requests. If you manage to make a nice plugin, add wish to share with
the world, it can become part of the repo through a pull request. A wrapper for
the JUCE framework is available, but since plugins are written in
platform-agnostic code, targeting more platforms in the future is possible. If
anyone wishes to colaborate, adding a wrapper for eg. ESP32 or an ADI device
would be an option for a project.
