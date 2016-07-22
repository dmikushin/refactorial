#include "ExplicitConstructorTransform.h"

#include "Replacer.h"

using namespace clang::ast_matchers;

REGISTER_TRANSFORM(ExplicitConstructorTransform);

//==============================================================================
class ConstructorHandler : public MatchFinder::MatchCallback
{
public:
	ConstructorHandler(Transform* transform, std::vector<std::string> ignores)
	  : _transform(transform),
		_ignores(ignores)
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

			// Nothing to do here if it is already explicit.
			if (ctor->isExplicit()) return;

			// Determine if this class is in our ignore list.
			std::string name = ctor->getQualifiedNameAsString();
			for (std::string ignore_class : _ignores) {
				if (ignore_class == name) {
					return;
				}
			}

			clang::SourceManager* src_manager = result.SourceManager;
			Replacer::instance().insert(loc, "explicit ", *src_manager);
		}
	}

private:
	Transform* _transform;
	std::vector<std::string> _ignores;
};

//==============================================================================
void ExplicitConstructorTransform::HandleTranslationUnit(clang::ASTContext& c)
{
	init();

	auto config = static_cast<refactorial::config::ExplicitConstructorTransformConfig*>(getTransformConfig());
	std::vector<std::string> ignores = config->ignores;
	ConstructorHandler HandlerForConstructor(this, ignores);

	MatchFinder finder;
	finder.addMatcher(cxxConstructorDecl(hasParent(cxxRecordDecl())).bind("ctor"), &HandlerForConstructor);

	finder.matchAST(c);
}

//==============================================================================
refactorial::config::TransformConfig* ExplicitConstructorTransform::getTransformConfig()
{
	return &(TransformRegistry::get().config.transforms.explicit_constructor_transform);
}
