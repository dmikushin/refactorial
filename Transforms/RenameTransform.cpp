#include "NamedDeclMatcher.h"
#include "Transforms.h"

#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Lex/Lexer.h>
#include <clang/Index/USRGeneration.h>
#include <llvm/ADT/SmallVector.h>

using namespace llvm;
using namespace clang;

// NamedDeclFindingASTVisitor recursively visits each AST node to find the
// symbol underneath the cursor.
// FIXME: move to seperate .h/.cc file if this gets too large.
namespace {
class NamedDeclVisitor :
    public Transform,
    public RecursiveASTVisitor<NamedDeclVisitor> {
private:
  std::string TransformKey;
  std::string RenameKey;
  NamedDeclMatcher Matcher;

public:
    NamedDeclVisitor(const char *TransformKey, const char *RenameKey)
      : TransformKey(TransformKey), RenameKey(RenameKey)
    {}

    void HandleTranslationUnit(ASTContext &C) override {
      Matcher.setCompilerInstance(*this->ci);
      if (!Matcher.loadConfig(TransformKey, RenameKey)) {
        return;
      }
      this->TraverseDecl(C.getTranslationUnitDecl());
    }

  // Declaration visitors:

  // \brief Checks if the point falls within the NamedDecl. This covers every
  // declaration of a named entity that we may come across. Usually, just
  // checking if the point lies within the length of the name of the declaration
  // and the start location is sufficient.
  bool VisitNamedDecl(const NamedDecl *Decl) {
    return setResult(Decl, Decl->getLocation(),
                     Decl->getNameAsString().length());
  }

  // Expression visitors:

  bool VisitDeclRefExpr(const DeclRefExpr *Expr) {
    // Check the namespace specifier first.
    if (!checkNestedNameSpecifierLoc(Expr->getQualifierLoc()))
      return false;

    const auto *Decl = Expr->getFoundDecl();
    return setResult(Decl, Expr->getLocation(),
                     Decl->getNameAsString().length());
  }

  bool VisitMemberExpr(const MemberExpr *Expr) {
    const auto *Decl = Expr->getFoundDecl().getDecl();
    return setResult(Decl, Expr->getMemberLoc(),
                     Decl->getNameAsString().length());
  }

  bool VisitTypeLoc(TypeLoc TL) {
    const NamedDecl *Decl = nullptr;
    switch(TL.getTypeLocClass()) {
      // TODO case TypeLoc::ObjCObject:

      case TypeLoc::InjectedClassName: {
        if (auto TSTL = TL.getAs<InjectedClassNameTypeLoc>()) {
          Decl = TSTL.getDecl();
        }
        break;
      }

      case TypeLoc::TemplateSpecialization: {
        if (auto TSTL = TL.getAs<TemplateSpecializationTypeLoc>()) {
          if (auto TT = dyn_cast<TemplateSpecializationType>(TL.getTypePtr())) {
            if (auto TD = TT->getTemplateName().getAsTemplateDecl()) {
              Decl = TD->getTemplatedDecl();
            }
          }
        }
        break;
      }

      case TypeLoc::Typedef: {
        if (auto TDT = dyn_cast<TypedefType>(TL.getTypePtr())) {
          Decl = TDT->getDecl();
        }
        break;
      }

      case TypeLoc::Builtin:
      case TypeLoc::Enum:
      case TypeLoc::Record:
      case TypeLoc::ObjCInterface:
      case TypeLoc::TemplateTypeParm: {
        if (auto TT = dyn_cast<TagType>(TL.getTypePtr())) {
          Decl  = TT->getDecl();
        }
        break;
      }

      default: break;
    }
    if (Decl) {
        return setResult(Decl, TL.getBeginLoc(), TL.getEndLoc());
    }
    return true;
  }

  virtual bool acceptsDecl(const NamedDecl *Decl) = 0;

private:
  // \brief Determines if a namespace qualifier contains the point.
  // \returns false on success and sets Result.
  bool checkNestedNameSpecifierLoc(NestedNameSpecifierLoc NameLoc) {
    while (NameLoc) {
      const auto *Decl = NameLoc.getNestedNameSpecifier()->getAsNamespace();
      if (Decl && !setResult(Decl, NameLoc.getLocalBeginLoc(),
                             Decl->getNameAsString().length()))
        return false;
      NameLoc = NameLoc.getPrefix();
    }
    return true;
  }

  bool setResult(const NamedDecl *Decl,
                 SourceLocation Start,
                 SourceLocation End) {
    std::string NewName;
    if (this->acceptsDecl(Decl) &&
        Matcher.nameMatches(Decl, NewName, false)) {
      Matcher.renameLocation(Start, NewName);
    }
    return true;
  }

  bool setResult(const NamedDecl *Decl, SourceLocation Loc,
                 unsigned Offset) {
    // FIXME: Add test for Offset == 0. Add test for Offset - 1 (vs -2 etc).
    return Offset == 0 ||
           setResult(Decl, Loc, Loc.getLocWithOffset(Offset - 1));
  }
};

class TypeRenameTransform : public NamedDeclVisitor {
public:
  TypeRenameTransform() : NamedDeclVisitor("TypeRename", "Types") {}

  bool acceptsDecl(const NamedDecl *Decl) override {
    return dyn_cast<TypeDecl>(Decl) ||
           dyn_cast<ClassTemplateDecl>(Decl) ||
           dyn_cast<TypeAliasTemplateDecl>(Decl);
  }
};

class FunctionRenameTransform : public NamedDeclVisitor {
public:
  FunctionRenameTransform() : NamedDeclVisitor("FunctionRename", "Functions") {}

  bool acceptsDecl(const NamedDecl *Decl) override {
    return (dyn_cast<FunctionDecl>(Decl) &&
           !dyn_cast<CXXConstructorDecl>(Decl) &&
           !dyn_cast<CXXDestructorDecl>(Decl) &&
           !dyn_cast<CXXConversionDecl>(Decl)) ||
           dyn_cast<FunctionTemplateDecl>(Decl);
  }
};

class RecordFieldRenameTransform : public NamedDeclVisitor {
public:
  RecordFieldRenameTransform() : NamedDeclVisitor("RecordFieldRename", "Field") {}

  bool acceptsDecl(const NamedDecl *Decl) override {
    return dyn_cast<FieldDecl>(Decl);
  }
};

REGISTER_TRANSFORM(TypeRenameTransform);
REGISTER_TRANSFORM(FunctionRenameTransform);
REGISTER_TRANSFORM(RecordFieldRenameTransform);
}
