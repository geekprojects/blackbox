//
// Created by Ian Parker on 16/12/2025.
//

#ifndef BLACKBOX_DATABASE_H
#define BLACKBOX_DATABASE_H

#include <sqlite3.h>
#include <string>

#include "logger.h"

class Database : public BlackBox::Logger
{
    sqlite3* m_db = nullptr;

 protected:
    explicit Database(std::string name);

    static std::string getString(sqlite3_stmt* stmt, int col);

 public:
    ~Database() override;

    bool open(std::string path);
    virtual void close();

    bool isOpen() const { return m_db != nullptr; }

    void startTransaction();
    void commitTransaction();

    sqlite3* getDB() const { return m_db; }
};

#endif //BLACKBOX_DATABASE_H
