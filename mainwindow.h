#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QQueue>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QTableView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QtCharts/QChartView>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QThread>
#include "pairs.h"
#include "dialog.h"
#include "global.h"
#include "isql.h"

QT_CHARTS_USE_NAMESPACE

struct CurTable {
    QString table;
};


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QMap<QString, CustomSet > dataSet;
    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *testButton;
    QPushButton *quitButton;
    QPushButton *pb;
    QPushButton *deleteButton;
    QComboBox *cmbbox;
    QSqlDatabase sdb;
    CurTable tables;
    QTabWidget *tabWidget;
//    QTableView *createTable();
    QTableView *table;
    ModelPairs *model;
    QChart *chart;
    QChartView *chartView;
    QMutex mutex;
    QThread *_thread;
    void setSeries();
    void initSet();
    void updateBox();
    void initISql();
    QQueue<QString> queue;
public slots:
    void slotDataChanged();
    void loadTab();
    void slotTest();
    void clickView(QModelIndex index);
    bool containsName(QString name);
    void slotReadySql();
    void initData(QString _name, QList<QPointF > _points);
signals:
    void signalLockChanged();
    void signalCreateTable(QString);
    void signalDeleteTable(QString);
    void signalGetDataTable(QString);
    void signalSaveData(const QList <QPointF > _points, const QString _table);
    QStringList signalLoadTables();
};

#endif // MAINWINDOW_H
