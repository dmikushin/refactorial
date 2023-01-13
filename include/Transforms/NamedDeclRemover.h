#ifndef NAMED_DECL_REMOVER_H
#define NAMED_DECL_REMOVER_H

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

class NamedDeclRemover : public Transform
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
		bool checkOnly = false);

	void removeLocation(clang::SourceLocation L, clang::SourceLocation E);

	int indentLevel;
	std::string indentString;

	struct Remove
	{
		Remove(const std::string& name_, bool mangled_) :
			name(name_), mangled(mangled_) { }
	
		llvm::Regex name;
		bool mangled;
	};

	std::vector<Remove> removeList;

	std::set<const clang::Decl *> nameMap;
};

#endif // NAMED_DECL_REMOVER_H

