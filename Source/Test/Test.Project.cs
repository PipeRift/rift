using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

[Generate]
class TestProject : BaseProject
{
    public TestProject() : base("Test") {}

    public override void ConfigureAll(Configuration conf, RiftTarget target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionFolder = "Projects";

        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]");
    
        conf.Defines.Remove("TRACY_ENABLE");

        conf.Options.Add(Sharpmake.Options.Vc.Linker.SubSystem.Application);

        conf.AddPublicDependency<CoreModule>(target);
        conf.AddPublicDependency<GoogleTestLibrary>(target);
    }
}
