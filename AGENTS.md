# AGENTS.md

## Project

Rift: visual programming language compiled to machine code.

## Build
C++20, CMake 3.26+, Ninja generator (optional).

```bash
# Configure (first time or after CMakeLists changes)
cmake -GNinja -S . -B Build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build Build --config Release

# Build Debug
cmake --build Build --config Debug
```

## Test

```bash
# From Build directory
cd Build
ctest --output-on-failure -j2 -C Release
```

Test target: `RiftTests` (links Bandit test framework). Test source: `Tests/`.

## Format & Lint

```bash
# Format all source files
cmake --build Build --target ClangFormat

# Run clang-tidy
cmake --build Build --target ClangTidy
```

Formatting uses `.clang-format` (Microsoft base, tabs, 100 col limit).

## Naming Conventions

From `.clang-tidy`:
- Namespaces, classes, structs, functions: `CamelCase`
- Members, parameters, variables, global constants: `camelBack`
- Global variables: `g` prefix + `CamelCase`

## Architecture

```
Apps/
  CLI/          - Rift command-line tool (output name: "Rift")
  Editor/       - Visual editor (imgui + OpenGL + glfw)
Libs/
  AST/          - Core AST library (RiftASTLib) - main library
  Backends/MIR/ - MIR compiler backend
  Bindings/Native/ - Native language bindings
  Editor/       - Editor module
  Runtimes/     - Runtime modules
  UI/           - UI library
  Views/Graph/  - Visual graph view
Tests/          - Bandit-based tests (RiftTests)
Extern/         - Git submodules (see below)
CMake/          - CMake utilities (rift_module(), etc.)
Build/          - Build output (do not edit)
```

## Key Conventions

- `rift_module()` function in `CMake/Util.cmake` configures targets: platform defines, C++20, RTTI disabled, shared output dir
- `rift_compiler_module()`, `rift_editor_module()`, `rift_runtime_module()` add module-specific setup
- RTTI is disabled project-wide (`-fno-rtti`)
- Compiler modules accumulate into `RiftCompilerModules`, `RiftEditorModules`, `RiftRuntimeModules` interface libraries
- Extern submodules: Pipe (core utilities), CLI11, imgui, glfw, taskflow, mir (MIR backend), IconFontCppHeaders
- Editor requires OpenGL (gl3w + glfw + imgui)
- Pipe submodule has its own `CMake/Util.cmake` with platform detection and helper functions

## CI

- Build matrix: MSVC, Clang 16, GCC 13 across Windows/Linux/macOS
- Static analysis: cppcheck via GitHub Action
- Periodic format: auto-applies clang-format on schedule (Mon/Wed/Sat)
- Release: triggered by `v*.*.*` tags, builds then publishes zips

## PR Rules

- PRs target `main` or feature branches only (not `release`)
- Tests must pass (`ctest`)
