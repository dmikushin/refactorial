#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <string>
#include <vector>
#include <stdint.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/TokenKinds.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Lexer.h>
#include <clang/Tooling/Refactoring.h>

#include <yamlreader.h>

class Transform : public clang::ASTConsumer
{
public:
	virtual refactorial::config::TransformConfig* getTransformConfig() = 0;

	clang::SourceLocation findLocAfterToken(clang::SourceLocation curLoc, clang::tok::TokenKind tok);
	clang::SourceLocation getLocForEndOfToken(clang::SourceLocation curLoc);
	clang::SourceLocation findLocAfterSemi(clang::SourceLocation curLoc);

	void addAllowedPath(const std::string& path);
	bool canChangeLocation(const clang::SourceLocation& loc);

	clang::CompilerInstance *ci;

protected:
	virtual void init();

private:
	std::vector<llvm::Regex> allowedDirectoryList;
};

template <typename T> std::unique_ptr<Transform> transform_factory()
{
	return std::unique_ptr<T>(new T);
}

typedef std::unique_ptr<Transform> (*transform_creator)(void);

class TransformRegistry
{
 private:
	std::map<std::string,transform_creator> m_transforms;
 public:
	refactorial::config::Config config;
	std::map<std::string, std::string> touchedFiles;
	std::vector<clang::tooling::Replacement> *replacements;

	static TransformRegistry& get();
	void add(const std::string &, transform_creator);
	transform_creator operator[](const std::string &name) const;
};

class TransformRegistration
{
public:
	TransformRegistration(const std::string& name, transform_creator creator) {
		TransformRegistry::get().add(name, creator);
	}
};

#define REGISTER_TRANSFORM(transform)	  TransformRegistration _transform_registration_ ## transform(#transform, &transform_factory<transform>)

class TransformFactory : public clang::tooling::FrontendActionFactory {
private:
	transform_creator tcreator;
public:
	TransformFactory(transform_creator creator);
    std::unique_ptr<clang::FrontendAction> create();
};

#endif
