# Rift
[![build](https://github.com/PipeRift/rift/actions/workflows/build.yml/badge.svg)](https://github.com/PipeRift/rift/actions/workflows/build.yml) [![license](https://img.shields.io/github/license/PipeRift/rift?labelColor=394047)](https://github.com/PipeRift/rift/blob/main/LICENSE) [![Code Quality](https://api.codacy.com/project/badge/Grade/a377314fe8ae4a2bb17159bce8a1ac76)](https://app.codacy.com/gh/PipeRift/rift?utm_source=github.com&utm_medium=referral&utm_content=PipeRift/rift&utm_campaign=Badge_Grade)

Rift is a **compiled**, **typed** and **visual** programming language.

## Code is Data

**Rift** code files do not contain plain code, instead they have a **data representation of the code**.

Where in languages like C, C++ you write code into text directly, in Rift you use an editor to modify it.
At first this might sound counterintuitive, but it actually brings multiple key advantages with huge potential for the developer and code quality.

### Coding style is User preference
Code does not specify how your code looks, your settings in the editor do, and they are your choice.
_You will always be comfortable in any code, because it is always the coding standard you prefer_

### Many code representations, one language
Rift has the concept of _"Views"_ which are different visualizations of code (E.g: Visual Nodes and Text).

This means developers can choose how to code, while on the **same language**, **same editor**, **same build system**, and **same compiler**. One example would be in the development of a game where a designer uses visual nodes and a programmer uses text, but they are both using the same environment. In the future, they could potentially even convert code between views.

### No include or definition order
Since it is data we are talking about, theres no need for includes or order of functions or variables. Its all already there as soon as the project has loaded.

## Compile times

Rift´s compiler is designed to be **fast** (no I mean, really fast).
Some development workflows now a day have terrible iteration times, and a big reason of that is really slow compile times (talking about hours, days or even weeks).

The problem with slow compile times is not only the amount of wasted time for the developer (including focus and concentration issues), but also the speed and cost of CICD pipelines, or the frequency at which pipelines can run.
Fast iteration times means many solutions we implement for our projects to work around this issues are simply not needed at all.
You don't need to distribute binaries if building takes less time than syncing them. You don't need complicated pipelines with package managers downloading binaries either.

It means more productivity, and less wasted money. So yes, in Rift this is important.


## Modularity

Unlike traditional compilers, Rift is designed so that modules are plugged in and out, modifying editor and compiler behavior according to the needs of your project.

Due to Rift being so new, this modularity is only partially implemented at the moment.
However, you can already plug in your own backends. They will automatically show up in the editor and CLI.

One example of a custom backend could be JIT (Just In Time compilation).

Some examples of what future modularity would look like:
- Custom views<br>
  Some examples of what custom views could be:
  - Text view
  - Execution flow view
  - Behavior Trees
- Editor/compiler/both passes
  - E.g: A format checker that ensures correct naming across a module, even while compiling


## Software Quality
[![build](https://github.com/PipeRift/rift/actions/workflows/build.yml/badge.svg)](https://github.com/PipeRift/rift/actions/workflows/build.yml) [![static-analysis](https://github.com/PipeRift/rift/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/PipeRift/rift/actions/workflows/static-analysis.yml)

The code is validated on the following platforms/compilers:

| Windows     | Ubuntu      | Mac         |
| ----------- | ----------- | ----------- |
| Clang ^12.0 | Clang ^12.0 | Clang ^12.0 |
| MSVC        | GCC ^9      |             |

It has also been tested on **x64** and **ARM64** architectures.

## Credits
Developed by Miguel Fernandez Arce (Muit) and every direct or indirect [contributors](https://github.com/piperift/rift/graphs/contributors) to the GitHub repository.

## License
Rift is licensed under the LGPLv3 License, see [LICENSE](https://github.com/piperift/rift/blob/master/LICENSE) for more information.

All libraries outside of Rift are free to choose their own licenses even if they extend Rift's functionality as modules.

Some libraries inside Rift could also be licensed under more permissive licenses.
