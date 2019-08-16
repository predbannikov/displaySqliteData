#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString dbName = "db_name.sqlite";

    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        QMessageBox::critical(
                    this,
                    "Unable to load database",
                    "This demo needs the SQLITE driver"
                    );


    sdb = QSqlDatabase::addDatabase("QSQLITE");
    sdb.setDatabaseName(dbName);
    if (!sdb.open()) {
        qDebug() << "Not open sqlite" << dbName << sdb.lastError().text();
        return;
    }


    chart = new QChart();
    chartView = new QChartView(chart);
    chartView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    chartView->setRenderHint(QPainter::Antialiasing);
    initSet();
    setSeries();

    cmbbox = new QComboBox;
    QHBoxLayout *hblayout = new QHBoxLayout;

    QPushButton *pb_save = new QPushButton("save");
    connect(pb_save, &QPushButton::clicked, [this] () mutable {
        qDebug() << this->dataSet.size();
        saveData();
    });
    quitButton = new QPushButton(tr("Quit"));
    testButton = new QPushButton(tr("test"));
    deleteButton = new QPushButton(tr("delete"));
    connect(quitButton, &QPushButton::clicked, this, &MainWindow::close);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::slotDelete);
    connect(testButton, &QPushButton::clicked, this, &MainWindow::slotTest);
    hblayout->addWidget(quitButton);
    hblayout->addWidget(testButton);
    hblayout->addWidget(deleteButton);
    hblayout->addWidget(pb_save);
    hblayout->addWidget(cmbbox);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QWidget *window = new QWidget();
    window->setLayout(mainLayout);
    setCentralWidget(window);



    tabWidget = new QTabWidget;
    tabWidget->setMaximumHeight(110);
    tabWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    model = new ModelPairs(&dataSet, this);
    model->setTable(_service);
    table = new QTableView(this);
    DelegatEditing *delegat = new DelegatEditing(this);
    connect(this, &MainWindow::signalLockChanged, delegat, &DelegatEditing::lockEdit, Qt::DirectConnection	);
    table->setItemDelegate(delegat);
    table->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    table->resize(table->minimumSize());
    connect(table, SIGNAL(clicked(QModelIndex)), this, SLOT(clickView(QModelIndex)) );
    connect(model, &ModelPairs::dataChanged, this, &MainWindow::slotDataChanged);
    table->horizontalHeader()->setDefaultSectionSize(45);
    table->setModel(model);
    table->resizeColumnsToContents();

    QHeaderView *head = table->horizontalHeader();
    head->hide();
    tabWidget->addTab(table, QString(dataSet[_service].name));
    pb = new QPushButton();
    QTabBar *bar = tabWidget->tabBar();
    connect(bar, &QTabBar::currentChanged, [=] () {
        QString curTable = bar->tabText(tabWidget->currentIndex());
        //            qDebug() << "TABBAR CHANGE:" << curTable << dataSet[_service].points.size();
        emit signalLockChanged();
        dataSet[_service].name = curTable;
        model->update();
        setSeries();
    });

    connect(pb, &QPushButton::clicked, [=] () {
        QTabBar *bar = tabWidget->tabBar();
        emit signalLockChanged();

        Dialog dialog(dataSet, this);
        dialog.show();
        int code = dialog.exec();
        if(code == QDialog::Rejected)
            return;
        else if(code == QDialog::Accepted) {
            int itab = bar->addTab(dataSet[_service].name);
            bar->setCurrentIndex(itab);
            QString curTable = bar->tabText(tabWidget->currentIndex());
            dataSet[_service].name = curTable;
            model->update();
            setSeries();
        }
    });

    ISql *isql = new ISql;
    _thread = new QThread;
    connect(_thread, &QThread::started, isql, &ISql::doWork);
    connect(_thread, &QThread::finished, isql, &ISql::deleteLater);

    isql->moveToThread(_thread);
    _thread->start();
    qDebug() << "MainWindow this id:" << this->thread()->currentThreadId();

    tabWidget->setCornerWidget(pb);
    mainLayout->addWidget(chartView);
    mainLayout->addWidget(tabWidget);
    mainLayout->addLayout(hblayout);

    this->resize(640, 480);
    model->update();
    updateBox();
}

