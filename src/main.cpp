#include "Transforms/Transforms.h"
#include "xunused.h"

#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/DebugInfo/Symbolize/Symbolize.h>

#include <algorithm>
#include <atomic>
#include <execution>
#include <iostream>
#include <fstream>
#include <map>

using namespace clang;
using namespace clang::tooling;
using namespace llvm::symbolize;

static llvm::cl::OptionCategory optionCategory("Refactorial options");

static llvm::cl::opt<bool> apply_replacements(
	"apply",
	llvm::cl::desc("Apply identified replacements."),
	llvm::cl::Optional);

static llvm::cl::opt<bool> print_replacements(
	"print",
	llvm::cl::desc("Print identified replacements."),
	llvm::cl::Optional);

static llvm::cl::opt<std::string> specfile_path(
	"spec",
	llvm::cl::desc("Refactor specification file path."),
	llvm::cl::Required);

static const char usageText[] = "";

// Compare replacements w/o comparing text
static bool ordering_cmp(const Replacement &LHS, const Replacement &RHS)
{
	if (LHS.getOffset() != RHS.getOffset())
		return LHS.getOffset() < RHS.getOffset();
	if (LHS.getLength() != RHS.getLength())
		return LHS.getLength() < RHS.getLength();
	return LHS.getFilePath() < RHS.getFilePath();
}

// Imported RefactoringTool::runAndSave() using std::vector
static int commit_changes(RefactoringTool &Tool, const Replacements &R)
{
	LangOptions DefaultLangOptions;
	IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
	TextDiagnosticPrinter DiagnosticPrinter(llvm::errs(), &*DiagOpts);
	DiagnosticsEngine Diagnostics(
		IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()),
		&*DiagOpts, &DiagnosticPrinter, false);
	SourceManager Sources(Diagnostics, Tool.getFiles());
	Rewriter Rewrite(Sources, DefaultLangOptions);

	if (!applyAllReplacements(R, Rewrite))
		llvm::errs() << "Skipped some replacements.\n";

	return Rewrite.overwriteChangedFiles() ? 1 : 0;
}

static std::string readFromStream(std::istream& in)
{
	std::string ret;
	char buffer[4096];
	while (in.read(buffer, sizeof(buffer)))
	{
		ret.append(buffer, sizeof(buffer));
	}
	ret.append(buffer, in.gcount());
	return ret;
}

int main(int argc, const char **argv)
{
	CommonOptionsParser optionsParser(argc, argv, optionCategory, usageText);

	std::vector<std::function<void()>> tasks;

	auto && sources = optionsParser.getCompilations().getAllFiles();
	const size_t total = sources.size();

	std::ifstream spec_stream(specfile_path);
	if (!spec_stream.is_open())
	{
		llvm::errs() << "Unable to open provided spec file path." << "\n";
		return 1;
	}

	std::string config_yaml = readFromStream(spec_stream);
	spec_stream.close();

	refactorial::config::Config config;
	llvm::yaml::Input yin(config_yaml);
	yin >> config;

	if (config.transforms.function_remove_transform.remove_unused)
	{
		// Run xunused separately to get the list of unused functions.
		std::vector<UnusedDefInfo> unused;
		xunused(optionsParser.getCompilations(), unused);

		// Add unused functions to the list for removal.
		auto& removes = config.transforms.function_remove_transform.removes;
		for (int i = 0; i < unused.size(); i++)
		{
			auto& I = unused[i];

			if (print_replacements)
			{
				llvm::errs() << I.filename << ":" << I.line << ": function '" <<
					LLVMSymbolizer::DemangleName(I.nameMangled, nullptr) << "' is unused";
				llvm::errs() << "\n";
				for (auto & D : I.declarations)
				{
					llvm::errs() << D.filename << ":" << D.line <<
						": note:" << " declared here";
					llvm::errs() << "\n";
				}
			}

			removes.emplace_back(I.nameMangled, true);
		}
	}

	TransformRegistry::get().config = config;

	for (auto&& file : sources)
	{
		tasks.emplace_back([total, file, &optionsParser, &config]
		{
			static std::atomic_int32_t counter;
			std::cout << "[" << counter++ << "/" << total << "] " << file << std::endl;

			RefactoringTool rt(optionsParser.getCompilations(), file);

			IgnoringDiagConsumer ignore;
			rt.setDiagnosticConsumer(&ignore);

			std::map<Replacement, std::pair<std::string, int> > replacements;
			// TODO Not thread-safe
			TransformRegistry::get().replacements = &replacements;

			// Execute only "used" transforms - those, for which a YAML config has been provided.
			#define EXEC_TRANSFORM(transform_name, transform_config) \
				if (config.transforms.transform_config.used) \
				{ \
					rt.run(new TransformFactory(TransformRegistry::get()[transform_name])); \
				}

			// TODO Each transform should run in a replacement facility alone,
			// so we need to run them one by one.	
			EXEC_TRANSFORM("Qt3To5UIClasses", qt3_to_5_ui_classes)
			EXEC_TRANSFORM("TypeRenameTransform", type_rename_transform)
			EXEC_TRANSFORM("FunctionRenameTransform", function_rename_transform)
			EXEC_TRANSFORM("FunctionRemoveTransform", function_remove_transform)
			EXEC_TRANSFORM("RecordFieldRenameTransform", record_field_rename_transform)
			EXEC_TRANSFORM("ExplicitConstructorTransform", explicit_constructor_transform)
			EXEC_TRANSFORM("ArgumentChange", argument_change_transform)
			
			// TODO Is not a replacement, so should not be here.
			EXEC_TRANSFORM("AccessorsTransform", accessors_transform)

			if (print_replacements)
			{
				llvm::outs() << "The following changes will be made to the code:\n";
				for (const auto& r : replacements)
				{
					const auto& file = r.first.getFilePath();
					const int line = r.second.second;
					const auto& original = r.second.first;
					const auto& replacement = r.first.getReplacementText();
					
					llvm::outs() << file << ":" << line << ": ";
					if ((original != "") && (replacement != ""))
						llvm::outs() << "'" << original << "' -> '" << replacement << "'\n";
					else if (original != "")
						llvm::outs() << "removing '" << original << "'\n";
					else if (replacement != "")
						llvm::outs() << "inserting '" << replacement << "'\n";
				}
			}

			if (apply_replacements)
			{
				std::map<llvm::StringRef, Replacements> rs;
				for (const auto& r : replacements)
					auto err = rs[r.first.getFilePath()].add(r.first);

				for (const auto& r : rs)
					commit_changes(rt, r.second);
			}
		});
	}

	std::for_each(/*std::execution::par_unseq,*/ std::begin(tasks), std::end(tasks), [](auto && f) { f(); });

	return 0;
}

