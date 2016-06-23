#include "ExplicitConstructorTransform.h"

#include <clang/Lex/Preprocessor.h>

#include "Replacer.h"
#include "yamlreader.h"

using namespace clang::ast_matchers;

REGISTER_TRANSFORM(ExplicitConstructorTransform);

//==============================================================================
class ConstructorHandler : public MatchFinder::MatchCallback
{
public:
	ConstructorHandler(clang::CompilerInstance* ci)
	  : compilerInstance(ci)
	{
		config = TransformRegistry::get().config.transforms.explicit_constructor_transform;

		// FIXME: This is also copied largely from NamedDeclMatcher. Unify them as much as possible.
		for (const std::string& path : config.within_paths) {
			std::string regex = llvm::Regex::escape(path);
			allowedDirectoryList.push_back(llvm::Regex(regex + ".*", llvm::Regex::IgnoreCase));
		}
	}

	bool shouldIgnore(clang::SourceLocation L) {
		// FIXME: This is duped from NamedDeclMatcher. Unify them.
		if (!L.isValid()) {
			return true;
		}

		clang::SourceManager &SM = compilerInstance->getSourceManager();
		clang::FullSourceLoc FSL(L, SM);
		const clang::FileEntry *FE = SM.getFileEntryForID(FSL.getFileID());
		if (!FE) {
			// attempt to get the spelling location
			auto SL = SM.getSpellingLoc(L);
			if (!SL.isValid()) {
				return true;
			}

			clang::FullSourceLoc FSL2(SL, SM);
			FE = SM.getFileEntryForID(FSL2.getFileID());
			if (!FE) {
				return true;
			}
		}

		std::string absolute_name = refactorial::util::absolutePath(FE->getName());
		for (auto I = allowedDirectoryList.begin(), E = allowedDirectoryList.end(); I != E; ++I) {
			if (I->match(absolute_name)) {
				return false;
			}
		}

		return true;
	}

	virtual void run(const MatchFinder::MatchResult& result) {
		if (const clang::CXXConstructorDecl* ctor = result.Nodes.getNodeAs<clang::CXXConstructorDecl>("ctor")) {
			clang::SourceManager* src_manager = result.SourceManager;

			// Determine if source location is within accepted paths.
			clang::SourceLocation loc = ctor->getLocation();
			if (shouldIgnore(loc)) {
				return;
			}

			clang::FullSourceLoc src_loc(loc, *src_manager);

			llvm::outs() << "ctor: " << ctor->getQualifiedNameAsString() << "\n";
			llvm::StringRef s = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(ctor->getSourceRange()), *src_manager, clang::LangOptions(), 0);

			llvm::Regex explicit_regex("^explicit.*$", llvm::Regex::IgnoreCase);
			if (!explicit_regex.match(s))
			{
				llvm::Twine replacement_text = "explicit " + s;
				Replacer::instance().replace(ctor->getSourceRange(), replacement_text.str(), *src_manager);
			}
		}
	}

private:
	clang::CompilerInstance* compilerInstance;
	refactorial::config::ExplicitConstructorTransformConfig config;
	std::vector<llvm::Regex> allowedDirectoryList;
};

//==============================================================================
void ExplicitConstructorTransform::HandleTranslationUnit(clang::ASTContext& c)
{
	ConstructorHandler HandlerForConstructor(ci);

	MatchFinder finder;
	finder.addMatcher(cxxConstructorDecl(hasParent(cxxRecordDecl())).bind("ctor"), &HandlerForConstructor);

	finder.matchAST(c);
}
