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
		if (const clang::CallExpr* call = result.Nodes.getNodeAs<clang::CallExpr>("call")) {
			clang::SourceLocation loc = call->getBeginLoc();
			if (!_transform->canChangeLocation(loc)) {
				return;
			}

			const clang::FunctionDecl* fn_decl = 0;
			if (const clang::CXXMemberCallExpr* member_call = llvm::dyn_cast<clang::CXXMemberCallExpr>(call)) {
				fn_decl = member_call->getMethodDecl();
			} else {
				fn_decl = call->getDirectCallee();
			}

			if (!fn_decl) return;

			std::pair<bool, refactorial::config::Change> match = findMatchingChange(fn_decl);
			bool match_found = match.first;
			refactorial::config::Change matched_change = match.second;
			if (!match_found) return;

			if (!argumentsMatch(fn_decl, matched_change)) return;

			auto args = getArgumentSource(result, call->getNumArgs(), [&call](int index) {
					return call->getArg(index);
				});

			std::string new_src = generateNewSource(matched_change, args);

			clang::SourceRange range;
			if (llvm::isa<clang::CXXMemberCallExpr>(call)) {
				range = clang::SourceRange(call->getArg(0)->getExprLoc(), call->getRParenLoc().getLocWithOffset(-1));
			} else {
				range = call->getSourceRange();
			}

			Replacer::instance().replace(range, "", new_src, *(result.SourceManager));

		} else if (const clang::CXXConstructExpr* ctor = result.Nodes.getNodeAs<clang::CXXConstructExpr>("ctor")) {
			clang::SourceLocation loc = ctor->getBeginLoc();
			if (!_transform->canChangeLocation(loc)) {
				return;
			}

			std::pair<bool, refactorial::config::Change> match = findMatchingChange(ctor->getConstructor());
			bool match_found = match.first;
			refactorial::config::Change matched_change = match.second;
			if (!match_found) return;

			if (!argumentsMatch(ctor->getConstructor(), matched_change)) return;

			auto args = getArgumentSource(result, ctor->getNumArgs(), [&ctor](int index) {
					return ctor->getArg(index);
				});

			auto new_src = generateNewSource(matched_change, args);
			clang::SourceRange range(ctor->getArg(0)->getExprLoc(), ctor->getParenOrBraceRange().getEnd().getLocWithOffset(-1));
			Replacer::instance().replace(range, "", new_src, *(result.SourceManager));
		}
	}

private:
	std::pair<bool, refactorial::config::Change> findMatchingChange(const clang::FunctionDecl* function_decl)
	{
		std::string name = function_decl->getQualifiedNameAsString();
		int param_count = function_decl->getNumParams();

		bool match_found = false;
		refactorial::config::Change matched_change;
		for (refactorial::config::Change change : _config->changes) {
			if (name == change.from_function && param_count == change.from_args.size())
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
	std::vector<llvm::StringRef> getArgumentSource(const MatchFinder::MatchResult& result, int arg_count,
											   Func arg_getter)
	{
		std::vector<llvm::StringRef> args;
		for (int i = 0; i < arg_count; ++i) {
			const clang::Expr* arg = arg_getter(i);
			clang::SourceRange range = arg->getSourceRange();
			auto src = refactorial::util::sourceText(range, *(result.SourceManager));
			args.push_back(src);
		}
		return args;
	}

	std::string generateNewSource(const refactorial::config::Change& change, const std::vector<llvm::StringRef>& args)
	{
		std::string new_src(change.to);
		for (int i = 0; i < args.size(); ++i) {
			llvm::Regex re("\\$" + std::to_string(i + 1));
			new_src = re.sub(args[i], new_src);
		}
		return new_src;
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
	finder.addMatcher(callExpr().bind("call"), &HandlerForMethodMatcher);
	finder.addMatcher(cxxConstructExpr().bind("ctor"), &HandlerForMethodMatcher);

	finder.matchAST(c);
}

//==============================================================================
refactorial::config::TransformConfig* ArgumentChange::getTransformConfig()
{
	return &(TransformRegistry::get().config.transforms.argument_change_transform);
}
