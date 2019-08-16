//#include <QtWidgets>
#include "dialog.h"

Dialog::Dialog(QMap<QString, CustomSet > &sets, QWidget *parent) : QDialog(parent)
{
    this->setWindowFlags(Qt::Tool | Qt::Dialog);

    QHBoxLayout *hbLayout = new QHBoxLayout;
    setLayout(hbLayout);
    QLineEdit *ledit = new QLineEdit;
    ledit->setPlaceholderText("set name");
    connect(ledit, &QLineEdit::textChanged, [this](const QString str) mutable {
        this->name_set = str;
//        qDebug() << str;
    });




    ledit->setText(QString("Набор %1").arg(sets.size()+1));
    QPushButton *pbOk = new QPushButton("Ok");
    connect(pbOk, &QPushButton::clicked, [&, this]()mutable{
        if(name_set.isEmpty()) {
            QMessageBox msgBox;
            msgBox.setText("Текстовое поле не должно быть пустым");
            msgBox.exec();
        } else {
            if(sets.contains(name_set)) {
                QMessageBox msgBox;
                msgBox.setText(QString("Это '%1' имя уже используется").arg(this->name_set));
                msgBox.exec();
            } else {
                CustomSet set;
                set.name = name_set;
                set.color = "#" + color_name;
                set.active = false;
                sets.insert(this->name_set, set);
                sets[_service].name = name_set;
                sets[name_set].points = QList<QPointF>();
                emit accept();
            }
        }
    });
    QPushButton *pbClose = new QPushButton("Close");
    connect(pbClose, &QPushButton::clicked, this, &Dialog::reject);


    color_name = randomColor();
    QString t_color = inversColor(color_name);
    ClickableLabel *pbSetColor = new ClickableLabel("choose color");
    QString style = QString("QLabel {background-color : #%1; color: #%2;}").arg(color_name).arg(t_color);
    pbSetColor->setStyleSheet(style);
//    qDebug() << this->color_name << style;
    QString cname;
    connect(pbSetColor, &ClickableLabel::clicked, [=]() mutable {
//        qDebug() << QString("#" + color_name);
        QColor _color = QColorDialog::getColor(QString("#" + color_name), this, "choose color", QColorDialog::DontUseNativeDialog);
        if(!_color.isValid())
            return;
        this->color_name = _color.name(QColor::HexRgb).remove(0, 1);
        style = QString("QLabel {background-color : %1; color: #%2;}").arg(this->color_name).arg(inversColor(this->color_name));
        pbSetColor->setStyleSheet(style);
//        qDebug() << this->color_name << style;
    });

    hbLayout->addWidget(pbClose);
    hbLayout->addWidget(pbSetColor);
    hbLayout->addWidget(ledit);
    hbLayout->addWidget(pbOk);
}

QString Dialog::randomColor()
{
    QStringList list;
    list << QString::number(QRandomGenerator::global()->bounded(0xFF), 16);
    list << QString::number(QRandomGenerator::global()->bounded(0xFF), 16);
    list << QString::number(QRandomGenerator::global()->bounded(0xFF), 16);
    for(int i = 0; i < list.size(); i++)
        if(list[i].size() < 2)
            list[i].prepend("0");
    QString col = list.join("");
    return col;
}

QString Dialog::inversColor(QString col)
{
    QStringList list;
    list << QString::number(0xFF - col.left(2).toInt(nullptr, 16), 16);
    list << QString::number(0xFF - col.mid(2,2).toInt(nullptr, 16), 16);
    list << QString::number(0xFF - col.right(2).toInt(nullptr, 16), 16);
    for(int i = 0; i < list.size(); i++)
        if(list[i].size() < 2)
            list[i].prepend("0");
    QString result = list.join("");
//    qDebug() << result << col << list;
    return list.join("");
}

void Dialog::slot_ok_button()
{

}

