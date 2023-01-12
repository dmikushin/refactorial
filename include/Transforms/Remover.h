#ifndef REMOVER_H
#define REMOVER_H

#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/SourceManager.h>

class Remover
{
public :

	static Remover& instance()
	{
		static Remover instance;
		return instance;
	}

	void remove(clang::SourceRange range, clang::SourceManager& sourceManager);
	void removeText(clang::SourceRange range, clang::SourceManager& sourceManager);

protected :

	clang::CompilerInstance *ci;

private :

	Remover() {}
	Remover(const Remover&) {}
	void operator=(const Remover&) {}
};

#endif // REMOVER_H

