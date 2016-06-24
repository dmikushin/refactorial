#ifndef QT3TO5UICLASSES_H
#define QT3TO5UICLASSES_H

#include "Transforms.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class Qt3To5UIClasses : public Transform
{
public:
	void HandleTranslationUnit(clang::ASTContext& c) override;

protected:
	refactorial::config::TransformConfig* getTransformConfig() override;
};

#endif
