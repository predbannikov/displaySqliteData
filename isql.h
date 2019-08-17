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
    explicit ISql(QObject *parent = nullptr);
    ~ISql();

private:
    QSqlDatabase db;
    QQueue<QJsonObject> queue;
    QMutex mutex;
    QMap<QString, const QList<QPointF > *> points;

    void parsQueue();
public slots:
    void doWork();
    void loadData(QString _tableName);
    QStringList loadTables();
    void createTable(QString _tableName);
    void saveData(const QList<QPointF> _point, const QString _tableName);
    void deleteTable(QString _tableName);

signals:
    void signalReady();
    void signalSendDataTable(QString, QList<QPointF >);
};

#endif // ISQL_H
