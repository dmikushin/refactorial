#include "Replacer.h"
#include "Transforms.h"

using namespace clang;
using namespace clang::tooling;
using namespace std;

void Replacer::insert(SourceLocation loc, string text, SourceManager& sourceManager)
{
	TransformRegistry::get().replacements->push_back(Replacement(sourceManager, loc, 0, text));
}

void Replacer::replace(SourceRange range, string text, SourceManager& sourceManager)
{
	TransformRegistry::get().replacements->push_back(Replacement(sourceManager, CharSourceRange(range, true), text));
}

void Replacer::replaceText(SourceRange range, string text, SourceManager& sourceManager)
{
	TransformRegistry::get().replacements->push_back(Replacement(sourceManager, CharSourceRange(range, false), text));
}

