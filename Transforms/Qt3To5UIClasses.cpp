#include "Qt3To5UIClasses.h"

#include "Replacer.h"

using namespace clang::ast_matchers;

REGISTER_TRANSFORM(Qt3To5UIClasses);

//==============================================================================
class BaseConstructorHandler : public MatchFinder::MatchCallback
{
public:
	BaseConstructorHandler(Transform* transform)
	  : _transform(transform),
		_invalid_record_name_regex(llvm::Regex("^.*::.*$", llvm::Regex::IgnoreCase)),
		_ui_class_regex(llvm::Regex("(public|protected|private)? ?([^_ ,]+)_([^, ]+)"))
	{}

	virtual void run(const MatchFinder::MatchResult& result) {
		if (const clang::CXXRecordDecl* rec = result.Nodes.getNodeAs<clang::CXXRecordDecl>("class_definition")) {
			clang::SourceLocation loc = rec->getLocation();
			if (!_transform->canChangeLocation(loc)) {
				return;
			}

			std::string name = rec->getQualifiedNameAsString();
			// Filter out some mismatched records that don't have CXXBaseSpecifier records.
			if (_invalid_record_name_regex.match(name)) {
				return;
			}

			clang::SourceManager* src_manager = result.SourceManager;
			for (const clang::CXXBaseSpecifier base : rec->bases()) {
				clang::SourceRange range = base.getSourceRange();
				llvm::StringRef src_text = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(range),
																	   *src_manager, clang::LangOptions(), 0);

				llvm::SmallVector<llvm::StringRef, 10> matched;
				if (!_ui_class_regex.match(src_text, &matched)) {
					continue;
				}

				std::string accessmodifier = matched[1].str();
				std::string classname = matched[2].str();
				std::string parentclassname = matched[3].str();

				if (parentclassname == "scr") {
					parentclassname = "ZScreen";
				} else if (parentclassname == "ui") {
					parentclassname = "QWidget";
				}

				std::string replacement_text = accessmodifier + " " + parentclassname + ", public Ui::" + classname;
				Replacer::instance().replace(range, replacement_text, *src_manager);
			}
		}
	}

private:
	Transform* _transform;
	llvm::Regex _invalid_record_name_regex;
	llvm::Regex _ui_class_regex;
};

//==============================================================================
void Qt3To5UIClasses::HandleTranslationUnit(clang::ASTContext& c)
{
	init();

	BaseConstructorHandler HandlerForBaseConstructor(this);

	MatchFinder finder;
	finder.addMatcher(cxxRecordDecl().bind("class_definition"), &HandlerForBaseConstructor);

	finder.matchAST(c);
}

//==============================================================================
refactorial::config::TransformConfig* Qt3To5UIClasses::getTransformConfig()
{
	return &(TransformRegistry::get().config.transforms.qt3_to_5_ui_classes);
}
