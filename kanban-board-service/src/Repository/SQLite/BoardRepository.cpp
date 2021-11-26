#include "BoardRepository.hpp"
#include <filesystem>
#include <string.h>

using namespace Prog3::Repository::SQLite;
using namespace Prog3::Core::Model;
using namespace std;

#ifdef RELEASE_SERVICE
string const BoardRepository::databaseFile = "./data/kanban-board.db";
#else
string const BoardRepository::databaseFile = "../data/kanban-board.db";
#endif

BoardRepository::BoardRepository() : database(nullptr) {

    string databaseDirectory = filesystem::path(databaseFile).parent_path().string();

    if (filesystem::is_directory(databaseDirectory) == false) {
        filesystem::create_directory(databaseDirectory);
    }

    int result = sqlite3_open(databaseFile.c_str(), &database);

    if (SQLITE_OK != result) {
        cout << "Cannot open database: " << sqlite3_errmsg(database) << endl;
    }

    initialize();
}

BoardRepository::~BoardRepository() {
    sqlite3_close(database);
}

void BoardRepository::initialize() {
    int result = 0;
    char *errorMessage = nullptr;

    string sqlCreateTableColumn =
        "create table if not exists column("
        "id integer not null primary key autoincrement,"
        "name text not null,"
        "position integer not null UNIQUE);";

    string sqlCreateTableItem =
        "create table if not exists item("
        "id integer not null primary key autoincrement,"
        "title text not null,"
        "date text not null,"
        "position integer not null,"
        "column_id integer not null,"
        "unique (position, column_id),"
        "foreign key (column_id) references column (id));";

    result = sqlite3_exec(database, sqlCreateTableColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
    result = sqlite3_exec(database, sqlCreateTableItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    // only if dummy data is needed ;)
    //createDummyData();
}

Board BoardRepository::getBoard() {

    Board board = Board(boardTitle);

    vector<Column> columns = getColumns();
    board.setColumns(columns);

    return board;
}

std::vector<Column> BoardRepository::getColumns() {

    std::vector<Column> columns;

    string sqlQueryColumns =
        "SELECT column.id, column.name, column.position, item.id, item.title, item.position, item.date from column "
        "left join item on item.column_id = column.id "
        "order by column.position, item.position";

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlQueryColumns.c_str(), queryColumnsCallback, &columns, &errorMessage);
    handleSQLError(result, errorMessage);

    return columns;
}

std::optional<Column> BoardRepository::getColumn(int id) {

    std::vector<Column> columns;

    string sqlQueryColumns =
        "SELECT column.id, column.name, column.position, item.id, item.title, item.position, item.date from column "
        "left join item on item.column_id = column.id where column.id = " +
        to_string(id) +
        " order by column.position, item.position";

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlQueryColumns.c_str(), queryColumnsCallback, &columns, &errorMessage);
    handleSQLError(result, errorMessage);

    if (columns.size() == 1) {
        return columns.front();
    } else {
        return std::nullopt;
    }
}

std::optional<Column> BoardRepository::postColumn(std::string name, int position) {

    string sqlPostItem =
        "INSERT INTO column('name', 'position') "
        "VALUES('" +
        name + "', '" + to_string(position) + "')";

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlPostItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    if (SQLITE_OK == result) {
        int columnId = sqlite3_last_insert_rowid(database);
        return Column(columnId, name, position);
    }

    return std::nullopt;
}

std::optional<Prog3::Core::Model::Column> BoardRepository::putColumn(int id, std::string name, int position) {

    string sqlUpdateColumn =
        "UPDATE column "
        "SET name = '" +
        name + "', position = '" + to_string(position) +
        "' WHERE id = " + to_string(id);

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlUpdateColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    return getColumn(id);
}

void BoardRepository::deleteColumn(int id) {

    string sqlDeleteColumnItems =
        "DELETE FROM item "
        "WHERE item.column_id = " +
        to_string(id);

    string sqlDeleteColumn =
        "DELETE FROM column "
        "WHERE column.id = " +
        to_string(id);

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlDeleteColumnItems.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    result = sqlite3_exec(database, sqlDeleteColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

std::vector<Item> BoardRepository::getItems(int columnId) {

    std::vector<Item> items;

    string sqlQueryItems =
        "SELECT item.id, item.title, item.position, item.date from item "
        "where item.column_id = " +
        std::to_string(columnId) +
        " order by item.position";

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlQueryItems.c_str(), queryItemsCallback, &items, &errorMessage);
    handleSQLError(result, errorMessage);

    return items;
}

std::optional<Item> BoardRepository::getItem(int columnId, int itemId) {

    std::vector<Item> items;

    string sqlQueryItems =
        "SELECT item.id, item.title, item.position, item.date from item "
        "where item.column_id = " +
        std::to_string(columnId) + " and item.id = " + std::to_string(itemId) +
        " order by item.position";

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlQueryItems.c_str(), queryItemsCallback, &items, &errorMessage);
    handleSQLError(result, errorMessage);

    if (items.size() == 1) {
        return items.front();
    } else {
        return std::nullopt;
    }
}

