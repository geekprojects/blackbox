//
// Created by Ian Parker on 16/12/2025.
//

#include "blackbox/database.h"

using namespace std;
using namespace BlackBox;

Database::Database(std::string name) : Logger("Database[" + name + "]")
{
}

Database::~Database()
{
    Database::close();
}

bool Database::open(std::string path)
{
    int res = sqlite3_open_v2(path.c_str(), &m_db, SQLITE_OPEN_READONLY, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "Failed to open database: %d: %s", res, sqlite3_errmsg(m_db));
        m_db = nullptr;
        return false;
    }
    return true;
}

void Database::close()
{
    if (m_db != nullptr)
    {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

void Database::startTransaction()
{
    int res = sqlite3_exec(m_db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "startTransaction: Failed to start transaction: %d: %s", res, sqlite3_errmsg(m_db));
    }
}

void Database::commitTransaction()
{
    int res = sqlite3_exec(m_db, "COMMIT", nullptr, nullptr, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "commitTransaction: Failed to commit transaction: %d: %s", res, sqlite3_errmsg(m_db));
    }
}

string Database::getString(sqlite3_stmt* stmt, int col)
{
    const unsigned char* str = sqlite3_column_text(stmt, col);
    if (str == nullptr)
    {
        return "";
    }
    return {reinterpret_cast<const char*>(str)};
}
