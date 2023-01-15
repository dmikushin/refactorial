#include "NamedDeclRemover.h"
#include "Transforms.h"

#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Lex/Lexer.h>
#include <clang/Index/USRGeneration.h>
#include <llvm/ADT/SmallVector.h>

using namespace llvm;
using namespace clang;

namespace {

#include "NamedDeclVisitor.h"

class TypeRemoveTransform : public NamedDeclVisitor<NamedDeclRemover>
{
public :

	TypeRemoveTransform() : NamedDeclVisitor<NamedDeclRemover>()
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

class FunctionRemoveTransform : public NamedDeclVisitor<NamedDeclRemover>
{
public :

	FunctionRemoveTransform() : NamedDeclVisitor<NamedDeclRemover>()
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

class RecordFieldRemoveTransform : public NamedDeclVisitor<NamedDeclRemover>
{
public :

	RecordFieldRemoveTransform() : NamedDeclVisitor<NamedDeclRemover>()
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

} // namespace

