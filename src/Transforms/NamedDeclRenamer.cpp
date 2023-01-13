#include "NamedDeclRenamer.h"
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

void NamedDeclRenamer::loadConfig(refactorial::config::TransformConfig* transform_)
{
	auto transform = static_cast<refactorial::config::RenameConfig*>(transform_);
	for (const refactorial::config::Rename& r : transform->renames)
		renameList.push_back(RegexStringPair(llvm::Regex(r.from), r.to));
}

// If we have a NamedDecl and the fully-qualified name matches.
bool NamedDeclRenamer::nameMatches(
	const clang::NamedDecl *D,
	std::string& nameOld, std::string& nameNew,
	bool checkOnly)
{
	if (!D) return false;

	nameOld = D->getQualifiedNameAsString();
	
	auto I = nameMap.find(D);
	if (I != nameMap.end())
	{
		nameNew = (*I).second;
		return true;
	}

	// if we only need a quick look-up of renamed Decl's
	// (e.g. for renamed parent classes, overridden methods etc.)
	if (checkOnly) return false;

	if (!D->getLocation().isValid()) return false;

	if (nameOld.size() == 0) return false;

	// special handling for TagDecl
	if (auto T = llvm::dyn_cast<clang::TagDecl>(D))
	{
		auto KN = T->getKindName();
		assert(!KN.empty() && "getKindName() must return a non-empty name");
		nameOld.insert(0, KN);
		nameOld.insert(KN.size(), " ");
	}

	llvm::SmallVector<llvm::StringRef, 10> matched;
	for (auto I = renameList.begin(), E = renameList.end(); I != E; ++I)
	{
		if (I->first.match(nameOld, &matched))
		{
			nameNew = I->first.sub(I->second, nameOld);
			nameMap[D] = nameNew;
			return true;
		}
	}

	return false;
}

void NamedDeclRenamer::renameLocation(clang::SourceLocation L,
	const std::string& nameOld, const std::string& nameNew)
{
	if (!L.isValid()) return;
    
	if (L.isMacroID())
	{
		// TODO: emit error using diagnostics
		clang::SourceManager &SM = ci->getSourceManager();
		if (SM.isMacroArgExpansion(L) || SM.isInSystemMacro(L))
		{
			// see if it's the macro expansion we can handle
			// e.g.
			//   #define call(x) x
			//   call(y());   // if we want to rename y()
			L = SM.getSpellingLoc(L);

			// this falls through to the rename routine below
		}
		else
		{
			// if the spelling location is from an actual file that we can
			// touch, then do the replacement, but show a warning
			clang::SourceManager &SM = ci->getSourceManager();
			auto SL = SM.getSpellingLoc(L);
			clang::FullSourceLoc FSL(SL, SM);
			const clang::FileEntry *FE = SM.getFileEntryForID(FSL.getFileID());
			if (FE)
			{
				llvm::errs() << "Warning: Rename attempted as a result of macro "
					<< "expansion may break things, at: " << loc(L) << "\n";
				L = SL;
				// this falls through to the rename routine below
			}
			else
			{
				// cannot handle this case
				llvm::errs() << "Error: Token is resulted from macro expansion"
					" and is therefore not renamed, at: " << loc(L) << "\n";
				return;
			}
		}
	}

	if (!canChangeLocation(L)) return;

	clang::Preprocessor &P = ci->getPreprocessor();
	auto LE = P.getLocForEndOfToken(L);
	if (LE.isValid())
	{
		// getLocWithOffset returns the location *past* the token, hence -1
		auto E = LE.getLocWithOffset(-1);

		// TODO: Determine if it's a writable file

		// TODO: Determine if the location has already been touched or
		// needs skipping (such as in refactoring API user's code, then
		// the API headers need no changing since later the new API will be
		// in place)

		Replacer::instance().replace(clang::SourceRange(L, E),
			nameOld, nameNew, ci->getSourceManager());
	}
}

std::string NamedDeclRenamer::loc(clang::SourceLocation L)
{
	return L.printToString(ci->getSourceManager());
}

bool NamedDeclRenamer::setResult(const NamedDecl *Decl,
	SourceLocation Start, SourceLocation End)
{
	std::string nameOld, nameNew;
	const NamedDecl *ED = this->getEffectiveDecl(Decl);
	if (ED && nameMatches(ED, nameOld, nameNew, false))
		renameLocation(Start, nameOld, nameNew);

	return true;
}

