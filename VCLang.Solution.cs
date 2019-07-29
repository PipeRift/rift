using System;
using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

[module: Sharpmake.Include("Properties/Base.Project.cs")]
[module: Sharpmake.Include("Properties/Base.Module.cs")]
[module: Sharpmake.Include("Source/Core/Core.Module.cs")]
[module: Sharpmake.Include("Source/Compiler/Compiler.Project.cs")]
[module: Sharpmake.Include("Source/Editor/Editor.Project.cs")]
[module: Sharpmake.Include("Source/Test/Test.Project.cs")]
[module: Sharpmake.Include("Source/ThirdParty/SDL2/SDL2.Library.cs")]
[module: Sharpmake.Include("Source/ThirdParty/Assimp/Assimp.Library.cs")]
[module: Sharpmake.Include("Source/ThirdParty/curl/Curl.Library.cs")]
[module: Sharpmake.Include("Source/ThirdParty/GoogleTest/GoogleTest.Library.cs")]

[Generate]
class VCLangSolution : Solution
{
    public VCLangSolution() : base(typeof(RiftTarget))
    {
        // The name of the solution.
        Name = "VCLang";
        IsFileNameToLower = false;

        // As with the project, define which target this solution builds for.
        // It's usually the same thing.
        AddTargets(new RiftTarget( Editor.Editor | Editor.Game ));
    }

    [Configure]
    public virtual void ConfigureAll(Solution.Configuration conf, RiftTarget target)
    {   
        // Puts the generated solution in the /generated folder too.
        conf.SolutionPath = @"[solution.SharpmakeCsPath]";
        //conf.ProjectPath = @"[solution.SharpmakeCsPath]/Binaries";

        conf.AddProject<CompilerProject>(target);
        conf.AddProject<EditorProject>(target);
        conf.AddProject<TestProject>(target);
        conf.AddProject<CoreModule>(target);
        conf.AddProject<SDL2Library>(target);
        conf.AddProject<AssimpLibrary>(target);
        conf.AddProject<CurlLibrary>(target);
    }


    [Main]
    public static void SharpmakeMain(Arguments sharpmakeArgs)
    {
        sharpmakeArgs.Generate<VCLangSolution>();
    }
}


[Fragment, Flags]
public enum Editor
{
    Editor = 0x01,
    Game = 0x02
}

class RiftTarget : Target {
    public Editor Editor;

    public override string Name
    {
        get {
            //if(Optimization == Optimization.Release)
            //    return Optimization.ToString();
            return Optimization.ToString() + " " + Editor.ToString();
        }
    }

    public RiftTarget() { }

    public RiftTarget(Editor editor, OutputType outputType = OutputType.Lib) : base(
        Platform.win32 | Platform.win64,
        DevEnv.vs2017,
        Optimization.Debug | Optimization.Release,
        outputType)
    {
        Editor = editor;
    }
}
