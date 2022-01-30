# Contributing to Rift
Thank you for considering contributing to Rift. Rift needs of people like you to keep evolving and growing.

There are many ways of contributing to Rift. **Using Rift** and sharing your feedback, opinions and ideas with the community is an incredibly helpful way to start. But you are also welcomed to report bugs, propose new features, submit code changes, improve the documentation or discuss language design with us.

## Getting started
You can [read this guide](Docs/Development/Setup.md) you can follow with an step by step process on how to setup the development environment.

## Bug Reports
Before submitting a bug report, make sure you are testing the last version of Rift. The issue could have already been fixed. If that is not the case, have a look at the open [list of bugs](https://github.com/PipeRift/rift/issues?q=is%3Aopen+is%3Aissue+label%3Abug) before submitting a new [bug report](https://github.com/PipeRift/rift/issues/new?labels=bug&template=bug_report.md).

## Contributing code
You are welcome to contribute to any part of the source code. To do so you can create [Pull Requests](https://github.com/PipeRift/rift/pulls), hwever they must be directed to _main_ or to feature branches. Pull Requests to _release_ branch won't be allowed.

Some areas that always need improvement are:
- Function graph editor
  Improved visuals, quality of life improvements, better grid snapping, better rendering...
- LLVM Backend
- AST improvements. Rift's AST follows an ECS(Entity-Component-System) architecture.

Some features we don't have yet:
- Linking the C standard library
- Generation of C interfaces (using LLVM) in editor and compiler time

