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

    if(!prepWorkPath()) {
        qDebug() << "exit";
        exit(1);
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
    connect(pb_save, &QPushButton::clicked, [this] () {                     // Save
        if(this->dataSet[_service].name == _service) {
            QMessageBox::warning(
                        this,
                        "Note",
                        "service таблицу нельзя сохранить"
                        );
            return;
        }
//        bar->currentIndex();
        bar->setTabIcon(bar->currentIndex(), QIcon());
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
        qDebug() << jDataConfig;
//        updateBox();
    });
    connect(deleteButton, &QPushButton::clicked, [this] () {                    // remove
        QString curTable = this->bar->tabText(this->bar->currentIndex());
        removeTab(curTable);
    });
    QPushButton *pb_load = new QPushButton(tr("Загрузить"));
    connect(pb_load, &QPushButton::clicked, [this] () {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open sqlite db"), QApplication::applicationDirPath());
        if(!fileName.isEmpty()) {
            QEventLoop loop;
            connect(isql, &ISql::finished, &loop, &QEventLoop::quit);
            _thread->quit();
            _thread->wait();
            loop.exec();
            _thread->deleteLater();
            addNewDB(fileName);
            resetWindow();
            initISql(jConfig["pathSql"].toString());
        }
    });

//    hblayout->addWidget(quitButton);
//    hblayout->addWidget(testButton);
    hblayout->addWidget(pb_save);
    hblayout->addWidget(pb_load);
    hblayout->addWidget(deleteButton);
    hblayout->addStretch();
//    hblayout->addWidget(cmbbox);

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
    pb = new QPushButton();         // Кнопка добавить таблицу
    QString  pixmapstr = ":/addico.png";
    QPixmap pixmap(pixmapstr);
    QIcon ico = QIcon(pixmap);
    pb->setIcon(ico);
    pb->setIconSize(QSize(15, 15));

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
        dialog.setModal(true);
//        dialog.show();
        int code = dialog.exec();
        if(code == QDialog::Rejected)
            return;
        else if(code == QDialog::Accepted) {
            createTab(dataSet[_service].name);
            saveConfig();
            emit signalCreateTable(dataSet[_service].name);
        }
    });

    initISql(jConfig["pathSql"].toString());

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
    delete _thread;
    delete ui;
    qDebug() << "MainWindow deleteLater id:" << this->thread()->currentThreadId();
}

QString MainWindow::getIndexCurName()
{
    QString index;
    QJsonArray array = jConfig["dbPaths"].toArray();
    for(int i = 0; i < array.size(); i++) {
        QJsonObject jobj = array[i].toObject();
        if(jobj["path"].toString() == jConfig["pathSql"].toString()) {
            index = jobj["index"].toString();
        }
    }
    return index;
}

void MainWindow::addNewDB(QString db_path )
{
    jConfig["pathSql"] = db_path;
    QJsonArray jCurArr = jConfig["dbPaths"].toArray();
    int i = 0;
    QJsonObject obj;
    for(; i < jCurArr.size(); i++) {
        obj = jCurArr[i].toObject();
        if(obj[QString::number(i)].toString() == db_path)
            break;
    }
    if(i == jCurArr.size()) {
        obj = QJsonObject();
        obj[QString::number(i)] = db_path;
        obj["pathSqlConf"] = jConfig["pathDirData"].toString() + "/" + QString::number(i);
        jCurArr.append(obj);
        jConfig["dbPaths"] = jCurArr;
    }
    jConfig["pathSqlConf"] = obj["pathSqlConf"].toString();
    saveAppConfig();
}

void MainWindow::loadAppConfig()
{
    QFile file(jConfig["pathApp"].toString());
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "not open file to read app config";
        return;
    }
    jConfig = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
}

void MainWindow::saveAppConfig()
{
    QFile file(jConfig["pathApp"].toString());
    if(!file.open(QFile::WriteOnly | QFile::Text)) {
        qDebug() << "not open file to save app config";
        return;
    }
    file.write(QJsonDocument(jConfig).toJson());
    file.close();
}

