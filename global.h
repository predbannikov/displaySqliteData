#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

struct CustomSet {
    QString name;
    QString color;
    QString width;
    QList <QPointF >  points;
    bool active;
};

static QString _service = "service";
static QString dbName = "db_name.sqlite";

#endif // GLOBAL_H
