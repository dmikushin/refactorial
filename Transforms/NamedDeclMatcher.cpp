#include "NamedDeclMatcher.h"
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

bool
NamedDeclMatcher::loadConfig(refactorial::config::RenameConfig* transform)
{
	llvm::outs() << "found type translations" << "\n";
	for (const refactorial::config::Rename& r : transform->renames)
	{
		renameList.push_back(RegexStringPair(llvm::Regex(r.from), r.to));
		llvm::errs() << "from: " << r.from << " to: " << r.to << "\n";
	}

	return true;
}

  // if we have a NamedDecl and the fully-qualified name matches
bool
NamedDeclMatcher::nameMatches(
    const clang::NamedDecl *D,
    std::string &outNewName,
    bool checkOnly)
{
    if (!D) {
      return false;
    }

    auto I = nameMap.find(D);
    if (I != nameMap.end()) {
      outNewName = (*I).second;
      return true;
    }

    // if we only need a quick look-up of renamed Decl's
    // (e.g. for renamed parent classes, overridden methods etc.)
    if (checkOnly) {
      return false;
    }

    if (!D->getLocation().isValid()) {
      return false;
    }

    auto QN = D->getQualifiedNameAsString();
    if (QN.size() == 0) {
      return false;
    }

    // special handling for TagDecl
    if (auto T = llvm::dyn_cast<clang::TagDecl>(D)) {
      auto KN = T->getKindName();
      assert(KN && "getKindName() must return a non-NULL value");
      QN.insert(0, KN);
      QN.insert(KN.size(), " ");
    }

    llvm::SmallVector<llvm::StringRef, 2> matched;
    for (auto I = renameList.begin(), E = renameList.end(); I != E; ++I) {
      if (I->first.match(QN, &matched)) {
        std::string err;
        outNewName = I->first.sub(I->second, QN, &err);
        nameMap[D] = outNewName;
        return true;
      }
    }
    return false;
}

  // useful when we can't just rely on Decl, e.g. built-in type
  // unmatched names are cached to speed things up
bool
NamedDeclMatcher::stringMatches(std::string name, std::string &outNewName)
{
    auto I = matchedStringMap.find(name);
    if (I != matchedStringMap.end()) {
      outNewName = (*I).second;
      return true;
    }

    auto J = unmatchedStringSet.find(name);
    if (J != unmatchedStringSet.end()) {
      return false;
    }

	llvm::SmallVector<llvm::StringRef, 2> matched;
    for (auto I = renameList.begin(), E = renameList.end(); I != E; ++I) {
	  if (I->first.match(name, &matched)) {
        std::string newName = matched[0];
        matchedStringMap[name] = newName;
        outNewName = newName;
        return true;
      }
    }

    unmatchedStringSet.insert(name);
    return false;
}

bool
NamedDeclMatcher::stmtInSameFileAsDecl(clang::Stmt *S, clang::Decl *D)
{
    return ci->getSourceManager().isWrittenInSameFile(
        S->getLocStart(),
        D->getLocation());
}

void
NamedDeclMatcher::renameLocation(clang::SourceLocation L, std::string& N)
{
    if (L.isValid()) {
      if (L.isMacroID()) {
        // TODO: emit error using diagnostics
        clang::SourceManager &SM = ci->getSourceManager();
        if (SM.isMacroArgExpansion(L) || SM.isInSystemMacro(L)) {
          // see if it's the macro expansion we can handle
          // e.g.
          //   #define call(x) x
          //   call(y());   // if we want to rename y()
          L = SM.getSpellingLoc(L);

          // this falls through to the rename routine below
        }
        else {
          // if the spelling location is from an actual file that we can
          // touch, then do the replacement, but show a warning
          clang::SourceManager &SM = ci->getSourceManager();
          auto SL = SM.getSpellingLoc(L);
          clang::FullSourceLoc FSL(SL, SM);
          const clang::FileEntry *FE = SM.getFileEntryForID(FSL.getFileID());
          if (FE) {
            llvm::errs() << "Warning: Rename attempted as a result of macro "
                         << "expansion may break things, at: " << loc(L) << "\n";
            L = SL;
            // this falls through to the rename routine below
          }
          else {
            // cannot handle this case
            llvm::errs() << "Error: Token is resulted from macro expansion"
              " and is therefore not renamed, at: " << loc(L) << "\n";
            return;
          }
        }
      }

      if (!canChangeLocation(L)) {
        return;
      }

      clang::Preprocessor &P = ci->getPreprocessor();
      auto LE = P.getLocForEndOfToken(L);
      if (LE.isValid()) {

        // getLocWithOffset returns the location *past* the token, hence -1
        auto E = LE.getLocWithOffset(-1);

        // TODO: Determine if it's a writable file

        // TODO: Determine if the location has already been touched or
        // needs skipping (such as in refactoring API user's code, then
        // the API headers need no changing since later the new API will be
        // in place)

        Replacer::instance().replace(clang::SourceRange(L, E), N, ci->getSourceManager());
      }
    }
}

const std::string &
NamedDeclMatcher::indent()
{
    return indentString;
}

void
NamedDeclMatcher::pushIndent()
{
    indentLevel++;
    indentString.resize(indentString.size() + 2, ' ');
}

void
NamedDeclMatcher::popIndent()
{
    assert(indentLevel >= 0 && "indentLevel must be >= 0");
    indentLevel--;
    indentString.resize(indentString.size() - 2);
}

std::string
NamedDeclMatcher::loc(clang::SourceLocation L)
{
    return L.printToString(ci->getSourceManager());
}

std::string
NamedDeclMatcher::range(clang::SourceRange R)
{
    std::string src;
    llvm::raw_string_ostream sst(src);
    sst << "(";
    R.getBegin().print(sst, ci->getSourceManager());
    sst << ", ";
    R.getEnd().print(sst, ci->getSourceManager());
    sst << ")";
    return sst.str();
}
