#ifndef PAIRS_H
#define PAIRS_H

#include <QObject>
#include <QAbstractTableModel>
#include <QtWebSockets/QtWebSockets>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QMetaType>
#include <QPointF>
#include <QVector>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSqlTableModel>
#include <QMouseEvent>
#include <QTimer>
#include <QMutex>
#include "global.h"



struct g_point
{
public:
    g_point(): x(0), y(0), set(false) {}

    double x = 0;
    double y = 0;
    bool set = false;

};

//*********************    TableModel    ************************

class ModelPairs: public QSqlTableModel
{
    Q_OBJECT


public:


    ModelPairs(QMap<QString, CustomSet > *_dataSet, QObject* parent = nullptr);
    const int ROW = 2;
    int rowCount( const QModelIndex& ) const override;
    int columnCount( const QModelIndex&  ) const override;
    QVariant data( const QModelIndex& index, int role ) const override;
    bool setData(const QModelIndex &index,const QVariant& value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex& index ) const override;
    bool removeRows(int position, int rows, const QModelIndex &parent) override;
    bool removeColumns(int position, int column, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool insertColumns(int column, int count, const QModelIndex &parent) override;
    void clearItem();
    void applyChanges();
    enum KeyMarkets{
        X=0 , Y
    };
    QMap<QString, CustomSet > *dataSet;
//    bool chkfiltr;
//    bool chkFindProfit = 0;
    QList<g_point> g_points;
    QList<QPointF> getPoints();
    QMutex mutex;
//    QString curTable;
public slots:
    void update();
    void addCell();

public slots:



//signals:
//    void getMarket(QModelIndex);
};



class DelegatEditing : public QStyledItemDelegate  {
    Q_OBJECT

    enum LOCK {STATE_OPEN=0, STATE_LOCK};
    enum STATE {STATE_EDITING=0, STATE_WAIT};
    STATE state;
    LOCK lock;
public:
    DelegatEditing(QWidget *parent = nullptr) : QStyledItemDelegate (parent), state(STATE_WAIT), lock(STATE_OPEN){}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
signals:
    void lockedChange();

public slots:

    void lockEdit();
};



#endif // PAIRS_H
