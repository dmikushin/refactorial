#include "NamedDeclMatcher.h"
#include "Transforms.h"
#include "NamedDeclVisitor.h"

#include <llvm/ADT/SmallVector.h>

using namespace llvm;
using namespace clang;

class TypeRenameTransform : public NamedDeclVisitor {
public:
  TypeRenameTransform() : NamedDeclVisitor() {
	  init();
  }

	refactorial::config::TransformConfig* getTransformConfig() override {
		return &(TransformRegistry::get().config.transforms.type_rename_transform);
	}

  const NamedDecl *getEffectiveDecl(const NamedDecl *Decl) override {
    if (dyn_cast<TypeDecl>(Decl) ||
        dyn_cast<ClassTemplateDecl>(Decl) ||
        dyn_cast<TypeAliasTemplateDecl>(Decl)) {
      return Decl;
    }
    if (dyn_cast<CXXConstructorDecl>(Decl)) {
      return dyn_cast<CXXMethodDecl>(Decl)->getParent();
    }
    return 0;
  }
};

class FunctionRenameTransform : public NamedDeclVisitor {
public:
  FunctionRenameTransform() : NamedDeclVisitor() {
	  init();
  }

	refactorial::config::TransformConfig* getTransformConfig() override {
		return &(TransformRegistry::get().config.transforms.function_rename_transform);
	}

  const NamedDecl *getEffectiveDecl(const NamedDecl *Decl) override {
    if ((dyn_cast<FunctionDecl>(Decl) &&
        !dyn_cast<CXXConstructorDecl>(Decl) &&
        !dyn_cast<CXXDestructorDecl>(Decl) &&
        !dyn_cast<CXXConversionDecl>(Decl)) ||
        dyn_cast<FunctionTemplateDecl>(Decl)) {
      return Decl;
    }
    return 0;
  }
};

class RecordFieldRenameTransform : public NamedDeclVisitor {
public:
  RecordFieldRenameTransform() : NamedDeclVisitor() {
	  init();
  }

	refactorial::config::TransformConfig* getTransformConfig() override {
		return &(TransformRegistry::get().config.transforms.record_field_rename_transform);
	}

  const NamedDecl *getEffectiveDecl(const NamedDecl *Decl) override {
    return dyn_cast<FieldDecl>(Decl);
  }
};

REGISTER_TRANSFORM(TypeRenameTransform);
REGISTER_TRANSFORM(FunctionRenameTransform);
REGISTER_TRANSFORM(RecordFieldRenameTransform);
}
