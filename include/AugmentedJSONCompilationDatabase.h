#ifndef AUGMENTEDJSONCOMPILATIONDATABASE_H
#define AUGMENTEDJSONCOMPILATIONDATABASE_H

#include "clang/Tooling/JSONCompilationDatabase.h"

namespace clang {
namespace tooling {

    class AugmentedJSONCompilationDatabase : public CompilationDatabase {

    private:
        JSONCompilationDatabase* json_db;

        std::vector<std::string> additional_cmds;

    public:
        AugmentedJSONCompilationDatabase(
                JSONCompilationDatabase* json_db,
                FixedCompilationDatabase* fixed_db);

        ~AugmentedJSONCompilationDatabase();

        void setAdditionalCmds(std::vector<std::string> cmds) {
            additional_cmds = cmds;
        }
        void setJsonDB(JSONCompilationDatabase* db) {
            if (json_db)
                delete json_db;
            json_db = db;
        }

        std::vector<CompileCommand>
            getCompileCommands(StringRef FilePath) const override;

        std::vector<CompileCommand>
            getAllCompileCommands() const override;

        std::vector<std::string>
            getAllFiles() const override;
    };
}
}

#endif /* !AUGMENTEDJSONCOMPILATIONDATABASE_H */

