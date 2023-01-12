#include "NamedDeclRenamer.h"
#include "Transforms.h"
#include "NamedDeclVisitor.h"

#include <llvm/ADT/SmallVector.h>

using namespace llvm;
using namespace clang;

class TypeRemoveTransform : public NamedDeclVisitor
{
public :

	TypeRemoveTransform() : NamedDeclVisitor()
	{
		init();
	}

	refactorial::config::TransformConfig* getTransformConfig() override
	{
		return &(TransformRegistry::get().config.transforms.type_rename_transform);
	}

	const NamedDecl *getEffectiveDecl(const NamedDecl *Decl) override
	{
		if (dyn_cast<TypeDecl>(Decl) ||
			dyn_cast<ClassTemplateDecl>(Decl) ||
			dyn_cast<TypeAliasTemplateDecl>(Decl))
			return Decl;

		if (dyn_cast<CXXConstructorDecl>(Decl))
			return dyn_cast<CXXMethodDecl>(Decl)->getParent();

		return 0;
	}
};

class FunctionRemoveTransform : public NamedDeclVisitor
{
public :

	FunctionRemoveTransform() : NamedDeclVisitor()
	{
		init();
	}

	refactorial::config::TransformConfig* getTransformConfig() override
	{
		return &(TransformRegistry::get().config.transforms.function_remove_transform);
	}

	const NamedDecl *getEffectiveDecl(const NamedDecl *Decl) override
	{
		if ((dyn_cast<FunctionDecl>(Decl) &&
			!dyn_cast<CXXConstructorDecl>(Decl) &&
			!dyn_cast<CXXDestructorDecl>(Decl) &&
			!dyn_cast<CXXConversionDecl>(Decl)) ||
			dyn_cast<FunctionTemplateDecl>(Decl))
			return Decl;

		return 0;
	}
};

class RecordFieldRemoveTransform : public NamedDeclVisitor
{
public :

	RecordFieldRemoveTransform() : NamedDeclVisitor()
	{
		init();
	}

	refactorial::config::TransformConfig* getTransformConfig() override
	{
		return &(TransformRegistry::get().config.transforms.record_field_rename_transform);
	}

	const NamedDecl *getEffectiveDecl(const NamedDecl *Decl) override
	{
		return dyn_cast<FieldDecl>(Decl);
	}
};

REGISTER_TRANSFORM(TypeRemoveTransform);
REGISTER_TRANSFORM(FunctionRemoveTransform);
REGISTER_TRANSFORM(RecordFieldRemoveTransform);

}

