# Rift Project Structure

## Project File
All rift projects must have their own **.rift** file that identifies a project and its settings.

- Specifies the type of build to target (executable, static library, dynamic library, interface...)
- Defines where to find other projects that act as dependencies
- Controls what the build process should be like (E.g: Build types)


## Type Files
Type files (with file extension **.rf**) are the "code" of the project. Classes, structs, function libraries.

**One file** contains **only one type**.
