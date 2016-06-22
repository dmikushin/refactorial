#include "Transforms.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class ExplicitConstructorTransform : public Transform
{
public:
	void HandleTranslationUnit(clang::ASTContext& c) override;
};
