#ifndef RENAME_TRANSFORMS_H
#define RENAME_TRANSFORMS_H

#include "Transforms.h"

#include <clang/Basic/SourceLocation.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <yamlreader.h>

namespace clang {
	
class CompilerInstance;
class Decl;
class NamedDecl;
class Stmt;

} // namespace clang

namespace llvm {

class Regex;

} // namespace llvm

class NamedDeclRenamer : public Transform
{
public :

	void loadConfig(refactorial::config::RenameConfig* transform);

	bool nameMatches(
		const clang::NamedDecl *decl,
		std::string &newName,
		bool checkOnly = false);

	bool stringMatches(std::string name, std::string &outNewName);

	bool stmtInSameFileAsDecl(clang::Stmt *S, clang::Decl *D);

	void renameLocation(clang::SourceLocation L, std::string& N);

	const std::string& indent();
	void pushIndent();
	void popIndent();

	std::string loc(clang::SourceLocation L);
	std::string range(clang::SourceRange R);

private :

	int indentLevel;
	std::string indentString;

	typedef std::pair<llvm::Regex, std::string> RegexStringPair;
	std::vector<RegexStringPair> renameList;

	std::map<const clang::Decl *, std::string> nameMap;
	std::map<std::string, std::string> matchedStringMap;
	std::set<std::string> unmatchedStringSet;
};

#endif // RENAME_TRANSFORMS_H

