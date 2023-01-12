#ifndef NAMED_DECL_RENAMER_H
#define NAMED_DECL_RENAMER_H

#include "Transforms.h"

#include <clang/Basic/SourceLocation.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <yamlreader.h>

using namespace clang;

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

	void loadConfig(refactorial::config::TransformConfig* transform);

	std::string loc(clang::SourceLocation L);

	virtual const NamedDecl *getEffectiveDecl(const NamedDecl *Decl) = 0;
	
	virtual refactorial::config::TransformConfig* getTransformConfig() override = 0;

	bool setResult(const NamedDecl *Decl,
		SourceLocation Start, SourceLocation End);
	
private :

	bool nameMatches(
		const clang::NamedDecl *decl,
		std::string &newName,
		bool checkOnly = false);

	void renameLocation(clang::SourceLocation L, std::string& N);

	int indentLevel;
	std::string indentString;

	typedef std::pair<llvm::Regex, std::string> RegexStringPair;
	std::vector<RegexStringPair> renameList;

	std::map<const clang::Decl *, std::string> nameMap;
	std::map<std::string, std::string> matchedStringMap;
	std::set<std::string> unmatchedStringSet;
};

#endif // NAMED_DECL_RENAMER_H

