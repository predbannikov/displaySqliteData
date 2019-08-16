#include "isql.h"

ISql::ISql(QObject *parent): QObject (parent)
{


}

void ISql::doWork()
{
//    qDebug() << "ISql doWork id:" << this->thread()->currentThreadId();
    QString dbName = "db_name.sqlite";

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName);
    if (!db.open()) {
        qDebug() << "Not open sqlite" << dbName << db.lastError().text();
        return;
    }
    emit signalReady();
}

ISql::~ISql()
{
    db.close();
//    qDebug() << "ISql deleteLater id:" << this->thread()->currentThreadId();
}

void ISql::loadData(QString _name)
{
    QList<QPointF> _data;
    QSqlQuery query;
    QString requestX = QString("SELECT %1, %2 FROM '%3'; ").arg("X").arg("Y").arg(_name);
    if(!query.exec(requestX)) {
        qDebug() << "not select table requestX" << _name << query.lastError().text();
        return;
    }
    QSqlRecord rec = query.record();
    while(query.next()) {
        _data << QPointF(query.value(rec.indexOf("X")).toDouble(), query.value(rec.indexOf("Y")).toDouble());
    }

    emit signalSendDataTable(_name, _data);
}

QStringList ISql::loadTables()
{
//    qDebug() << "ISql loadTables id:" << this->thread()->currentThreadId();
    return db.tables();
}

void ISql::createTable(QString _name)
{
//    qDebug() << "ISql createTables id:" << this->thread()->currentThreadId();

    QSqlQuery query;
    QString request = QString("CREATE TABLE '%1' ("
                              "'id_i' INTEGER PRIMARY KEY, 'X' REAL, 'Y' REAL);").arg(_name);
    if(!query.exec(request)) {
        qDebug() << "Unable to create table" << _name << query.lastError().text();
        return;
    }
}

void ISql::saveData(const QList<QPointF> _point, const QString _name)
{
    QSqlQuery query;
    QString request;
    for(int i = 0; i < _point.size(); i++) {
        request = QString("INSERT INTO '%1' ('X', 'Y', 'id_i') "
                          "VALUES(%2, %3, %4);")
                .arg(_name)
                .arg(_point[i].x())
                .arg(_point[i].y())
                .arg(i);
        if(!query.exec(request)) {
            qDebug() << "Insert not succes" << request << i << _name << query.lastError().text();;
            break;
        }
    }
}

void ISql::deleteTable(QString _name)
{
    QSqlQuery query;
    QString request = QString("DROP TABLE '%1';").arg(_name);
    if(!query.exec(request)) {
        qDebug() << "Cannot delete table" << request << _name << query.lastError().text();
        return;
    }
}
