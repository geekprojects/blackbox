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

bool Database::open(string path, bool readOnly)
{
    int flags = 0;
    if (readOnly)
    {
        flags |= SQLITE_OPEN_READONLY;
    }
    else
    {
        flags |= SQLITE_OPEN_READWRITE;
    }
    int res = sqlite3_open_v2(path.c_str(), &m_db, flags, nullptr);

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
    for (auto stmt : m_statements)
    {
        sqlite3_finalize(stmt);
    }
    m_statements.clear();

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

bool Database::prepare(const std::string &sql, sqlite3_stmt** stmt)
{
    int res = sqlite3_prepare_v2(getDB(), sql.c_str(), -1, stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "prepare: Failed to prepare statement: %d: %s: %s", res, sqlite3_errmsg(getDB()), sql.c_str());
        return false;
    }

    m_statements.push_back(*stmt);

    return true;
}
