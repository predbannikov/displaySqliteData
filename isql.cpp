#include "isql.h"

ISql::ISql(QObject *parent): QObject (parent)
{

}

void ISql::doWork()
{
    qDebug() << "ISql doWork id:" << this->thread()->currentThreadId();
}

ISql::~ISql()
{
    qDebug() << "ISql deleteLater id:" << this->thread()->currentThreadId();

}
