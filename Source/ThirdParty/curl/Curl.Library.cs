using System;
using Sharpmake;

[Generate]
public class CurlLibrary : Project
{
    public string BasePath = @"[project.SharpmakeCsPath]/include";

    public CurlLibrary() : base(typeof(RiftTarget))
    {
        Name = "libcurl";

        AddTargets(new RiftTarget(
            Editor.Editor | Editor.Game,
            OutputType.Dll | OutputType.Lib
        ));

        SourceRootPath = "[project.BasePath]";
        IsFileNameToLower = false;
    }

    [Configure()]
    public void ConfigureAll(Configuration conf, Target target)
    {
        conf.SolutionFolder = "Core/ThirdParty";
        conf.IncludePaths.Add("[project.BasePath]");
        conf.ProjectPath = "[project.SharpmakeCsPath]/../../../Binaries/curl";

        conf.TargetLibraryPath = "[project.SharpmakeCsPath]/lib/[target.Platform]";
        conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]/lib/[target.Platform]/libcurl.dll");

		conf.LibraryFiles.Add(@"libcurl.lib");

        // To use /DEBUG:FULL
        conf.Options.Add(Sharpmake.Options.Vc.Linker.GenerateDebugInformation.Enable);
        conf.Options.Add(Sharpmake.Options.Vc.Linker.GenerateFullProgramDatabaseFile.Enable);

        if (target.OutputType == OutputType.Dll)
        {
            conf.Output = Configuration.OutputType.Dll;
            conf.Defines.Add("_DLL");
        }
        else if (target.OutputType == OutputType.Lib)
        {
            conf.Output = Configuration.OutputType.Lib;
        }
    }
}
