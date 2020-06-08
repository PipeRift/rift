using System;
using System.IO; // for Path.Combine
using Sharpmake; // contains the entire Sharpmake object library.

[module: Sharpmake.Include("Properties/Base.Project.cs")]
[module: Sharpmake.Include("Properties/Base.Module.cs")]
[module: Sharpmake.Include("Source/Core/Core.Module.cs")]
[module: Sharpmake.Include("Source/Profiler/Profiler.Module.cs")]
[module: Sharpmake.Include("Source/Compiler/Compiler.Project.cs")]
[module: Sharpmake.Include("Source/Editor/Editor.Project.cs")]
[module: Sharpmake.Include("Source/ThirdParty/SDL2/SDL2.Library.cs")]


namespace VCLang
{
    [Generate]
    class Solution : Sharpmake.Solution
    {
        public Solution() : base(typeof(VCLang.Target))
        {
            // The name of the solution.
            Name = "VCLang";
            IsFileNameToLower = false;

            // As with the project, define which target this solution builds for.
            // It's usually the same thing.
            AddTargets(new VCLang.Target());
        }

        [Configure]
        public virtual void ConfigureAll(Solution.Configuration conf, VCLang.Target target)
        {
            // Puts the generated solution in the /generated folder too.
            conf.SolutionPath = @"[solution.SharpmakeCsPath]";
            //conf.ProjectPath = @"[solution.SharpmakeCsPath]/Binaries";

            conf.AddProject<CompilerProject>(target);
            conf.AddProject<EditorProject>(target);
            conf.AddProject<CoreModule>(target);
            conf.AddProject<ProfilerModule>(target);
            conf.AddProject<SDL2Library>(target);
        }

    }


    class Target : Sharpmake.Target
    {
        public override string Name
        {
            get { return Optimization.ToString(); }
        }

        public Target() : base(
            Platform.win32 | Platform.win64,
            DevEnv.vs2019,
            Optimization.Debug | Optimization.Release)
        {}

        public Target(OutputType outputType) : base(
            Platform.win32 | Platform.win64,
            DevEnv.vs2019,
            Optimization.Debug | Optimization.Release,
            outputType)
        {}
    }
}

public static class VCLangMain
{
    [Sharpmake.Main]
    public static void SharpmakeMain(Arguments arguments)
    {
        // Set Windows SDK 10 to use Latest
        KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2017, KitsRootEnum.KitsRoot10, Options.Vc.General.WindowsTargetPlatformVersion.Latest);
        KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2019, KitsRootEnum.KitsRoot10, Options.Vc.General.WindowsTargetPlatformVersion.Latest);

        arguments.Generate<VCLang.Solution>();
    }
}