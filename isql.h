#ifndef ISQL_H
#define ISQL_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include "global.h"

class ISql : public QObject
{
    Q_OBJECT
public:
    explicit ISql(QObject *parent = nullptr);
    ~ISql();
public slots:
    void doWork();
};

#endif // ISQL_H
