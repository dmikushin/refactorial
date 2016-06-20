#include "util.h"

#include <limits.h>
#include <stdlib.h>

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
		std::string absolutePath(const std::string& relative_path) {
			#ifdef WIN32
			// Entirely derived from LLVM's FileManager source (see getCanonicalName).
			llvm::SmallString<PATH_MAX> path_buffer(relative_path);
			llvm::sys::fs::make_absolute(path_buffer);
			llvm::sys::path::native(path_buffer);
			llvm::sys::path::remove_dots(path_buffer, true);
			return path_buffer.str().str();
			#else
			char resolved_path[PATH_MAX];
			realpath(relative_path.c_str(), resolved_path);
			return std::string(resolved_path);
			#endif
		}
	}
}
