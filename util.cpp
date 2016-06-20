#include "util.h"

namespace refactorial
{
	namespace util
	{
		std::string absolutePath(const std::string& relative_path) {
			char resolved_path[PATH_MAX];
			realpath(relative_path.c_str(), resolved_path);
			return std::string(resolved_path);
		}
	}
}
