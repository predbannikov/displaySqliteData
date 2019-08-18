#ifndef ISQL_H
#define ISQL_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QJsonObject>
#include <QtSql>
#include <QMutex>
#include "global.h"

class ISql : public QObject
{
    Q_OBJECT
public:
    explicit ISql(QString path = "/home/user/.local/share", QObject *parent = nullptr);
    ~ISql();

private:
    QSqlDatabase db;
    QQueue<QJsonObject> queue;
    QMutex mutex;
    QMap<QString, const CustomSet *> data;
    QString pathData;

    void parsQueue();
public slots:
    void doWork();
    void loadData(QString _tableName);
    QStringList loadTables();
    void createTable(QString _tableName);
    void deleteTable(QString _tableName);
    void saveData(const CustomSet _set);
    void saveWidth(QString _tableName);
signals:
    void signalReady();
    void signalSendDataTable(CustomSet);
};

#endif // ISQL_H
