#include "ArgumentChange.h"

#include "Replacer.h"

using namespace clang::ast_matchers;

REGISTER_TRANSFORM(ArgumentChange);

//==============================================================================
class MethodMatcherHandler : public MatchFinder::MatchCallback
{
public:
	MethodMatcherHandler(Transform* transform, refactorial::config::ArgumentChangeTransformConfig* config)
		: _transform(transform),
		  _config(config)
	{}

	virtual void run(const MatchFinder::MatchResult& result) {
		if (const clang::CXXMemberCallExpr* call = result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("call")) {
			clang::SourceManager* src_manager = result.SourceManager;

			clang::SourceLocation loc = call->getLocStart();
			if (!_transform->canChangeLocation(loc)) {
				return;
			}

			std::pair<bool, refactorial::config::Change> match = findMatchingChange(call->getMethodDecl());
			bool match_found = match.first;
			refactorial::config::Change matched_change = match.second;
			if (!match_found) return;

			if (!argumentsMatch(call->getMethodDecl(), matched_change)) return;

			std::vector<std::string> args = buildNewArguments(matched_change, src_manager, [&call](int index) {
					return call->getArg(index);
				});

			clang::SourceRange range(call->getArg(0)->getExprLoc(), call->getRParenLoc().getLocWithOffset(-1));
			Replacer::instance().replace(range, refactorial::util::joinStrings(args, ", "), *src_manager);

		} else if (const clang::CXXConstructExpr* ctor = result.Nodes.getNodeAs<clang::CXXConstructExpr>("ctor")) {
			clang::SourceManager* src_manager = result.SourceManager;

			clang::SourceLocation loc = ctor->getLocStart();
			if (!_transform->canChangeLocation(loc)) {
				return;
			}

			std::pair<bool, refactorial::config::Change> match = findMatchingChange(ctor->getConstructor());
			bool match_found = match.first;
			refactorial::config::Change matched_change = match.second;
			if (!match_found) return;

			if (!argumentsMatch(ctor->getConstructor(), matched_change)) return;

			std::vector<std::string> args = buildNewArguments(matched_change, src_manager, [&ctor](int index) {
					return ctor->getArg(index);
				});

			clang::SourceRange range(ctor->getArg(0)->getExprLoc(), ctor->getParenOrBraceRange().getEnd().getLocWithOffset(-1));
			Replacer::instance().replace(range, refactorial::util::joinStrings(args, ", "), *src_manager);
		}
	}

private:
	std::pair<bool, refactorial::config::Change> findMatchingChange(const clang::FunctionDecl* function_decl)
	{
		bool match_found = false;
		refactorial::config::Change matched_change;
		for (refactorial::config::Change change : _config->changes) {
			if (function_decl->getQualifiedNameAsString() == change.from_function &&
				function_decl->getNumParams() == change.from_args.size())
			{
				matched_change = change;
				match_found = true;
				break;
			}
		}
		return std::make_pair(match_found, matched_change);
	}

	bool argumentsMatch(const clang::FunctionDecl* function_decl, refactorial::config::Change change)
	{
		for (unsigned i = 0; i < function_decl->getNumParams(); ++i) {
			const clang::ParmVarDecl* param = function_decl->getParamDecl(i);
			if (param->getType().getAsString() != change.from_args[i]) {
				return false;
			}
		}
		return true;
	}

	template<typename Func>
	std::vector<std::string> buildNewArguments(refactorial::config::Change change,
											   clang::SourceManager* src_manager, Func arg_getter)
	{
		std::vector<std::string> args;
		for (std::string to_arg : change.to_args) {
			int index = std::stoi(to_arg) - 1;
			const clang::Expr* arg = arg_getter(index);
			clang::SourceRange r = arg->getSourceRange();
			args.push_back(refactorial::util::sourceText(r, *src_manager));
		}
		return args;
	}

	Transform* _transform;
	refactorial::config::ArgumentChangeTransformConfig* _config;
};

//==============================================================================
void ArgumentChange::HandleTranslationUnit(clang::ASTContext& c)
{
	init();

	MatchFinder finder;

	MethodMatcherHandler HandlerForMethodMatcher(this, static_cast<refactorial::config::ArgumentChangeTransformConfig*>(getTransformConfig()));
	finder.addMatcher(cxxMemberCallExpr().bind("call"), &HandlerForMethodMatcher);
	finder.addMatcher(cxxConstructExpr().bind("ctor"), &HandlerForMethodMatcher);

	finder.matchAST(c);
}

//==============================================================================
refactorial::config::TransformConfig* ArgumentChange::getTransformConfig()
{
	return &(TransformRegistry::get().config.transforms.argument_change_transform);
}
