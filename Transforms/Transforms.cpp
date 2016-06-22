#include "Transforms.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>

#include <stdexcept>

using namespace clang;
using namespace clang::tooling;
using namespace std;

TransformRegistry &TransformRegistry::get()
{
	static TransformRegistry instance;
	return instance;
}

void TransformRegistry::add(const string &name, transform_creator creator)
{
	llvm::outs() << "registered " << name << "\n";
	m_transforms.insert(pair<string, transform_creator>(name, creator));
}

transform_creator TransformRegistry::operator[](const string &name) const
{
	auto iter = m_transforms.find(name);
	if(iter==m_transforms.end())
		llvm::errs() << name << " out of range" << "\n";
	return iter->second;
}

class TransformAction : public ASTFrontendAction {
private:
	transform_creator tcreator;
public:
	TransformAction(transform_creator creator) {tcreator = creator;}
protected:
	std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) {
		std::unique_ptr<Transform> xform = tcreator();
		xform->ci = &CI;
		return std::move(xform);
	}

	virtual bool BeginInvocation(CompilerInstance &CI) {
		// CI.getHeaderSearchOpts().AddPath("/usr/local/lib/clang/3.2/include", frontend::System, false, false, false);
		return true;
	}
};

TransformFactory::TransformFactory(transform_creator creator) {
	tcreator = creator;
}
FrontendAction *TransformFactory::create() {
	return new TransformAction(tcreator);
}
