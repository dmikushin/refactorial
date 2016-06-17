#include <yamlreader.h>

#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>

#include <iostream>
#include <fstream>

using namespace clang;
using namespace clang::tooling;

#include "Transforms/Transforms.h"

static llvm::cl::OptionCategory optionCategory("Refactorial options");

static llvm::cl::opt<std::string> refactor_specifications(
        "rulespec",
        llvm::cl::desc("file with refactoring information, overrides stdin"),
        llvm::cl::Optional);

static const char usageText[] = "";

/// Compare replacements w/o comparing text
static bool
ordering_cmp(const Replacement &LHS, const Replacement &RHS) {
	if (LHS.getOffset() != RHS.getOffset())
		return LHS.getOffset() < RHS.getOffset();
	if (LHS.getLength() != RHS.getLength())
		return LHS.getLength() < RHS.getLength();
	return LHS.getFilePath() < RHS.getFilePath();
}

/// Remove duplicate replacements with order preservation
static void
stable_deduplicate(std::vector<Replacement> &r) {
	// array sorted with Replacement::operator<()
	std::vector<Replacement> sorted(r.begin(), r.end());
	std::sort(sorted.begin(), sorted.end());

	// search for duplicates, store every with counter
	std::map<Replacement, int> doublets;
	auto it = std::adjacent_find(sorted.begin(), sorted.end());
	while (it != sorted.end()) {
		doublets.insert(std::make_pair(*it, 0));
		auto range = std::equal_range(it, sorted.end(), *it);
		it = std::adjacent_find(range.second, sorted.end());
	}

	std::stable_sort(r.begin(), r.end(), ordering_cmp);

	// collect duplicate indices except for the first in every equality set
	std::vector<size_t> to_erase;
	for (size_t i = 0, n = r.size(); i < n; ++i) {
		auto dit = doublets.find(r.at(i));
		if (dit != doublets.end() && dit->second++ > 0) {
			to_erase.push_back(i);
		}
	}
	for (auto it = to_erase.rbegin(), end = to_erase.rend(); it != end; ++it) {
		r.erase(r.begin() + *it);
	}
}

/// Imported RefactoringTool::runAndSave() using std::vector
static int
commit_changes(RefactoringTool &Tool, const std::vector<Replacement> &R) {
  LangOptions DefaultLangOptions;
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  TextDiagnosticPrinter DiagnosticPrinter(llvm::errs(), &*DiagOpts);
  DiagnosticsEngine Diagnostics(
      IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()),
      &*DiagOpts, &DiagnosticPrinter, false);
  SourceManager Sources(Diagnostics, Tool.getFiles());
  Rewriter Rewrite(Sources, DefaultLangOptions);

  if (!applyAllReplacements(R, Rewrite)) {
    llvm::errs() << "Skipped some replacements.\n";
  }
  return Rewrite.overwriteChangedFiles() ? 1 : 0;
}

std::string readFromStream(std::istream& in)
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
    CommonOptionsParser cmdl(argc, argv, optionCategory, usageText);

	// // Test output
	// llvm::outs() << "test yaml generation" << "\n";

	// yaml::reader::TypeRenameTransform t;
	// t.types.push_back("Q3Variant QVariant");
	// t.types.push_back("Q3ValueList QList");
	// t.types.push_back("QCString QByteArray");
	// t.types.push_back("QString: QSTRING2");
	// yaml::reader::Transforms transforms;
	// transforms.type_rename_transform = t;
	// yaml::reader::Config c;
	// c.transforms = transforms;

	// llvm::yaml::Output yout(llvm::outs());
	// yout << c;

	// // Test input
	// std::string doc(std::istreambuf_iterator<char>(std::cin), {});

	// yaml::reader::Config c2;
	// llvm::yaml::Input yin(doc);
	// yin >> c2;

	// yout << c2;

    // // read refactoring specifications. either read from stdin or file
    // std::vector<YAML::Node> config;
    // if (refactor_specifications.empty()) {
    //     llvm::errs() << "falling back to read from stdin\n";
    //     config = YAML::LoadAll(std::cin);
    // } else {
    //     std::ifstream fin(refactor_specifications.c_str(), std::ifstream::binary);
    //     if (fin) {
    //         config = YAML::LoadAll(fin);
    //     } else {
    //         llvm::errs() << "barf: cannot open file '" << refactor_specifications << "'\n";
    //         exit(-1);
    //     }
    //     fin.close();
    // }

	RefactoringTool rt(cmdl.getCompilations(), cmdl.getSourcePathList());

	IgnoringDiagConsumer ignore;
	rt.setDiagnosticConsumer(&ignore);

	std::string config_yaml = readFromStream(std::cin);
	yaml::reader::Config config;
	llvm::yaml::Input yin(config_yaml);
	yin >> config;

	std::vector<Replacement> replacements;

	TransformRegistry::get().config = config;
	TransformRegistry::get().replacements = &replacements;

	// TODO: Add file support

	rt.run(new TransformFactory(TransformRegistry::get()["TypeRenameTransform"]));

	llvm::outs() << "Replacements collected by the tool:\n";
	for (auto &r : rt.getReplacements()) {
		llvm::outs() << r.toString() << "\n";
	}

	// TODO: Apply replacements

	// // iterate over refactoring specification
	// for(const YAML::Node &configSection : config) {
	// 	//figure out which files we need to work on
	// 	/*
	// 	std::vector<std::string> inputFiles;
	// 	if(configSection["Files"])
	// 		inputFiles = configSection["Files"].as<std::vector<std::string> >();
	// 	if(!configSection["Transforms"]) {
	// 		llvm::errs() << "No transforms specified in this configuration section:\n";
	// 		llvm::errs() << YAML::Dump(configSection) << "\n";
	// 	}
		
	// 	//load up the compilation database
    //     if (inputFiles.empty()) {
    //         inputFiles = Compilations.getAllFiles();
    //         llvm::errs() << "no input files in refactoring yml given. "
    //             << "using " << inputFiles.size() << " compile units "
    //             << "from compile_commands.json\n";
    //     }
	// 	*/


	// 	TransformRegistry::get().config = configSection["Transforms"];
	// 	TransformRegistry::get().replacements = &replacements;
		
	// 	//finally, run
	// 	for(const auto &trans : configSection["Transforms"]) {
	// 		replacements.clear();
	// 		std::string transId = trans.first.as<std::string>() + "Transform";
	// 		llvm::errs() << "Doing a '" << transId << "'\n";
	// 		if (rt.run(new TransformFactory(TransformRegistry::get()[transId])) == 0) {
	// 			// TODO check return values
	// 			stable_deduplicate(replacements);
	// 			commit_changes(rt, replacements);
	// 		}
	// 	}
	// }
	return 0;
}
