// 
// AugmentedJSONCompilationDatabase.cpp
// 
// 2014 mzenzes
// 
#include "AugmentedJSONCompilationDatabase.h"

#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/Support/raw_ostream.h>


namespace clang {
namespace tooling {

AugmentedJSONCompilationDatabase::AugmentedJSONCompilationDatabase(
                JSONCompilationDatabase* json_db,
                FixedCompilationDatabase* fixed_db) :
            json_db(json_db)
{
    if (fixed_db) {
        std::vector<CompileCommand> cmds = fixed_db->getCompileCommands("");
        if (cmds.size() != 1)
            llvm::errs() << "angry bark!";
        additional_cmds = cmds.front().CommandLine;
    }
}

AugmentedJSONCompilationDatabase::~AugmentedJSONCompilationDatabase()
{
    if (json_db)
        delete json_db;
}


std::vector<CompileCommand>
AugmentedJSONCompilationDatabase::getCompileCommands(StringRef FilePath) const
{
    std::vector<CompileCommand> retval;
    retval = json_db->getCompileCommands(FilePath);

    std::vector<CompileCommand>::iterator it;
    for(it=retval.begin();it!=retval.end();++it) {
        it->CommandLine.insert(
                it->CommandLine.end(),
                additional_cmds.begin(),
                additional_cmds.end());
    }

    return retval;
};

std::vector<std::string>
AugmentedJSONCompilationDatabase::getAllFiles() const
{
    if (json_db)
        return json_db->getAllFiles();
    else
        return std::vector<std::string>();
}

}
}
