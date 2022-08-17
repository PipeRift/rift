---
title: "Compiler"
description: "How to use Rift's compiler"
date: 2020-11-16T13:59:39+01:00
lastmod: 2020-11-16T13:59:39+01:00
draft: false
images: []
menu:
  docs:
    parent: "quick-start"
weight: 130
toc: true
---

## Running the compiler

`Rift.exe` is the compiler of Rift.

Just like in the editor, a parameter **-p** can be passed to specify the project to build.

```bash
Rift -p {project_path}
```

The build files and binaries will be put by default into `{project_path}/Build`.