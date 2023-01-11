#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>

#include <algorithm>
#include <atomic>
#include <execution>
#include <iostream>
#include <fstream>
#include <map>

#include "Transforms/Transforms.h"

using namespace clang;
using namespace clang::tooling;

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

// Remove duplicate replacements with order preservation
static void stable_deduplicate(std::vector<Replacement> &r)
{
	// array sorted with Replacement::operator<()
	std::vector<Replacement> sorted(r.begin(), r.end());
	std::sort(sorted.begin(), sorted.end());

	// search for duplicates, store every with counter
	std::map<Replacement, int> doublets;
	auto it = std::adjacent_find(sorted.begin(), sorted.end());
	while (it != sorted.end())
	{
		doublets.insert(std::make_pair(*it, 0));
		auto range = std::equal_range(it, sorted.end(), *it);
		it = std::adjacent_find(range.second, sorted.end());
	}

	std::stable_sort(r.begin(), r.end(), ordering_cmp);

	// collect duplicate indices except for the first in every equality set
	std::vector<size_t> to_erase;
	for (size_t i = 0, n = r.size(); i < n; ++i)
	{
		auto dit = doublets.find(r.at(i));
		if (dit != doublets.end() && dit->second++ > 0)
			to_erase.push_back(i);
	}
	
	for (auto it = to_erase.rbegin(), end = to_erase.rend(); it != end; ++it)
		r.erase(r.begin() + *it);
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
	const size_t total = std::size(sources);

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

			std::vector<Replacement> replacements;
			// TODO Not thread-safe
			TransformRegistry::get().replacements = &replacements;

			// Execute only "used" transforms - those, for which a YAML config has been provided.
			#define EXEC_TRANSFORM(transform_name, transform_config) \
				if (config.transforms.transform_config.used) \
				{ \
					rt.run(new TransformFactory(TransformRegistry::get()[transform_name])); \
				}
	
			EXEC_TRANSFORM("Qt3To5UIClasses", qt3_to_5_ui_classes)
			EXEC_TRANSFORM("TypeRenameTransform", type_rename_transform)
			EXEC_TRANSFORM("FunctionRenameTransform", function_rename_transform)
			EXEC_TRANSFORM("RecordFieldRenameTransform", record_field_rename_transform)
			EXEC_TRANSFORM("ExplicitConstructorTransform", explicit_constructor_transform)
			EXEC_TRANSFORM("ArgumentChange", argument_change_transform)
			EXEC_TRANSFORM("AccessorsTransform", accessors_transform)

			stable_deduplicate(replacements);

			if (print_replacements)
			{
				llvm::outs() << "Replacements collected by the tool:\n";
				for (const auto& r : replacements)
					llvm::outs() << r.toString() << "\n";
			}

			if (apply_replacements)
			{
				std::map<llvm::StringRef, Replacements> rs;
				for (const auto& r : replacements)
					auto err = rs[r.getFilePath()].add(r);

				for (const auto& r : rs)
					commit_changes(rt, r.second);
			}
		});
	}

	std::for_each(std::execution::par_unseq, std::begin(tasks), std::end(tasks), [](auto && f) { f(); });

	return 0;
}

