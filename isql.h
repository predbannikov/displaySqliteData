#ifndef ISQL_H
#define ISQL_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>
#include "global.h"

class ISql : public QObject
{
    Q_OBJECT
public:
    explicit ISql(QObject *parent = nullptr);
    ~ISql();

private:
    QSqlDatabase db;

public slots:
    void doWork();
    void loadData(QString _name);
    QStringList loadTables();
    void createTable(QString _name);
    void saveData(const QList<QPointF> _point, const QString _name);
    void deleteTable(QString _name);
signals:
    void signalReady();
    void signalSendDataTable(QString, QList<QPointF >);
};

#endif // ISQL_H
