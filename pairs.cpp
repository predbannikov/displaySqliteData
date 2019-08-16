#include "pairs.h"
#include <cmath>
#include <limits>
/***********************************************************************************
 ***                                 TableModel                                  **/

ModelPairs::ModelPairs(QMap<QString, CustomSet> *_dataSet, QObject *parent): QSqlTableModel(parent)
{
    dataSet =_dataSet;
}

int ModelPairs::rowCount(const QModelIndex &) const {return ROW;}

int ModelPairs::columnCount(const QModelIndex &) const {return g_points.size() ;}

QVariant ModelPairs::data(const QModelIndex &index, int role) const
{
    QVariant result;
    if (!index.isValid())
        return QVariant();
    const g_point &rec = g_points.at(index.column());
    switch(role){
//    case Qt::BackgroundRole: {
//        int key = index.column();
//        switch (key) {
//        case KeyMarkets::X:
//            if(rec.x > 0)
//                return QColor(0, 255, 0, 50);
//            else
//                return QColor(255, 0, 0, 50);

//        }
//    }
//    case Qt::DecorationRole:{}
    case Qt::DisplayRole:{
        if(!rec.set)
            return "";
        int key = index.row();
        switch( key) {
        case KeyMarkets::X:
            return rec.x;
        case KeyMarkets::Y:
            return rec.y;
        }
    }
    default:
        return QVariant();
    }
}

QVariant ModelPairs::headerData(int section, Qt::Orientation orientation, int role) const
{
    // Для любой роли, кроме запроса на отображение, прекращаем обработку
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical) {
        switch (section) {
        case KeyMarkets::X:
            return tr("X");
        case KeyMarkets::Y:
            return tr("Y");
        }
    }
    return QVariant();
}

Qt::ItemFlags ModelPairs::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;

}

bool ModelPairs::removeRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), position, position+rows-1);
    endRemoveRows();
    return true;
}

bool ModelPairs::removeColumns(int position, int column, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveColumns(QModelIndex(), position, position+column-1);
    endRemoveColumns();
    return true;
}

bool ModelPairs::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

bool ModelPairs::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertColumns(QModelIndex(), column, column + count - 1);
    endInsertColumns();
    return true;
}

void ModelPairs::clearItem()
{
    if(g_points.empty())
        return;
//    removeRows(0,currencies.count(),QModelIndex());
    removeColumns(0, g_points.count(),QModelIndex());
    g_points.clear();
}

bool ModelPairs::setData(const QModelIndex &index, const QVariant &value, int role)
{
//    qDebug() << "setData" << value;
    if (index.isValid() && role == Qt::CheckStateRole){
    } else if(role == Qt::EditRole) {

        if (!checkIndex(index))
            return false;
        int col = index.column()-1;
        if(g_points.size() == col) {
            return false;
        } else {

            switch (index.row()) {
            case 0: g_points[index.column()].x = value.toDouble(); break;
            case 1: g_points[index.column()].y = value.toDouble(); break;
            }

            g_points[index.column()].set = true;
            (*dataSet)[dataSet->value(_service).name].points = getPoints();
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

void ModelPairs::update()
{    
    QString _name = dataSet->value(_service).name;
//    qDebug() << "parsing" << curTable;
    clearItem();

    int _size = dataSet->value(_name).points.size();
    for(int k=0; k<_size; k++) {
        g_point curr;
        curr.x = dataSet->value(_name).points[k].x();
        curr.y = dataSet->value(_name).points[k].y();
        curr.set = true;
        g_points.append(curr);
    }

    g_point _app;
    g_points.append(_app);
    applyChanges();
}

void ModelPairs::addCell()
{
    g_point _app;
    g_points.append(_app);
    beginInsertColumns(QModelIndex(), g_points.size(), g_points.size());
    endInsertColumns();
}

void ModelPairs::applyChanges()
{
    int size = g_points.size();
    if(size == 0)
        return;
//    insertRows(0, size, QModelIndex());
    insertColumns(0, size, QModelIndex());
}

QList<QPointF> ModelPairs::getPoints()
{
    QList<QPointF> points;
    for (int i=0; i<g_points.size();i++) {
        if(g_points[i].set)
            points.append(QPointF(g_points[i].x,g_points[i].y));
    }
    return points;
}



QWidget *DelegatEditing::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const_cast<DelegatEditing *>(this)->state = STATE_EDITING;
//    qDebug() << "createEditor" << lock << state;
    if (index.isValid()) {
        QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
        double max = std::numeric_limits<double>::max();
        double min = std::numeric_limits<double>::min();
        editor->setRange(min+1, max);
        editor->setValue(index.data(Qt::DisplayRole).toDouble());
        return editor;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void DelegatEditing::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor); Q_UNUSED(index);
}
void DelegatEditing::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
//    qDebug() << "setModelData" << lock << state;
    if(lock == STATE_LOCK) {
        const_cast<DelegatEditing *>(this)->lock = STATE_OPEN;
        const_cast<DelegatEditing *>(this)->state = STATE_WAIT;
        return;
    }
    QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox *>(editor);
    model->setData(index, QVariant::fromValue(dsb->value()));
    const_cast<DelegatEditing *>(this)->state = STATE_WAIT;

}

void DelegatEditing::lockEdit() {
//    qDebug() << "lockEdit" << lock << state;
    if(state == STATE_EDITING)
        lock = STATE_LOCK;
//    qDebug() << "lockEdit_" << lock << state ;
}
