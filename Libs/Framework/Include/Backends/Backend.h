// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Project.h"

#include <CoreObject.h>
#include <Profiler.h>


namespace VCLang::Backends
{
	struct CompileError : public Struct
	{
		STRUCT(CompileError, Struct)

		String text;
	};


	class Backend : public Object
	{
		CLASS(Backend, Object)

	protected:
		Ptr<Project> project;

		Path buildPath;
		Path intermediatesPath;
		Path binariesPath;

		PROP(TArray<CompileError>, errors)
		TArray<CompileError> errors;


	public:
		void SetProject(Ptr<Project> inProject);

		void Compile();

		bool HasErrors() const
		{
			return errors.Size() > 0;
		}

		void AddError(StringView str)
		{
			Log::Error(str);
			CompileError newError{};
			newError.text = str;
			errors.Add(newError);
		}

	protected:
		virtual void OnCompile() = 0;
		virtual void OnCleanup(){};
	};
}	 // namespace VCLang::Backends
