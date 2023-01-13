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
			if (!rec->isThisDeclarationADefinition()) {
				return;
			}

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
				llvm::StringRef src_text = 
					clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(range),
					*src_manager, clang::LangOptions(), 0);

				llvm::SmallVector<llvm::StringRef, 10> matched;
				if (!_ui_class_regex.match(src_text, &matched)) {
					continue;
				}

				std::string accessmodifier = matched[1].str();
				std::string classname = matched[2].str();
				std::string parentclassname = matched[3].str();

				std::string replacement_text = accessmodifier + " " +
					classname + "_" + parentclassname + ", public Ui::" + classname;
				Replacer::instance().replace(range, src_text.str(), replacement_text, *src_manager);
			}
		}
	}

private:
	Transform* _transform;
	llvm::Regex _invalid_record_name_regex;
	llvm::Regex _ui_class_regex;
};

//==============================================================================
class ConstructorInitializerHandler : public MatchFinder::MatchCallback
{
public:
	ConstructorInitializerHandler(Transform* transform)
		: _transform(transform),
		  _ui_initializer_regex(llvm::Regex("([^_ ,]+)_([^, ]+)"))
	{}

	virtual void run(const MatchFinder::MatchResult& result) {
		if (const clang::CXXConstructorDecl* ctor = result.Nodes.getNodeAs<clang::CXXConstructorDecl>("ctor")) {
			// Nothing to do if this isn't a user defined constructor.
			if (!ctor->isUserProvided()) {
				return;
			}

			clang::SourceLocation loc = ctor->getLocation();
			if (!_transform->canChangeLocation(loc)) {
				return;
			}

			clang::SourceManager* src_manager = result.SourceManager;

			bool has_ui_initializer = false;
			for (const clang::CXXCtorInitializer* init : ctor->inits()) {
				if (!init->isBaseInitializer()) {
					continue;
				}

				clang::SourceRange range(init->getSourceLocation(), init->getLParenLoc().getLocWithOffset(-1));
				llvm::StringRef src_text = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(range),
																	   *src_manager, clang::LangOptions(), 0);
				if (_ui_initializer_regex.match(src_text)) {
					has_ui_initializer = true;
					break;
				}
			}

			if (!has_ui_initializer) return;

			clang::CompoundStmt* stmt = llvm::cast<clang::CompoundStmt>(ctor->getBody());
			// TODO: Almost certainly need to handle indentation better here but should be good enough for now.
			Replacer::instance().insert(stmt->getLBracLoc().getLocWithOffset(1), "\n\tsetupUi(this);\n", *src_manager);
		}
	}

private:
	Transform* _transform;
	llvm::Regex _ui_initializer_regex;
};

//==============================================================================
void Qt3To5UIClasses::HandleTranslationUnit(clang::ASTContext& c)
{
	init();

	MatchFinder finder;

	BaseConstructorHandler HandlerForBaseConstructor(this);
	finder.addMatcher(cxxRecordDecl().bind("class_definition"), &HandlerForBaseConstructor);

	ConstructorInitializerHandler HandlerForConstructorInitializer(this);
	finder.addMatcher(cxxConstructorDecl(hasAnyConstructorInitializer(allOf(isWritten(), isBaseInitializer()))).bind("ctor"),
					  &HandlerForConstructorInitializer);

	finder.matchAST(c);
}

//==============================================================================
refactorial::config::TransformConfig* Qt3To5UIClasses::getTransformConfig()
{
	return &(TransformRegistry::get().config.transforms.qt3_to_5_ui_classes);
}
