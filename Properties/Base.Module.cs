using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

class BaseModule : BaseProject
{
    public BaseModule(string CustomName)
    {
        Name = CustomName;
        SourceRootPath = @"[project.SharpmakeCsPath]";
        IsFileNameToLower = false;

        AddTargets(new RiftTarget( Editor.Editor | Editor.Game ));
    }

    public override void ConfigureAll(Configuration conf, RiftTarget target)
    {
        base.ConfigureAll(conf, target);

        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]");
        //conf.TargetLibraryPath = "[project.SharpmakeCsPath]/Lib/[target.Optimization]_[target.Platform]";

        conf.Defines.Add("NOMINMAX");
        conf.Defines.Add("_TCHAR_DEFINED");

        string UpperName = Name.ToUpper();

        // The library wants LIBRARY_COMPILE defined when it compiles the
        // library, so that it knows whether it must use dllexport or
        // dllimport.
        conf.Defines.Add(UpperName + "_COMPILE");

        if (target.OutputType == OutputType.Dll)
        {
            // We want this to output a shared library. (DLL)
            conf.Output = Configuration.OutputType.Dll;

            // This library project expects LIBRARY_DLL symbol to be defined
            // when used as a DLL. While we could define it in the executable,
            // it is better to put it as an exported define. That way, any
            // projects with a dependency on this one will have LIBRARY_DLL
            // automatically defined by Sharpmake.
            conf.ExportDefines.Add(UpperName + "_DLL");

            // Exported defines are not necessarily defines as well, so we need
            // to add LIBRARY_DLL as an ordinary define too.
            conf.Defines.Add(UpperName + "_DLL");
        }
        else if (target.OutputType == OutputType.Lib)
        {
            // We want this to output a static library. (LIB)
            conf.Output = Configuration.OutputType.Lib;
        }
    }
}
