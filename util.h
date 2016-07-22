#ifndef UTIL_H
#define UTIL_H

#include <iostream>

#include <llvm/ADT/StringRef.h>
#include <clang/AST/AST.h>

namespace refactorial
{
	namespace util
	{
		std::string absolutePath(const std::string& relative_path);

		llvm::StringRef sourceText(clang::SourceRange& range, clang::SourceManager& source_manager);

		std::pair<std::string, std::vector<std::string>> parseSignature(const std::string& signature);
		std::string joinStrings(std::vector<std::string> strings, std::string delimiter);

		std::vector<std::string> convertTypeNamesForSource(std::vector<std::string> types);
		std::vector<std::string> convertTypeNamesForClang(std::vector<std::string> types);
	}
}

#endif
