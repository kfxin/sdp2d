#include "sdp2dParasSettingDelegate.h"
#include "sdp2dParasSettingItem.h"

#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Sdp2dParasSettingDelegate::Sdp2dParasSettingDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *Sdp2dParasSettingDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    Q_UNUSED(option);

    if(index.column() == 1){
        QComboBox *editor = new QComboBox(parent);
        editor->setFrame(false);
        QStringList values1;
        values1 << "String" << "Int" << "Float"<< "Bool" << "StringList" << "ParamsGroup" << "ParaOptions"  ;
        if(index.parent().isValid()){
            Sdp2dParasSettingItem* item = static_cast<Sdp2dParasSettingItem*>(index.parent().internalPointer());
            if(item->data(1).toString().compare("ParaOptions") == 0) {
                values1.clear();
                values1 << "ParamsGroup"  ;
            }
        }
        editor->addItems(values1);

        return editor;
    } else if(index.column() == 4 || index.column() == 5){
        QCheckBox *editor = new QCheckBox(parent);
        editor->setChecked(false);
        editor->setAutoFillBackground(true);
        return editor;
    } else if(index.column() == 6){
        QTextEdit *editor = new QTextEdit(parent);
        editor->setAutoFillBackground(true);
        return editor;
    } else {
        QLineEdit *editor = new QLineEdit(parent);
        editor->setFrame(false);
        return editor;
    }
}

void Sdp2dParasSettingDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    if(index.column() == 1){
        QComboBox *cBox = static_cast<QComboBox*>(editor);
        if(index.parent().isValid()){
            Sdp2dParasSettingItem* item = static_cast<Sdp2dParasSettingItem*>(index.parent().internalPointer());
            if(item->data(1).toString().compare("ParaOptions") == 0) {
                cBox->setCurrentIndex(0);
                return;
            }
        }
        if(value.compare("String")==0) cBox->setCurrentIndex(0);
        else if(value.compare("Int")==0) cBox->setCurrentIndex(1);
        else if(value.compare("Float")==0) cBox->setCurrentIndex(2);
        else if(value.compare("Bool")==0) cBox->setCurrentIndex(3);
        else if(value.compare("StringList")==0) cBox->setCurrentIndex(4);
        else if(value.compare("ParamsGroup")==0) cBox->setCurrentIndex(5);
        else if(value.compare("ParaOptions")==0) cBox->setCurrentIndex(6);
    } else if(index.column() == 4 ){
        QModelIndex idx = index.siblingAtColumn(1);
        QCheckBox *cBox = static_cast<QCheckBox*>(editor);
        if(idx.data().toString().compare("ParamsGroup") == 0){
            if(value.compare("True")) cBox->setChecked(true);
            else cBox->setChecked(false);
        }else{
            cBox->setChecked(false);
            QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
            model->setData(index, QVariant(""), Qt::DisplayRole);
        }
    } else if(index.column() == 5){
        QCheckBox *cBox = static_cast<QCheckBox*>(editor);
        if(value.compare("True")) cBox->setChecked(true);
        else cBox->setChecked(false);
    } else if(index.column() == 6){
        QTextEdit *texteditor = static_cast<QTextEdit*>(editor);
        texteditor->setDocument(new QTextDocument(value));
    } else {
        QLineEdit *lineeditor = static_cast<QLineEdit*>(editor);
        lineeditor->setText(value);
    }
}


void Sdp2dParasSettingDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{    
    //cout << " setModelData 1: col=" << index.column() << " row=" << index.row() << " value="<< model->data(index, Qt::EditRole).toString().toStdString().c_str()<< endl;

    QVariant value;
    if(index.column() == 1){
        QComboBox *cBox = static_cast<QComboBox*>(editor);
        if(index.parent().isValid()){
            Sdp2dParasSettingItem* item = static_cast<Sdp2dParasSettingItem*>(index.parent().internalPointer());
            if(item->data(1).toString().compare("ParaOptions") == 0) {
                model->setData(index, QVariant("ParamsGroup"), Qt::EditRole);
                return;
            }
        }
        int idx = cBox->currentIndex();
        if(idx == 0) value = QVariant("String");
        else if(idx == 1) value = QVariant("Int");
        else if(idx == 2) value = QVariant("Float");
        else if(idx == 3) value = QVariant("Bool");
        else if(idx == 4) value = QVariant("StringList");
        else if(idx == 5) value = QVariant("ParamsGroup");
        else if(idx == 6) value = QVariant("ParaOptions");
    } else if(index.column() == 4 ){
        QModelIndex idx = index.siblingAtColumn(1);
        QCheckBox *cBox = static_cast<QCheckBox*>(editor);
        if(idx.data().toString().compare("ParamsGroup") == 0){
            bool ischecked = cBox->isChecked();
            if(ischecked) value = QVariant("Yes");
            else  value = QVariant("No");
        } else {
            cBox->setChecked(false);
            value = QVariant("");
        }
    } else if(index.column() == 5){
        QCheckBox *cBox = static_cast<QCheckBox*>(editor);
        bool ischecked = cBox->isChecked();
        if(ischecked) value = QVariant("Yes");
        else  value = QVariant("No");
    } else if(index.column() == 6){
        QTextEdit *texteditor = static_cast<QTextEdit*>(editor);
        value = texteditor->toPlainText();
    } else {
        QLineEdit *lineeditor = static_cast<QLineEdit*>(editor);
        value = lineeditor->text();
    }

    model->setData(index, value, Qt::EditRole);

    //cout << " setModelData 2: col=" << index.column() << " row=" << index.row() << " value="<< model->data(index, Qt::EditRole).toString().toStdString().c_str()<< endl;
}

void Sdp2dParasSettingDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
