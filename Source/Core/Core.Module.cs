using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

[Generate]
class CoreModule : BaseModule
{
    public CoreModule() : base("Core") {}

    public override void ConfigureAll(Configuration conf, RiftTarget target)
    {
        base.ConfigureAll(conf, target);

        conf.AddPublicDependency<SDL2Library>(target);
        conf.AddPublicDependency<AssimpLibrary>(target);
        conf.AddPublicDependency<CurlLibrary>(target);
        
        conf.SolutionFolder = "Core";
        //conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/Core");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/ThirdParty");

        // EA STL
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/ThirdParty/EASTL/include");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/ThirdParty/EASTL/test/packages/EAAssert/include");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/ThirdParty/EASTL/test/packages/EABase/include/Common");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/ThirdParty/EASTL/test/packages/EAMain/include");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/ThirdParty/EASTL/test/packages/EAStdC/include");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/ThirdParty/EASTL/test/packages/EAThread/include");

        
        conf.Options.Add(new Sharpmake.Options.Vc.Compiler.DisableSpecificWarnings("4152", "4100", "4201"));
        //conf.Options.Add(Sharpmake.Options.Vc.Compiler.ShowIncludes.Enable);

        if(target.Optimization == Optimization.Debug)
        {
            conf.Defines.Add("GLM_FORCE_XYZW_ONLY");
        }
        conf.Defines.Add("TRACY_ENABLE");

        switch(target.Platform) {
            case Platform.win32:
                conf.Defines.Add("PLATFORM_WINDOWS");
                conf.Defines.Add("_WIN32");
                conf.ExportDefines.Add("PLATFORM_WINDOWS");
                conf.ExportDefines.Add("_WIN32");
                break;
            case Platform.win64:
                conf.Defines.Add("PLATFORM_WINDOWS");
                conf.Defines.Add("_WIN64");
                conf.ExportDefines.Add("PLATFORM_WINDOWS");
                conf.ExportDefines.Add("_WIN64");
                break;
        }
    }
}