std::optional<Item> BoardRepository::postItem(int columnId, std::string title, int position) {

    string sqlPostItem =
        "INSERT INTO item ('title', 'date', 'position', 'column_id')"
        "VALUES ('" +
        title + "', datetime(), " + to_string(position) + ", " + to_string(columnId) + ");";

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlPostItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    int itemId = INVALID_ID;
    if (SQLITE_OK == result) {
        itemId = sqlite3_last_insert_rowid(database);
    }

    return getItem(columnId, itemId);
}

std::optional<Prog3::Core::Model::Item> BoardRepository::putItem(int columnId, int itemId, std::string title, int position) {

    string sqlUpdateItem =
        "UPDATE item SET title = '" + title + "', position = " + to_string(position) +
        " WHERE item.column_id = " + to_string(columnId) + " AND item.id = " + to_string(itemId);

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlUpdateItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    return getItem(columnId, itemId);
}

void BoardRepository::deleteItem(int columnId, int itemId) {
    string sqlQueryItem =
        "DELETE FROM item "
        "WHERE item.column_id = " +
        to_string(columnId) +
        " AND item.id = " + to_string(itemId);

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlQueryItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

Item BoardRepository::getItemFromCallback(char **fieldValues, int startIndex) {
    int index = startIndex;

    int itemId = fieldValues[index] ? atoi(fieldValues[index]) : INVALID_ID;
    index++;

    string title = fieldValues[index] ? fieldValues[index] : "";
    index++;

    int position = fieldValues[index] ? atoi(fieldValues[index]) : 0;
    index++;

    string timestamp = fieldValues[index] ? fieldValues[index] : "";

    Item item(itemId, title, position, timestamp);
    return item;
}

Column BoardRepository::getColumnFromCallback(char **fieldValues, int startIndex) {
    int index = startIndex;

    int columnId = fieldValues[index] ? atoi(fieldValues[index]) : INVALID_ID;
    index++;

    string name = fieldValues[index] ? fieldValues[index] : "";
    index++;

    int position = fieldValues[index] ? atoi(fieldValues[index]) : 0;
    index++;

    Column column(columnId, name, position);
    return column;
}

int BoardRepository::queryColumnsCallback(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {

    std::vector<Column> *pColumns = static_cast<std::vector<Column> *>(data);
    std::vector<Column> &columns = *pColumns;

    Item item = getItemFromCallback(fieldValues, 3);

    int columnId = fieldValues[0] ? atoi(fieldValues[0]) : INVALID_ID;
    bool wasColumnAlreadyAdded = false;

    for (Column &column : columns) {
        if (column.getId() == columnId) {
            wasColumnAlreadyAdded = true;
            if (isValid(item.getId())) {
                column.addItem(item);
            }
            break;
        }
    }

    if (false == wasColumnAlreadyAdded) {
        Column column = getColumnFromCallback(fieldValues, 0);

        if (isValid(item.getId())) {
            column.addItem(item);
        }

        if (isValid(column.getId())) {
            columns.push_back(column);
        }
    }

    return 0;
}

int BoardRepository::queryItemsCallback(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {

    std::vector<Item> *result = static_cast<std::vector<Item> *>(data);

    Item item = getItemFromCallback(fieldValues, 0);
    if (isValid(item.getId())) {
        result->push_back(item);
    }
    return 0;
}

void BoardRepository::handleSQLError(int statementResult, char *errorMessage) {

    if (statementResult != SQLITE_OK) {
        cout << "SQL error: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
}

void BoardRepository::createDummyData() {

    cout << "creatingDummyData ..." << endl;

    int result = 0;
    char *errorMessage;
    string sqlInsertDummyColumns =
        "insert into column (name, position)"
        "VALUES"
        "(\"prepare\", 1),"
        "(\"running\", 2),"
        "(\"finished\", 3);";

    result = sqlite3_exec(database, sqlInsertDummyColumns.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    string sqlInserDummyItems =
        "insert into item (title, date, position, column_id)"
        "VALUES"
        "(\"in plan\", date('now'), 1, 1),"
        "(\"some running task\", date('now'), 1, 2),"
        "(\"finished task 1\", date('now'), 1, 3),"
        "(\"finished task 2\", date('now'), 2, 3);";

    result = sqlite3_exec(database, sqlInserDummyItems.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}
