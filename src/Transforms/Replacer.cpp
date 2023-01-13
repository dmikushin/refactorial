#include "Replacer.h"
#include "Transforms.h"

#include <unistd.h>

using namespace clang;
using namespace clang::tooling;
using namespace std;

static bool fileIsWritable(const llvm::StringRef& filename)
{
	return access(filename.str().c_str(), W_OK) == 0;
}

void Replacer::insert(const SourceLocation& loc, const string& text, SourceManager& sourceManager)
{
	Replacement r(sourceManager, loc, 0, text);
	if (fileIsWritable(r.getFilePath()))
		TransformRegistry::get().replacements->push_back(r);
}

void Replacer::replace(const SourceRange& range, const string& text, SourceManager& sourceManager)
{
	Replacement r(sourceManager, CharSourceRange(range, true), text);
	if (fileIsWritable(r.getFilePath()))
		TransformRegistry::get().replacements->push_back(r);
}

void Replacer::replaceText(const SourceRange& range, const string& text, SourceManager& sourceManager)
{
	Replacement r(sourceManager, CharSourceRange(range, false), text);
	if (fileIsWritable(r.getFilePath()))
		TransformRegistry::get().replacements->push_back(r);
}

