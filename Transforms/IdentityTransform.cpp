//
// IdentityTransform.cpp
//

#include "Transforms.h"

using namespace clang;

class IdentityTransform : public Transform {
public:

  virtual void HandleTranslationUnit(ASTContext &);
};

REGISTER_TRANSFORM(IdentityTransform);


void IdentityTransform::HandleTranslationUnit(ASTContext &)
{
}
