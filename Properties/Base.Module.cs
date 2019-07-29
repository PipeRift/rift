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

        conf.Output = Configuration.OutputType.Lib;
    }
}
