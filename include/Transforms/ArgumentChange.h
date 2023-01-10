#ifndef ARGUMENTCHANGE_H
#define ARGUMENTCHANGE_H

#include "Transforms.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class ArgumentChange : public Transform
{
public:
	void HandleTranslationUnit(clang::ASTContext& c) override;

protected:
	refactorial::config::TransformConfig* getTransformConfig() override;
};

#endif