bool MainWindow::prepWorkPath()
{
    QString pathApp = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/" + appDirName + "/" + appConfig;
    jConfig["pathApp"] = pathApp;

    if(!QFileInfo::exists(pathApp)) {               // Если папка отсутствует инициализируем по умолчанию
        QDir pathAppDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/" + appDirName);
        if(!pathAppDir.exists())
            pathAppDir.mkdir(pathAppDir.path());
        QDir pathDirData(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/" + appDirName);
        if(!pathDirData.exists())
            pathDirData.mkdir(pathDirData.path());
        jConfig["pathDirData"] = pathDirData.path();
        QString pathSql = jConfig["pathDirData"].toString() + "/" + dbName;
        jConfig["mode"] = "home";
        jConfig["dbPaths"] = QJsonArray();
        addNewDB(pathSql);
//        QJsonArray jarray;
//        QJsonObject jobj;
//        jobj["0"] = jConfig["pathSql"].toString();
//        jarray.append(jobj);
//        jConfig["dbPaths"] = jarray;
//        jConfig["pathSqlConf"] = pathDirData.path() + "/" + "0";
//        saveAppConfig();
    } else {
        loadAppConfig();
    }

    qDebug() << jConfig["pathApp"].toString();
    qDebug() << jConfig["pathDirData"].toString();
    qDebug() << jConfig["pathSql"].toString();


//    return false;

//    QString curPath = QApplication::applicationDirPath();
//    if(!QFileInfo::exists(curPath + "/" +fileNameConfig)) {            // 1
//        QString nameConfig = curPath + "/" + fileNameConfig;
//        qDebug() << "nameConfig" << nameConfig;
//        QFile fileConfig(nameConfig);
//        if(!fileConfig.open(QFile::WriteOnly | QFile::Text)) {
//            int ret = QMessageBox::warning(this, "Warning",
//                                           "В текущей дирректории не удаётся сохранить данные, желаете работать c домашним каталогом?",
//                                           QMessageBox::Ok, QMessageBox::Cancel);
//            switch (ret) {
//            case QMessageBox::Cancel:
//                qDebug() << "cancel";
//                return false;
//            case QMessageBox::Ok:
//                pathData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
//                QDir dir(pathData);
//                if(!dir.exists(pathData)) {
//                    qDebug() << "path not exists";
//                    dir.mkdir(pathData);
//                }
//                qDebug() << "PATH: " << pathData;
//                qDebug() << "CONFIG: " << pathData + "/" + fileNameConfig;
//                qDebug() << "SQL: " << pathData + "/" + dbName;
//                return true;

//            }
//        }
//        fileConfig.close();
//    }
//    pathData = curPath = QApplication::applicationDirPath();;
//    qDebug() << "PATH: " << curPath;
//    qDebug() << "CONFIG: " << curPath + "/" + fileNameConfig;
//    qDebug() << "SQL: " << curPath + "/" + dbName;
    return true;
}

void MainWindow::saveConfig()
{
    QJsonArray jarray;
    QMapIterator<QString, CustomSet > it(dataSet);
    while(it.hasNext()) {
        it.next();
        if(it.key() == _service)
            continue;
        QJsonObject jobj;
        jobj["name"] = it.value().name;
        jobj["color"] = it.value().color;
        jobj["width"] = it.value().width;
        jarray.append(jobj);
    }
    jDataConfig = jarray;
    updateConfig();
}

void MainWindow::loadConfig()
{
    QFile fileRead(jConfig["pathSqlConf"].toString());
    if(!fileRead.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Error config file not open to read";
        return;
    }
    QJsonDocument jdoc = QJsonDocument::fromJson(fileRead.readAll());
    fileRead.close();
    QJsonArray jarray = jdoc.array();
    QJsonArray matching;
    for(QString _tableName: dataSet.keys()) {
        for(int i = 0; i < jarray.size(); i++)
            if(jarray[i].toObject()["name"].toString() == _tableName) {
                dataSet[_tableName].color = jarray[i].toObject()["color"].toString();
                dataSet[_tableName].width = jarray[i].toObject()["width"].toString();
            }
    }
    jDataConfig = matching;
}

void MainWindow::resetWindow()
{
    QStringList list = dataSet.keys();
    for(QString str: list) {
        if(str == _service)
            continue;
        int curIndexBar = bar->currentIndex();
        bar->removeTab(curIndexBar);
        curIndexBar++;
        bar->setCurrentIndex(curIndexBar);
        dataSet.remove(str);
    }
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

}

void MainWindow::initISql(QString _path)
{
    isql = new ISql(_path);
//    QPointer<ISql* > = new ISql(_path);
    _thread = new QThread;
    connect(_thread, &QThread::started, isql, &ISql::doWork);
    connect(_thread, &QThread::finished, isql, &ISql::deleteLater);
    connect(isql, &ISql::finished, isql, &ISql::deleteLater);
    QObject::connect(this, &MainWindow::signalCreateTable, isql, &ISql::createTable, Qt::BlockingQueuedConnection);
    QObject::connect(this, &MainWindow::signalLoadTables, isql, &ISql::loadTables, Qt::BlockingQueuedConnection);
    QObject::connect(this, &MainWindow::signalDeleteTable, isql, &ISql::deleteTable);
    QObject::connect(this, &MainWindow::signalGetDataTable, isql, &ISql::loadData);
    QObject::connect(this, &MainWindow::signalSaveData, isql, &ISql::saveData);
    QObject::connect(this, &MainWindow::signalExitThread, isql, &ISql::exit);
    QObject::connect(isql, &ISql::signalReady, this, &MainWindow::slotReadySql);
    QObject::connect(isql, &ISql::signalSendDataTable, this, &MainWindow::initData);


    isql->moveToThread(_thread);
    _thread->start();
//    qDebug() << "MainWindow this id:" << this->thread()->currentThreadId();
}

void MainWindow::slotDataChanged()
{
//    dataSet[dataSet[_service].name].points = model->getPoints();
    QPixmap pixmap(":/modified.png");
    QIcon ico = QIcon(pixmap);
    bar->setTabIcon(bar->currentIndex(), ico);
    bar->setIconSize(QSize(15, 15));
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
//    qDebug() << "respons" << _set.name ;
    dataSet[_set.name] = _set;
    loadConfig();
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
        saveConfig();

    } catch (int a) {
        switch (a) {
        case 1: qErrnoWarning("ER: dataSet does not math current tab"); break;
        case 2: qErrnoWarning("ER: removeTab want delete 0 item ('service')"); break;
        default: qDebug() << "ERROR not handled";
        }
    }

}

void MainWindow::updateConfig()
{
    QFile fileWrite(jConfig["pathSqlConf"].toString());
    if(!fileWrite.open(QFile::WriteOnly | QFile::Text)) {
        qDebug() << "Error config file not open to write";
        return;
    }
    fileWrite.write(QJsonDocument(jDataConfig).toJson());
    fileWrite.close();
}


void MainWindow::loadTab()
{

}
