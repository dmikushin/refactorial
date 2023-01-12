#include "Remover.h"
#include "Transforms.h"

using namespace clang;
using namespace clang::tooling;
using namespace std;

void Remover::remove(SourceRange range, SourceManager& sourceManager)
{
	TransformRegistry::get().replacements->push_back(Replacement(sourceManager, CharSourceRange(range, true), ""));
}

void Remover::removeText(SourceRange range, SourceManager& sourceManager)
{
	TransformRegistry::get().replacements->push_back(Replacement(sourceManager, CharSourceRange(range, false), ""));
}

