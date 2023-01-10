#include "util.h"

#include <limits.h>
#include <stdlib.h>

#include <clang/Lex/Lexer.h>

#ifdef WIN32
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include <windows.h>

#define PATH_MAX MAX_PATH
#endif

namespace refactorial
{
	namespace util
	{
		std::string absolutePath(const llvm::StringRef& relative_path) {
			#ifdef WIN32
			// Entirely derived from LLVM's FileManager source (see getCanonicalName).
			llvm::SmallString<PATH_MAX> path_buffer(relative_path);
			llvm::sys::fs::make_absolute(path_buffer);
			llvm::sys::path::native(path_buffer);
			llvm::sys::path::remove_dots(path_buffer, true);
			return path_buffer.str().str();
			#else
			char resolved_path[PATH_MAX];
			realpath(relative_path.str().c_str(), resolved_path);
			return std::string(resolved_path);
			#endif
		}

		llvm::StringRef sourceText(clang::SourceRange& range, clang::SourceManager& source_manager)
		{
			return clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(range),
											   source_manager, clang::LangOptions(), 0);
		}

		std::pair<llvm::StringRef, std::vector<std::string>> parseSignature(const std::string& signature)
		{
			std::pair<llvm::StringRef, llvm::StringRef> from_parts = llvm::StringRef(signature).split("(");

			// Split args out and drop the closing paren.
			llvm::SmallVector<llvm::StringRef, 0> raw_args;
			from_parts.second.drop_back(1).split(raw_args, ", ", -1, false);

			std::vector<std::string> args;
			for (llvm::StringRef arg : raw_args) {
				args.push_back(arg.str());
			}

			return std::make_pair(from_parts.first, args);
		}

		std::string joinStrings(std::vector<std::string> strings, std::string delimiter)
		{
			std::string s;
			for (std::vector<std::string>::const_iterator itr = strings.begin(); itr != strings.end(); ++itr)
			{
				s += (*itr);

				if (itr + 1 != strings.end()) {
					s += delimiter;
				}
			}
			return s;
		}

		std::vector<std::string> convertTypeNamesForSource(std::vector<std::string> types)
		{
			std::vector<std::string> new_types;
			for (std::string type : types) {
				if (type == "_Bool") {
					new_types.push_back("bool");
				} else {
					new_types.push_back(type);
				}
			}
			return new_types;
		}

		std::vector<std::string> convertTypeNamesForClang(std::vector<std::string> types)
		{
			// Handles the differences in what types look like in source vs how clang "sees" them.
			std::vector<std::string> new_types;
			for (std::string type : types) {
				if (type == "bool") {
					new_types.push_back("_Bool");
				} else {
					new_types.push_back(type);
				}
			}
			return new_types;
		}
	}
}
