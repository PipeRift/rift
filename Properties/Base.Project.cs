using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library

class BaseProject : Project
{
    public BaseProject() : base(typeof(RiftTarget)) {}
    public BaseProject(string CustomName) : base(typeof(RiftTarget))
    {
        Name = CustomName;
        SourceRootPath = @"[project.SharpmakeCsPath]";
        IsFileNameToLower = false;

        AddTargets(new RiftTarget( Editor.Editor | Editor.Game ));
    }

    [Configure]
    public virtual void ConfigureAll(Configuration conf, RiftTarget target)
    {
        conf.ProjectPath = @"[project.SharpmakeCsPath]\..\..\Binaries\[project.Name]";
        
        conf.Options.Add(Sharpmake.Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Sharpmake.Options.Vc.Compiler.RTTI.Enable);
        conf.Options.Add(Sharpmake.Options.Vc.Compiler.Exceptions.Enable);


        conf.Options.Add(Sharpmake.Options.Vc.Compiler.Exceptions.Enable);

        if(target.Optimization == Optimization.Release)
        {
            //conf.Options.Add(Sharpmake.Options.Vc.General.TreatWarningsAsErrors.Enable);
        }
        else if(target.Optimization == Optimization.Debug)
        {
            conf.Options.Add(Sharpmake.Options.Vc.Compiler.Optimization.Disable);
            conf.Options.Add(Sharpmake.Options.Vc.Compiler.Inline.OnlyInline);
        }
        
        if(conf.Platform == Platform.win32)
        {
            conf.Defines.Add("_WIN32");
        }
        else if(conf.Platform == Platform.win64)
        {
            conf.Defines.Add("_WIN64");
        }
    
        if(target.Editor == Editor.Editor)
        {
            conf.Defines.Add("WITH_EDITOR");
        }
    }
}
