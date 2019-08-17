/*
 *  Выполняются два потока
 *  1) GUI - основной поток реализация интерфейса
 *
 *      dataSet - QMap загруженных всех данных с БД
 *      первый элемент <_service> вспомогательный для передачи параметров
 *
 *      createTab(QString table) - создать таблицу и соответсвующую вкладку
 *
 *
 *
 *  2) ISql - поток обрабатывающий очередь комманд
 *
 *
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(QList <QPointF >)
Q_DECLARE_METATYPE(CustomSet)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType <QList <QPointF > >("QList <QPointF >");
    qRegisterMetaType <CustomSet> ("CustomSet");



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
        emit signalSaveData(dataSet[this->dataSet[_service].name]);
    });
    quitButton = new QPushButton(tr("Quit"));
    testButton = new QPushButton(tr("test"));
    deleteButton = new QPushButton(tr("delete"));
    connect(quitButton, &QPushButton::clicked, this, &MainWindow::close);
    connect(testButton, &QPushButton::clicked, [this] () {
        QMapIterator<QString, CustomSet> it(this->dataSet);
        while(it.hasNext()) {
            it.next();
            qDebug().noquote() << "DEBUG:MainWindow: " << it.key() << " size =" << it.value().points.size();
        }
        updateBox();
    });
    connect(deleteButton, &QPushButton::clicked, [this] () {
        QString curTable = this->bar->tabText(this->bar->currentIndex());
        removeTab(curTable);
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
    bar = tabWidget->tabBar();
    connect(bar, &QTabBar::currentChanged, [=] () {
        QString curTable = bar->tabText(tabWidget->currentIndex());
        //            qDebug() << "TABBAR CHANGE:" << curTable << dataSet[_service].points.size();
        emit signalLockChanged();
        dataSet[_service].name = curTable;
        model->update();
        setSeries();
    });

    connect(pb, &QPushButton::clicked, [this] () {          // Добавить таблицу
        this->bar = tabWidget->tabBar();
        emit signalLockChanged();

        Dialog dialog(dataSet, this);
        dialog.show();
        int code = dialog.exec();
        if(code == QDialog::Rejected)
            return;
        else if(code == QDialog::Accepted) {
            createTab(dataSet[_service].name);
            emit signalCreateTable(dataSet[_service].name);
        }
    });

    initISql();

    tabWidget->setCornerWidget(pb);
    mainLayout->addWidget(chartView);
    mainLayout->addWidget(tabWidget);
    mainLayout->addLayout(hblayout);

    this->resize(640, 480);
    model->update();
    QString style;
}

MainWindow::~MainWindow()
{
    _thread->quit();
    _thread->wait();
    _thread->deleteLater();
    delete _thread;
    delete ui;
    qDebug() << "MainWindow deleteLater id:" << this->thread()->currentThreadId();
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
    _set.width = "10";
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

//    for(int i = 0; i < list.size(); i++) {
//        bar->addTab(list[i]);
//    }
}

void MainWindow::initISql()
{
    ISql *isql = new ISql;
    _thread = new QThread;
    connect(_thread, &QThread::started, isql, &ISql::doWork);
    connect(_thread, &QThread::finished, isql, &ISql::deleteLater);
    QObject::connect(this, &MainWindow::signalCreateTable, isql, &ISql::createTable, Qt::BlockingQueuedConnection);
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
    for(int i = 0; i < list.size(); i++)
    {
        if(list[i] == _service)
            continue;
        emit signalGetDataTable(list[i]);
    }
}

void MainWindow::initData(CustomSet _set)
{
    qDebug() << "respons" << _set.name ;
    dataSet[_set.name] = _set;
    createTab(_set.name);
}

void MainWindow::createTab(QString _tableName)
{
    int itab = this->bar->addTab(_tableName);
    this->bar->setCurrentIndex(itab);
    model->update();
    setSeries();
    updateBox();
}

void MainWindow::removeTab(QString _tableName)
{
    try {
        if(_tableName != dataSet[_service].name)
            throw 1;
        if(_tableName == _service) {
            QMessageBox msgBox(QMessageBox::Information, "Note", "Service нельзя удалить", QMessageBox::Ok, this);
            msgBox.exec();
            return;
        }
        int curIndexBar = bar->currentIndex();
        bar->removeTab(curIndexBar);
        curIndexBar++;
        if(curIndexBar < 1)
            throw 2;
        bar->setCurrentIndex(curIndexBar);
        emit signalDeleteTable(_tableName);
        dataSet.remove(_tableName);
        updateBox();

    } catch (int a) {
        switch (a) {
        case 1: qErrnoWarning("ER: dataSet does not math current tab"); break;
        case 2: qErrnoWarning("ER: removeTab want delete 0 item ('service')"); break;
        default: qDebug() << "ERROR not handled";
        }
    }

}


void MainWindow::loadTab()
{

}
