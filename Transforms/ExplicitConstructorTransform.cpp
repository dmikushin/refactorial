#include "ExplicitConstructorTransform.h"

#include <clang/Lex/Preprocessor.h>

#include "Replacer.h"

using namespace clang::ast_matchers;

REGISTER_TRANSFORM(ExplicitConstructorTransform);

//==============================================================================
class ConstructorHandler : public MatchFinder::MatchCallback
{
public:
	ConstructorHandler(Transform* transform)
	  : _transform(transform),
		_explicit_regex(llvm::Regex("^explicit.*$", llvm::Regex::IgnoreCase))
	{}

	virtual void run(const MatchFinder::MatchResult& result) {
		if (const clang::CXXConstructorDecl* ctor = result.Nodes.getNodeAs<clang::CXXConstructorDecl>("ctor")) {
			// Nothing to do if this isn't a user defined constructor.
			if (!ctor->isUserProvided())
			{
				return;
			}

			// Determine if source location is within accepted paths.
			clang::SourceLocation loc = ctor->getLocation();
			if (!_transform->canChangeLocation(loc)) {
				return;
			}

			clang::SourceManager* src_manager = result.SourceManager;
			llvm::StringRef s = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(ctor->getSourceRange()),
															*src_manager, clang::LangOptions(), 0);
			if (!_explicit_regex.match(s))
			{
				Replacer::instance().insert(loc, "explicit ", *src_manager);
			}
		}
	}

private:
	Transform* _transform;
	llvm::Regex _explicit_regex;
};

//==============================================================================
void ExplicitConstructorTransform::HandleTranslationUnit(clang::ASTContext& c)
{
	init();

	ConstructorHandler HandlerForConstructor(this);

	MatchFinder finder;
	finder.addMatcher(cxxConstructorDecl(hasParent(cxxRecordDecl())).bind("ctor"), &HandlerForConstructor);

	finder.matchAST(c);
}

//==============================================================================
refactorial::config::TransformConfig* ExplicitConstructorTransform::getTransformConfig()
{
	return &(TransformRegistry::get().config.transforms.explicit_constructor_transform);
}
