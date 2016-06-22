#ifndef REPLACER_H
#define REPLACER_H

#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/SourceManager.h>

//==============================================================================
class Replacer
{

public:
	static Replacer& instance()
	{
		static Replacer instance;
		return instance;
  }

  void insert(clang::SourceLocation loc, std::string text, clang::SourceManager& sourceManager);
  void replace(clang::SourceRange range, std::string text, clang::SourceManager& sourceManager);
  void replaceText(clang::SourceRange range, std::string text, clang::SourceManager& sourceManager);

protected:
	clang::CompilerInstance *ci;

private:
	Replacer() {}
  Replacer(const Replacer&) {}
	void operator=(const Replacer&) {}
};

#endif

