#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(QList <QPointF >)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType <QList <QPointF > >("QList <QPointF >");

    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        QMessageBox::critical(
                    this,
                    "Unable to load database",
                    "This demo needs the SQLITE driver"
                    );

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
        qDebug() << this->dataSet[_service].points.size();
//        QList<QPointF > points;
        emit signalSaveData(dataSet[_service].points, _service);
    });
    quitButton = new QPushButton(tr("Quit"));
    testButton = new QPushButton(tr("Create"));
    deleteButton = new QPushButton(tr("delete"));
    connect(quitButton, &QPushButton::clicked, this, &MainWindow::close);
    connect(testButton, &QPushButton::clicked, [this] () {
        emit signalCreateTable(this->dataSet[_service].name);
        updateBox();
    });
    connect(deleteButton, &QPushButton::clicked, [this] () {
        emit signalDeleteTable(this->dataSet[_service].name);
        updateBox();
    });
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

    initISql();

    tabWidget->setCornerWidget(pb);
    mainLayout->addWidget(chartView);
    mainLayout->addWidget(tabWidget);
    mainLayout->addLayout(hblayout);

    this->resize(640, 480);
    model->update();
}

MainWindow::~MainWindow()
{
//    qDebug() << "MainWindow deleteLater id:" << this->thread()->currentThreadId();
    _thread->quit();
    _thread->wait();
    delete ui;
}

void MainWindow::slotTest()
{
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

void MainWindow::setSeries()
{
    QString _name = dataSet[_service].name;

    chart->removeAllSeries();
    chart->legend()->hide();
    QLineSeries *series = new QLineSeries;
    series->append(dataSet[_name].points);
    series->setName(_name);
    if(!dataSet[_name].width.isEmpty()) {
        QPen pen;
        pen.setWidth(dataSet[_name].width.toInt());
        series->setPen(pen);
    }
    series->setColor(QColor(dataSet[_name].color));
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
    _set.width = "5";
    QList <QPointF> s;
    s << QPointF(2.3, 5) << QPointF(1,1) << QPointF(5,5) << QPointF(4,1)
      << QPointF(9,1) << QPointF(9.5,3) << QPointF(6.5,3) << QPointF(7,5) << QPointF(6.5,3) << QPointF(9.5,3) << QPointF(10, 5)
      << QPointF(11, 1) << QPointF(14,1) << QPointF(11,1) << QPointF(12,5) << QPointF(15, 5)
      << QPointF(18,3) << QPointF(12.75,3) <<  QPointF(18, 3) << QPointF(15, 1);
    _set.points.append(s);
//    _set.points.append(s1);
//    _set.points.append(s2);
    dataSet.insert(_service, _set);
}

void MainWindow::updateBox()
{
    cmbbox->clear();
//    qDebug() << "MainWindow updateBox id:" << this->thread()->currentThreadId();
    QStringList list = emit signalLoadTables();
    cmbbox->addItems(list);
}

void MainWindow::initISql()
{
    ISql *isql = new ISql;
    _thread = new QThread;
    connect(_thread, &QThread::started, isql, &ISql::doWork);
    connect(_thread, &QThread::finished, isql, &ISql::deleteLater);
    QObject::connect(this, &MainWindow::signalCreateTable, isql, &ISql::createTable);
    QObject::connect(this, &MainWindow::signalLoadTables, isql, &ISql::loadTables, Qt::BlockingQueuedConnection);
    QObject::connect(this, &MainWindow::signalDeleteTable, isql, &ISql::deleteTable);
    QObject::connect(this, &MainWindow::signalGetDataTable, isql, &ISql::loadData);
    QObject::connect(this, &MainWindow::signalSaveData, isql, &ISql::saveData);
    QObject::connect(isql, &ISql::signalReady, this, &MainWindow::slotReadySql);
    QObject::connect(isql, &ISql::signalSendDataTable, this, &MainWindow::initData);


    isql->moveToThread(_thread);
    _thread->start();
//    qDebug() << "MainWindow this id:" << this->thread()->currentThreadId();
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

void MainWindow::slotReadySql()
{
    updateBox();
    QStringList list = emit signalLoadTables();
    queue.append(list);


    emit signalGetDataTable(queue.dequeue());

}

void MainWindow::initData(QString _name, QList<QPointF> _points)
{
    while (!queue.isEmpty()) {
        qDebug() << _name << _points;
        emit signalGetDataTable(queue.dequeue());
    }
    mutex.unlock();
}

void MainWindow::loadTab()
{

}
