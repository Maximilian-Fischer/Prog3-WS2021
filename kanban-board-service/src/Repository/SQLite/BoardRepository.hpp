#pragma once

#include "Repository/RepositoryIf.hpp"
#include "sqlite3.h"

namespace Prog3 {
namespace Repository {
namespace SQLite {

class BoardRepository : public RepositoryIf {
  private:
    sqlite3 *database;

    void initialize();
    void createDummyData();
    void handleSQLError(int statementResult, char *errorMessage);

    static bool isValid(int id) {
        return id != INVALID_ID;
    }

    static Prog3::Core::Model::Item getItemFromCallback(char **fieldValues, int startIndex);
    static Prog3::Core::Model::Column getColumnFromCallback(char **fieldValues, int startIndex);

    static int queryColumnsCallback(void *data, int numberOfColumns, char **fieldValues, char **columnNames);
    static int queryItemsCallback(void *data, int numberOfColumns, char **fieldValues, char **columnNames);

  public:
    BoardRepository();
    virtual ~BoardRepository();

    virtual Prog3::Core::Model::Board getBoard();
    virtual std::vector<Prog3::Core::Model::Column> getColumns();
    virtual std::optional<Prog3::Core::Model::Column> getColumn(int id);
    virtual std::optional<Prog3::Core::Model::Column> postColumn(std::string name, int position);
    virtual std::optional<Prog3::Core::Model::Column> putColumn(int id, std::string name, int position);
    virtual void deleteColumn(int id);
    virtual std::vector<Prog3::Core::Model::Item> getItems(int columnId);
    virtual std::optional<Prog3::Core::Model::Item> getItem(int columnId, int itemId);
    virtual std::optional<Prog3::Core::Model::Item> postItem(int columnId, std::string title, int position);
    virtual std::optional<Prog3::Core::Model::Item> putItem(int columnId, int itemId, std::string title, int position);
    virtual void deleteItem(int columnId, int itemId);

    static inline std::string const boardTitle = "Kanban Board";
    static inline int const INVALID_ID = -1;

    static std::string const databaseFile;
};

} // namespace SQLite
} // namespace Repository
} // namespace Prog3
