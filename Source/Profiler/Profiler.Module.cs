using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

[Generate]
class ProfilerModule : BaseModule
{
    public ProfilerModule() : base("Profiler") {}

    public override void ConfigureAll(Configuration conf, VCLang.Target target)
    {
        base.ConfigureAll(conf, target);

        conf.SolutionFolder = "Framework";
        //conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/Core");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/ThirdParty");

        //conf.Options.Add(new Sharpmake.Options.Vc.Compiler.DisableSpecificWarnings("4152", "4100", "4201"));

        conf.Defines.Add("TRACY_ENABLE");
    }
}
