/***************************************************************************
**
** Copyright (C) 2012 Marko Koschak (marko.koschak@tisno.de)
** All rights reserved.
**
** This file is part of ownKeepass.
**
** ownKeepass is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** ownKeepass is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with ownKeepass.  If not, see <http://www.gnu.org/licenses/>.
**
***************************************************************************/

#ifndef KDBDATABASE_H
#define KDBDATABASE_H

#include <QObject>
#include <QFile>
#include "private/AbstractDatabaseFactory.h"

namespace kpxPublic {

class KdbDatabase : public QObject
{
    Q_OBJECT
    Q_ENUMS(eDatabaseTypeWrapper)
    Q_ENUMS(eDatabaseAccessResultWrapper)

public:
    enum eDatabaseTypeWrapper {
        DB_TYPE_UNKNOWN = AbstractDatabaseInterface::DB_TYPE_UNKNOWN,
        DB_TYPE_KEEPASS_1 = AbstractDatabaseInterface::DB_TYPE_KEEPASS_1,
        DB_TYPE_KEEPASS_2 = AbstractDatabaseInterface::DB_TYPE_KEEPASS_2
    };

    enum eDatabaseAccessResultWrapper {
        RE_OK = AbstractDatabaseInterface::RE_OK,                                                           // no error
        RE_DB_LOAD_ERROR = AbstractDatabaseInterface::RE_DB_LOAD_ERROR,                                     // error loading data from database
        RE_DB_SAVE_ERROR = AbstractDatabaseInterface::RE_DB_SAVE_ERROR,                                     // error saving data into database
        RE_DB_NOT_OPENED = AbstractDatabaseInterface::RE_DB_NOT_OPENED,                                     // database is not opened
        RE_DB_OPEN = AbstractDatabaseInterface::RE_DB_OPEN,                                                 // other database is currently open, close it first
        RE_DB_ALREADY_CLOSED = AbstractDatabaseInterface::RE_DB_ALREADY_CLOSED,                             // database already closed, no harm
        RE_DB_CLOSE_FAILED = AbstractDatabaseInterface::RE_DB_CLOSE_FAILED,                                 // database closing failed
        RE_DB_FILE_ERROR = AbstractDatabaseInterface::RE_DB_FILE_ERROR,                                     // file path error for new database
        RE_DB_SETKEY_ERROR = AbstractDatabaseInterface::RE_DB_SETKEY_ERROR,                                 // error setting key (consisting of password and/or keyfile
        RE_DB_SETPW_ERROR = AbstractDatabaseInterface::RE_DB_SETPW_ERROR,                                   // error setting password for database
        RE_DB_SETKEYFILE_ERROR = AbstractDatabaseInterface::RE_DB_SETKEYFILE_ERROR,                         // error setting key file for database
        RE_DB_CREATE_BACKUPGROUP_ERROR = AbstractDatabaseInterface::RE_DB_CREATE_BACKUPGROUP_ERROR,         // error creating backup group
        RE_PRECHECK_DB_PATH_ERROR = AbstractDatabaseInterface::RE_PRECHECK_DB_PATH_ERROR,                   // database file does not exists on precheck
        RE_PRECHECK_KEY_FILE_PATH_ERROR = AbstractDatabaseInterface::RE_PRECHECK_KEY_FILE_PATH_ERROR,       // key file does not exists on precheck
        RE_PRECHECK_DB_PATH_CREATION_ERROR = AbstractDatabaseInterface::RE_PRECHECK_DB_PATH_CREATION_ERROR, // path to database file could not be created

        RE_LAST = AbstractDatabaseInterface::RE_LAST
    };


    Q_PROPERTY(int keyTransfRounds READ keyTransfRounds WRITE setKeyTransfRounds NOTIFY keyTransfRoundsChanged)
    Q_PROPERTY(int cryptAlgorithm READ cryptAlgorithm WRITE setCryptAlgorithm NOTIFY cryptAlgorithmChanged)
    Q_PROPERTY(bool showUserNamePasswordsInListView READ showUserNamePasswordsInListView WRITE setShowUserNamePasswordsInListView STORED true SCRIPTABLE true)
    Q_PROPERTY(bool sortAlphabeticallyInListView READ sortAlphabeticallyInListView WRITE setSortAlphabeticallyInListView STORED true SCRIPTABLE true)

public: // QtQuick 1.1 needs here a public keyword otherwise if does not find the next function ???
    Q_INVOKABLE void open(const int databaseType, const QString& dbFilePath, const QString &keyFilePath, const QString& password, bool readonly);
    Q_INVOKABLE void create(const int databaseType, const QString& dbFilePath, const QString &keyFilePath, const QString& password);
    Q_INVOKABLE void close();
    Q_INVOKABLE void changePassword(const QString& password, const QString &keyFile);

public:
    KdbDatabase(QObject* parent=0);
    virtual ~KdbDatabase() {}

    int keyTransfRounds() const { return m_keyTransfRounds; }
    void setKeyTransfRounds(const int value) { emit changeDatabaseKeyTransfRounds(value); }
    int cryptAlgorithm() const { return m_cryptAlgorithm; }
    void setCryptAlgorithm(const int value) { emit changeDatabaseCryptAlgorithm(value); }
    bool showUserNamePasswordsInListView() const { return m_showUserNamePasswordsInListView; }
    void setShowUserNamePasswordsInListView(bool value) { m_showUserNamePasswordsInListView = value; emit setting_showUserNamePasswordsInListView(value); }
    bool sortAlphabeticallyInListView() const { return m_sortAlphabeticallyInListView; }
    void setSortAlphabeticallyInListView(const bool value) { m_sortAlphabeticallyInListView = value; emit setting_sortAlphabeticallyInListView(value); }

signals:
    // signals to DatabaseClient backend thread
    void openDatabase(QString filePath, QString password, QString keyfile, bool readonly);
    void createNewDatabase(QString filePath, QString password, QString keyfile, int cryptAlgorithm, int keyTransfRounds);
    void closeDatabase();
    void changeDatabasePassword(QString password, QString keyFile);
    void changeDatabaseKeyTransfRounds(int value);
    void changeDatabaseCryptAlgorithm(int value);
    void setting_showUserNamePasswordsInListView(bool value);
    void setting_sortAlphabeticallyInListView(bool value);

    // signals to QML
    void databaseOpened();
    void newDatabaseCreated();
    void databaseClosed();
    void databasePasswordChanged();
    void keyTransfRoundsChanged();
    void cryptAlgorithmChanged();
    void errorOccured(int result, QString errorMsg);

private slots:
    // signals from DatabaseClient backend thread
    void slot_databaseKeyTransfRoundsChanged(int value) {
        if (value != m_keyTransfRounds) {
            m_keyTransfRounds = value;
            emit keyTransfRoundsChanged();
        }
    }
    void slot_databaseCryptAlgorithmChanged(int value) {
        if (value != m_cryptAlgorithm) {
            m_cryptAlgorithm = value;
            emit cryptAlgorithmChanged();
        }
    }
    void slot_databaseClosed();

private:
    void connectToDatabaseClient();
    void disconnectFromDatabaseClient();

private:
    // The following properties are read from backend and therefore there are slots for it
    int m_keyTransfRounds;
    int m_cryptAlgorithm;
    // Settings are simply passed over to the backend thread
    bool m_showUserNamePasswordsInListView;
    bool m_sortAlphabeticallyInListView;

    bool m_readOnly;

    bool m_connected;
    int m_database_type;
    Q_DISABLE_COPY(KdbDatabase)
};

}
#endif // KDBDATABASE_H
