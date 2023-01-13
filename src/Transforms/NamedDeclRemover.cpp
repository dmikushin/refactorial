#include "NamedDeclRemover.h"
#include "Transforms.h"
#include "Replacer.h"

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include "clang/AST/Mangle.h"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/raw_ostream.h>

using namespace clang;

static std::string getMangledName(const NamedDecl* decl)
{
	auto& context = decl->getASTContext();
	auto mangleContext = context.createMangleContext();

	if (!mangleContext->shouldMangleDeclName(decl))
		return decl->getQualifiedNameAsString();

	std::string mangledName;
	llvm::raw_string_ostream ostream(mangledName);

	mangleContext->mangleName(decl, ostream);

	ostream.flush();

	delete mangleContext;

	return mangledName;
};

void NamedDeclRemover::loadConfig(refactorial::config::TransformConfig* transform_)
{
	auto transform = static_cast<refactorial::config::RemoveConfig*>(transform_);
	for (const auto& r : transform->removes)
		removeList.emplace_back(r.name, r.mangled);
}

  // if we have a NamedDecl and the fully-qualified name matches
bool NamedDeclRemover::nameMatches(
	const clang::NamedDecl *D,
	bool checkOnly)
{
	if (!D) return false;

	if (nameMap.find(D) != nameMap.end())
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
	for (auto& I : removeList)
	{
		if (I.name.match(I.mangled ? getMangledName(D) : QN, &matched))
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
		else if (const FunctionTemplateDecl *FTD = dyn_cast<const FunctionTemplateDecl>(Decl))
			removeLocation(FTD->getSourceRange().getBegin(), FTD->getSourceRange().getEnd());
		else
			fprintf(stderr, "Unsupported declaration kind: \"%s\"\n", Decl->getDeclKindName());
	}

	return true;
}

