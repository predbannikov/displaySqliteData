#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QRandomGenerator>
#include <QGroupBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QToolBox>
#include <QToolBox>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QDebug>
#include <QChartView>
#include "global.h"


class Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QMap<QString, CustomSet > &set, QWidget *parent = nullptr);
private:
protected:
    QString color_name;
    QString name_set;
    QString randomColor();
    QString inversColor(QString col);
//    QString style;
    void setStyleColorBt();
signals:

public slots:
    void slot_ok_button();
};

class ClickableLabel : public QLabel
{
Q_OBJECT
public:
    explicit ClickableLabel( const QString& text="", QWidget* parent=nullptr ) : QLabel(parent) {setText(text);}
    ~ClickableLabel() {}
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* event) { Q_UNUSED(event); emit clicked();}
};

#endif // DIALOG_H
