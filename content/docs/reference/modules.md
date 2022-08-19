---
title: "Modules"
draft: false
toc: true
---

A module is constituted by a folder that contains a module file (with *.rift* format).

Any type files (*.rf* format) inside this folder or its subfolders are considered part of that module and will be compiled together.

## Submodules
If the subfolder of a module contains its own module file, this folder and all its files will be considered its own module.

## Exporting
By default **functions, types, and variables of a module are exported** for other modules to use.

## Dependencies
A module can have dependencies to other modules, granting it access to their code. Dependencies can be either Private or Public as it is explained later.

## Target types
The target type defines if a module will be compiled into an executable, a static library, or a dynamic library.
