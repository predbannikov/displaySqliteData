#include "isql.h"

ISql::ISql(QObject *parent): QObject (parent)
{


}

void ISql::doWork()
{
//    qDebug() << "ISql doWork id:" << this->thread()->currentThreadId();
//    QFileInfo file(dbName);
//    if(!file.exists()) {

//    }


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
    qDebug() << "ISql deleteLater id:" << this->thread()->currentThreadId();
}

QStringList ISql::loadTables()
{
//    qDebug() << "ISql loadTables id:" << this->thread()->currentThreadId();
    return db.tables();
}

void ISql::createTable(QString _tableName)
{
//    qDebug() << "ISql createTables id:" << this->thread()->currentThreadId();

    QSqlQuery query;
    QString request = QString("CREATE TABLE '%1' ("
                              "'id_i' INTEGER PRIMARY KEY, 'X' REAL, 'Y' REAL, 'WIDTH' INTEGER);").arg(_tableName);
    if(!query.exec(request)) {
        qDebug() << "Unable to create table" << _tableName << query.lastError().text();
        return;
    }
}

void ISql::deleteTable(QString _tableName)
{
    qDebug() << "deleteTable" << _tableName;
    QSqlQuery query;
    QString request = QString("DROP TABLE '%1';").arg(_tableName);
    if(!query.exec(request)) {
        qDebug() << "Cannot delete table" << request << _tableName << query.lastError().text();
        return;
    }
}

void ISql::parsQueue()
{
    while (!queue.isEmpty()) {
        QJsonObject script = queue.first();
        qDebug() << script;
        QString _tableName = script["table"].toString();
        QString _action = script["action"].toString();
        if(_action == "load") {                             // SELECT
            QList<QPointF> _data;
            QSqlQuery query;
            QString requestX = QString("SELECT %1, %2 FROM '%3'; ").arg("X").arg("Y").arg(_tableName);
            if(!query.exec(requestX)) {
                qDebug() << "not select table requestX" << _tableName << query.lastError().text();
                return;
            }
            QSqlRecord rec = query.record();
            while(query.next()) {
                _data << QPointF(query.value(rec.indexOf("X")).toDouble(), query.value(rec.indexOf("Y")).toDouble());
            }
            CustomSet _set;
            _set.name = _tableName;
            _set.points = _data;
            emit signalSendDataTable(_set);
        } else if (_action == "save") {                     // INSERT
            if(db.tables().contains(_tableName)) {
                deleteTable(_tableName);
                createTable(_tableName);
            }
            QSqlQuery query;
            QString request;
            for(int i = 0; i < data[_tableName]->points.size(); i++) {
                request = QString("INSERT INTO '%1' ('X', 'Y', 'id_i') "
                                  "VALUES(%2, %3, %4);")
                        .arg(_tableName)
                        .arg(data[_tableName]->points.at(i).x())
                        .arg(data[_tableName]->points.at(i).y())
                        .arg(i);
                if(!query.exec(request)) {
                    qDebug() << "Insert not succes" << request << i << _tableName << query.lastError().text();;
                    break;
                }
            }
            saveWidth(_tableName);
            data.remove(_tableName);
        }


        queue.dequeue();
    }
    mutex.unlock();
    QMapIterator<QString, const CustomSet *> it(data);
    while(it.hasNext()) {
        it.next();
        qDebug().noquote() << "DEBUG:ISql " << it.key() << " size =" << it.value()->points.size();
    }
}

void ISql::loadData(QString _tableName)
{
    QJsonObject req;
    req["table"] = _tableName;
    req["action"] = "load";
    queue.enqueue(req);

    if(queue.size() == 1) {
        mutex.lock();
        parsQueue();
    }
}

void ISql::saveData(const CustomSet _set)
{
    const CustomSet *set = &_set;
    data.insert(_set.name, set);
    QJsonObject req;
    req["table"] = _set.name;
    req["action"] = "save";
    queue.enqueue(req);
    if(queue.size() == 1) {
        mutex.lock();
        parsQueue();
    }
}

void ISql::saveWidth(QString _tableName)
{
    if(data[_tableName]->width.isEmpty())
        return;
    QSqlQuery query;
    QString request;
        request = QString("INSERT INTO '%1' ('WIDTH') "
                          "VALUES(%2);")
                .arg(_tableName)
                .arg(data[_tableName]->width);
        if(!query.exec(request)) {
            qDebug() << "Insert not succes" << request << data[_tableName]->width << _tableName << query.lastError().text();;
    }
}
