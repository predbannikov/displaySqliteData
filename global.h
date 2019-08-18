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

static const QString _service = "service";
static const QString dbName = "db_name.sqlite";
static const QString fileNameConfig = "config_data";
static const QString appConfig = "config_app";
static const QString appDirName = "displayChartSqlite";

#endif // GLOBAL_H
