using Sharpmake;

[Generate]
public class GoogleTestLibrary : Project
{
    public string BasePath = @"[project.SharpmakeCsPath]/include";

    public GoogleTestLibrary() : base(typeof(RiftTarget))
    {
        Name = "gtest";

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
        conf.ProjectPath = "[project.SharpmakeCsPath]/../../../Binaries/GoogleTest";
        conf.PrecompHeader = "gtest.h";

        conf.TargetLibraryPath = "[project.SharpmakeCsPath]/lib/[target.Platform]/[target.Optimization]";
        conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]/lib/[target.Platform]/[target.Optimization]/gtest.dll");
        conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]/lib/[target.Platform]/[target.Optimization]/gtest_main.dll");
        conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]/lib/[target.Platform]/[target.Optimization]/gtest.pdb");
        conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]/lib/[target.Platform]/[target.Optimization]/gtest_main.pdb");

		conf.LibraryFiles.Add(@"gtest.lib");
		conf.LibraryFiles.Add(@"gtest_main.lib");

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
