#include "NamedDeclRemover.h"
#include "Transforms.h"
#include "Replacer.h"

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/raw_ostream.h>

using namespace clang;

void NamedDeclRemover::loadConfig(refactorial::config::TransformConfig* transform_)
{
	auto transform = static_cast<refactorial::config::RemoveConfig*>(transform_);
	for (const auto& r : transform->removes)
		removeList.push_back(llvm::Regex(r));
}

  // if we have a NamedDecl and the fully-qualified name matches
bool NamedDeclRemover::nameMatches(
	const clang::NamedDecl *D,
	bool checkOnly)
{
	if (!D) return false;

	auto I = nameMap.find(D);
	if (I != nameMap.end())
		return true;

	// if we only need a quick look-up of renamed Decl's
	// (e.g. for renamed parent classes, overridden methods etc.)
	if (checkOnly) return false;

	if (!D->getLocation().isValid()) return false;

	auto QN = D->getQualifiedNameAsString();
	if (QN.size() == 0) return false;

	// special handling for TagDecl
	if (auto T = llvm::dyn_cast<clang::TagDecl>(D))
	{
		auto KN = T->getKindName();
		assert(!KN.empty() && "getKindName() must return a non-empty name");
		QN.insert(0, KN);
		QN.insert(KN.size(), " ");
	}

	llvm::SmallVector<llvm::StringRef, 10> matched;
	for (auto I = removeList.begin(), E = removeList.end(); I != E; ++I)
	{
		if (I->match(QN, &matched))
		{
			nameMap.insert(D);
			return true;
		}
	}

	return false;
}

void NamedDeclRemover::removeLocation(clang::SourceLocation L, clang::SourceLocation E)
{
	if (!L.isValid()) return;
    
	if (L.isMacroID()) return;

	if (!canChangeLocation(L)) return;

	Replacer::instance().replace(clang::SourceRange(L, E), "", ci->getSourceManager());
}

std::string NamedDeclRemover::loc(clang::SourceLocation L)
{
	return L.printToString(ci->getSourceManager());
}

bool NamedDeclRemover::setResult(const NamedDecl *Decl,
	SourceLocation Start, SourceLocation End)
{
	const NamedDecl *ED = this->getEffectiveDecl(Decl);
	if (ED && nameMatches(ED, false))
	{
		if (const FunctionDecl *FD = dyn_cast<const FunctionDecl>(Decl))
			removeLocation(FD->getSourceRange().getBegin(), FD->getSourceRange().getEnd());
		else
			printf("Sorry, not a FunctionDecl, kind = \"%s\"\n", Decl->getDeclKindName());
	}

	return true;
}

