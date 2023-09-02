# Contributing to Rift
Thank you for considering contributing to Rift. Rift needs of great people like you to keep evolving and growing.

There are many ways of contributing to Rift. **Using Rift** and sharing your feedback, opinions and ideas with us is an incredibly helpful way to start. But you are also welcomed to report bugs, propose new features, submit code changes, improve the documentation or discuss language design with us.


## Bug Reports
Before submitting a bug report, make sure you are testing the last version of Rift. The issue could have already been fixed. If that is not the case, have a look at the open [list of bugs](https://github.com/PipeRift/rift/issues?q=is%3Aopen+is%3Aissue+label%3Abug) before submitting a new [bug report](https://github.com/PipeRift/rift/issues/new?labels=bug&template=bug_report.md).

## Contributing code
You are welcome to contribute to any part of the source code. To do so you can create [Pull Requests](https://github.com/PipeRift/rift/pulls), however they must be directed to _main_ or to feature branches. Pull Requests to _release_ branch are not allowed.

The three big areas that need contributions are:
- **AST framework**: AST API, type checking, dependencies, etc.
- **Editor**: Editing & debugging rift code using the AST API.
- **Just-In-Time backend (MIR)**: Compiles and runs Rift code using *vnmakarov/mir*. (LLVM Backend has been discarded)

If you have experience with compiler backends and think you can build a great one for rift, **join me!**<br>
Sadly they are not my strength, so **your help would be extremely valuable**, and I could focus on other areas more.

### Getting started
You can [read this guide](https://riftlang.org/docs/development/setup/) you can follow with an step by step process on how to setup the development environment.