MainWindow::~MainWindow()
{
    _thread->quit();
    delete ui;
}

void MainWindow::slotDelete()
{
    QSqlQuery query(sdb);
    if(!query.exec("DROP TABLE " + cmbbox->currentText()))
        qDebug() << "not";
    cmbbox->clear();
    cmbbox->addItems(sdb.tables());
}

void MainWindow::slotTest()
{
////    QSqlQuery query(sdb);
////    query.exec("insert into person2 values(0, 0)");

////     QTableView *table = qobject_cast<QTableView *> (tabWidget->currentWidget());
////     ModelPairs *model = qobject_cast<ModelPairs *>(table->model());

//     QList<QPoint> list;
//     QList<QLineSeries *> series;
//     model->parsing(dataSet[dataSet[_service].name].points);

////    model
    createTable("service");

}

void MainWindow::clickView(QModelIndex index)
{
    int col = index.column();
    int icol = model->columnCount(index)-1;
//    qDebug() << "index" << col << icol;
    if(col == icol) {
//        qDebug() << "last";
        model->addCell();
    }
}

//QTableView *MainWindow::createTable()
//{
//    QSqlQuery query(sdb);
//    if(!sdb.tables().contains("chart"))
//        query.exec("create table chart (X Integer, Y Integer, id_i integer)");
//    else
//        qDebug() << "already create chart";
//    query.exec("insert into chart (X, Index) values(1, 0)");
//    query.exec("insert into chart (Y, Index) values(1, 0)");





//}

void MainWindow::setSeries()
{
    QString name = dataSet[_service].name;

    chart->removeAllSeries();
    chart->legend()->hide();
    QLineSeries *series = new QLineSeries;
    series->append(dataSet[name].points);
    series->setName(name);
    series->setColor(QColor(dataSet[name].color));
    chart->addSeries(series);
    chart->createDefaultAxes();

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment( Qt::AlignTop);
//    chart->setTitle(name);

    chartView->repaint();

}

void MainWindow::initSet()
{
    CustomSet _set;
    _set.name = _service;
    _set.color = "#00FF00";
    QList <QPointF> s;
    s << QPointF(2.3, 5) << QPointF(1,1) << QPointF(5,5) << QPointF(4,1) << QPointF(9,1) << QPointF(10,5) << QPointF(9.5,3) << QPointF(6,3) << QPointF(6.5,5);
//    QList <QPointF> s1;
//    s1 << QPointF(15, 1) << QPointF(11,1) << QPointF(12,5) << QPointF(16,5);
//    QList <QPointF> s2;
//    s2 << QPointF(17, 3) << QPointF(17.5,5) << QPointF(20,3) << QPointF(16.5,1) << QPointF(17, 3) << QPointF(13,3);
    _set.points.append(s);
//    _set.points.append(s1);
//    _set.points.append(s2);
    dataSet.insert(_service, _set);
}

void MainWindow::createTable(QString _name)
{
    QSqlQuery query;
    QString request = QString("CREATE TABLE '%1' ("
                              "'id_i' INTEGER PRIMARY KEY, 'X' REAL, 'Y' REAL);").arg(_name);
    if(!query.exec(request)) {
        qDebug() << "Unable to create table" << _name;
    }
    updateBox();
}

void MainWindow::saveData()
{
    QString _table = dataSet[_service].name;
    QSqlQuery query;
    QString request;
    for(int i = 0; i < dataSet[_table].points.size(); i++) {
        request = QString("INSERT INTO '%1' ('X', 'Y', 'id_i') "
                          "VALUES(%2, %3, %4);")
                .arg(_table)
                .arg(dataSet[_table].points[i].x())
                .arg(dataSet[_table].points[i].y())
                .arg(i);
        if(!query.exec(request)) {
            qDebug() << "Insert not succes" << request << i;
            break;
        }
    }
}

void MainWindow::updateBox()
{
    cmbbox->clear();
    cmbbox->addItems(sdb.tables());
}

void MainWindow::slotDataChanged()
{
//    dataSet[dataSet[_service].name].points = model->getPoints();
    setSeries();
//    qDebug() << "*** name" << dataSet[_service].name <<  model->getPoints();
}

bool MainWindow::containsName(QString name)
{
    return dataSet.contains(name);
}

void MainWindow::loadTab()
{

}
