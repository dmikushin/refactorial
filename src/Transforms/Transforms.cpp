#include "Transforms.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>

#include <stdexcept>

using namespace clang;
using namespace clang::tooling;
using namespace std;

void Transform::init()
{
	refactorial::config::TransformConfig* config = getTransformConfig();
	for (const std::string& path : config->within_paths)
		addAllowedPath(path);
}

clang::SourceLocation Transform::findLocAfterToken(
	clang::SourceLocation curLoc, clang::tok::TokenKind tok)
{
	return clang::Lexer::findLocationAfterToken(
		curLoc, tok, ci->getSourceManager(), ci->getLangOpts(), true);
}

clang::SourceLocation Transform::getLocForEndOfToken(clang::SourceLocation curLoc)
{
	return clang::Lexer::getLocForEndOfToken(
		curLoc, 0, ci->getSourceManager(), ci->getLangOpts());
}

clang::SourceLocation Transform::findLocAfterSemi(clang::SourceLocation curLoc)
{
	return findLocAfterToken(curLoc, clang::tok::semi);
}

void Transform::addAllowedPath(const std::string& path)
{
	std::string regex = llvm::Regex::escape(path);
	allowedDirectoryList.push_back(llvm::Regex(regex + ".*", llvm::Regex::IgnoreCase));
}

bool Transform::canChangeLocation(const clang::SourceLocation& loc)
{
	if (!loc.isValid()) return false;

	if (allowedDirectoryList.empty()) return true;

	clang::SourceManager &SM = ci->getSourceManager();
	clang::FullSourceLoc FSL(loc, SM);
	const clang::FileEntry *FE = SM.getFileEntryForID(FSL.getFileID());
	if (!FE)
	{
		// attempt to get the spelling location
		auto SL = SM.getSpellingLoc(loc);
		if (!SL.isValid()) return false;

		clang::FullSourceLoc FSL2(SL, SM);
		FE = SM.getFileEntryForID(FSL2.getFileID());
		if (!FE) return false;
	}

	auto absolute_name = refactorial::util::absolutePath(FE->getName());
	for (auto I = allowedDirectoryList.begin(), E = allowedDirectoryList.end(); I != E; ++I)
		if (I->match(absolute_name))
			return true;

	return false;
}

TransformRegistry &TransformRegistry::get()
{
	static TransformRegistry instance;
	return instance;
}

// FIXME: TransformRegistry and TransformAction should live somewhere else.
// Probably in a different header?
void TransformRegistry::add(const string &name, transform_creator creator)
{
	m_transforms.insert(pair<string, transform_creator>(name, creator));
}

transform_creator TransformRegistry::operator[](const string &name) const
{
	auto iter = m_transforms.find(name);
	if (iter == m_transforms.end())
		llvm::errs() << name << " out of range" << "\n";

	return iter->second;
}

class TransformAction : public ASTFrontendAction
{
	transform_creator tcreator;

public :
	
	TransformAction(transform_creator creator) : tcreator(creator) { }

protected :

	std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef)
	{
		std::unique_ptr<Transform> xform = tcreator();
		xform->ci = &CI;
		return std::move(xform);
	}
};

TransformFactory::TransformFactory(transform_creator creator) : tcreator(creator) { }

std::unique_ptr<FrontendAction> TransformFactory::create()
{
	return std::make_unique<TransformAction>(tcreator);
}

