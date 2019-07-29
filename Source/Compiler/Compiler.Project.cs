// Copyright 2015-2019 Piperift - All rights reserved

using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

[Generate]
class CompilerProject : BaseProject
{
    public CompilerProject() : base("Compiler") {}

    public override void ConfigureAll(Configuration conf, RiftTarget target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionFolder = "Projects";

        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]");
    
        conf.Options.Add(Sharpmake.Options.Vc.Linker.SubSystem.Application);

        conf.AddPublicDependency<CoreModule>(target);
    }
}
