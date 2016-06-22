#include "ExplicitConstructorTransform.h"

#include <clang/Lex/Preprocessor.h>

#include "Replacer.h"

using namespace clang::ast_matchers;

REGISTER_TRANSFORM(ExplicitConstructorTransform);

//==============================================================================
class ConstructorHandler : public MatchFinder::MatchCallback
{
public:
	ConstructorHandler(clang::CompilerInstance* ci) : compilerInstance(ci) {}

	virtual void run(const MatchFinder::MatchResult& result) {
		if (const clang::CXXConstructorDecl* ctor = result.Nodes.getNodeAs<clang::CXXConstructorDecl>("ctor")) {
			clang::SourceManager* src_manager = result.SourceManager;

			// Determine if source location is within accepted paths.
			clang::SourceLocation loc = ctor->getLocation();
			clang::FullSourceLoc src_loc(loc, *src_manager);

			const clang::FileEntry* entry = src_manager->getFileEntryForID(src_loc.getFileID());
			std::string absolute_name = refactorial::util::absolutePath(entry->getName());

			// TODO: Factor this out along with the NamedDeclMatcher allowed dir logic into common util fn.
			if (llvm::StringRef(absolute_name).startswith(llvm::StringRef("/src/refactorial")))
			{
				llvm::outs() << "ctor: " << ctor->getQualifiedNameAsString() << "\n";
				llvm::StringRef s = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(ctor->getSourceRange()), *src_manager, clang::LangOptions(), 0);
				llvm::outs() << s << " | " << absolute_name << "\n";

				llvm::Regex explicit_regex("^explicit.*$", llvm::Regex::IgnoreCase);
				if (!explicit_regex.match(s))
				{
					// FIXME: Seems like this should just be part of the Replacer. Rip out the logic from
					// NamedDeclMatcher as it appears to handle macros etc.
					clang::Preprocessor& preprocessor = compilerInstance->getPreprocessor();
					clang::SourceLocation loc_end = preprocessor.getLocForEndOfToken(loc);
					if (loc_end.isValid())
					{
						clang::SourceLocation end = loc_end.getLocWithOffset(-1);
						llvm::Twine replacement_text = "explicit " + s;
						Replacer::instance().replace(clang::SourceRange(loc, end), replacement_text.str(), *src_manager);
					}
				}
			}
		}
	}

private:
	clang::CompilerInstance* compilerInstance;
};

//==============================================================================
void ExplicitConstructorTransform::HandleTranslationUnit(clang::ASTContext& c)
{
	ConstructorHandler HandlerForConstructor(ci);

	MatchFinder finder;
	finder.addMatcher(cxxConstructorDecl(hasParent(cxxRecordDecl())).bind("ctor"), &HandlerForConstructor);

	finder.matchAST(c);
}
