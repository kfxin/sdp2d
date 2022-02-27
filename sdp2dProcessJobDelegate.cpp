#include "sdp2dProcessJobDelegate.h"

#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QValidator>
#include <QIntValidator>
#include <QDoubleValidator>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Sdp2dProcessJobDelegate::Sdp2dProcessJobDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *Sdp2dProcessJobDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    Q_UNUSED(option);

    if(index.column() <3) return nullptr;

    QModelIndex qmidx = index.siblingAtColumn(1);
    QString type  = index.model()->data(qmidx, Qt::DisplayRole).toString();

    int typeValue = 0;
    if(type.compare("Bool") == 0) typeValue = 1;
    else if(type.compare("List") == 0 ) typeValue = 2;
    else if(type.compare("Option") == 0 ) typeValue = 3;

    if(typeValue >0) {
        QComboBox *editor = new QComboBox(parent);
        editor->setAutoFillBackground(true);
        editor->setFrame(false);

        QStringList values;
        if(typeValue == 1){
            values << "True" << "False";
        } else {
            QStringList vals = index.model()->data(qmidx, Qt::ToolTipRole).toString().split(",");
            for(int i=0; i< vals.count(); i++) values << vals.at(i).trimmed();
        }
        editor->addItems(values);
        editor->setCurrentIndex(0);
        return editor;
    } else{
        QLineEdit *editor = new QLineEdit(parent);
        editor->setCursor(Qt::IBeamCursor);

        if(type.compare("Int") == 0 ) {
            QStringList vals = index.model()->data(qmidx, Qt::ToolTipRole).toString().split("-");
            int lower = -std::numeric_limits<int>::max();
            int upper = std::numeric_limits<int>::max();
            if(vals.count() == 2){
                lower = vals.at(0).toInt();
                upper = vals.at(0).toInt();
            }
            QIntValidator *validator = new QIntValidator(lower, upper);
            editor->setValidator(validator);
        } else if(type.compare("Float") == 0 ) {
            QStringList vals = index.model()->data(qmidx, Qt::ToolTipRole).toString().split("-");
            double lower = -std::numeric_limits<double>::max();
            double upper = std::numeric_limits<double>::max();
            if(vals.count() == 2){
                lower = vals.at(0).toDouble();
                upper = vals.at(0).toDouble();
            }
            QDoubleValidator* validator = new QDoubleValidator(lower, upper, 4);
            validator->setNotation(QDoubleValidator::StandardNotation);
            editor->setValidator(validator);
        }
        return editor;
    }

    //return QStyledItemDelegate::createEditor(parent, option, index);
}

void Sdp2dProcessJobDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    if(index.column() <3) return;

    QModelIndex qmidx = index.siblingAtColumn(1);
    QString type  = index.model()->data(qmidx, Qt::DisplayRole).toString();

    int typeValue = 0;
    if(type.compare("Bool") == 0) typeValue = 1;
    else if(type.compare("List") == 0 ) typeValue = 2;
    else if(type.compare("Option") == 0 ) typeValue = 3;

    QString value = index.model()->data(index, Qt::EditRole).toString();
    if(typeValue > 0) {
        QComboBox *cBox = static_cast<QComboBox*>(editor);
        if(typeValue == 1){
            if(value.compare("True")==0) cBox->setCurrentIndex(0);
            else cBox->setCurrentIndex(1);
        }else{
            QStringList vals = index.model()->data(qmidx, Qt::ToolTipRole).toString().split(",");
            for(int i=0; i< vals.count(); i++){
                if(value.compare(vals.at(i).trimmed()) == 0) cBox->setCurrentIndex(i);
            }
        }
    } else {        
        QLineEdit *lineeditor = static_cast<QLineEdit*>(editor);
        lineeditor->setText(value);
    }
}


void Sdp2dProcessJobDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{    
    if(index.column() <3) return;
    QModelIndex qmidx = index.siblingAtColumn(1);
    QString type  = index.model()->data(qmidx, Qt::DisplayRole).toString();

    int typeValue = 0;
    if(type.compare("Bool") == 0) typeValue = 1;
    else if(type.compare("List") == 0 ) typeValue = 2;
    else if(type.compare("Option") == 0 ) typeValue = 3;

    QVariant value;

    if(typeValue > 0) {        
        QComboBox *cBox = static_cast<QComboBox*>(editor);
        int idx = cBox->currentIndex();

        if(typeValue == 1){
            if(idx == 0) value = QVariant("True");
            else value = QVariant("False");
        }else{
            QStringList vals = index.model()->data(qmidx, Qt::ToolTipRole).toString().split(",");
            value = QVariant(vals.at(idx).trimmed());
        }        
    } else {
        QLineEdit *lineeditor = static_cast<QLineEdit*>(editor);
        value = QVariant(lineeditor->text());
    }
    model->setData(index, value, Qt::EditRole);
}

void Sdp2dProcessJobDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

