
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>

#include <QGeoView/QGVMap.h>

#include "mainwindow.h"

#include <leveldb/db.h>

using namespace std;

int main(int argc, char** argv)
{
/*
    leveldb::DB* db;
    leveldb::Options options;
    options.
    leveldb::Status status = leveldb::DB::Open(options, "/Users/ian/Library/Application Support/Orbx/Volanta/IndexedDB/file__0.indexeddb.leveldb", &db);
    printf("Status: %s\n", status.ToString().c_str());


    exit(1);
*/
    QApplication app(argc, argv);
    QApplication::setApplicationName("BlakBox Flight Tracker");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    MainWindow window;
    window.show();
    return app.exec();
}
