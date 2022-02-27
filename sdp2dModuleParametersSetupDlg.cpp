#include "sdp2dModuleParametersSetupDlg.h"
#include "sdp2dParasSettingModel.h"
#include "sdp2dParasSettingItem.h"
#include "sdp2dParasSettingDelegate.h"
#include "seismicdata2d.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QLabel>
#include <QTreeView>
#include <QTreeWidget>
#include <QGroupBox>
#include <QMenu>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>
#include <QModelIndex>

Sdp2dModuleParametersSetupDlg::Sdp2dModuleParametersSetupDlg(QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setMinimumSize(400,400);

    setupLayout();
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_domdoc = new QDomDocument("sdp2dProcessingModule");

    Sdp2dParasSettingModel* model = new Sdp2dParasSettingModel(m_domdoc, this);
    m_paraTreeView->setModel(model);
    Sdp2dParasSettingDelegate* delegate =  new Sdp2dParasSettingDelegate();
    m_paraTreeView->setItemDelegate(delegate);
    m_paraTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_paraTreeView->setExpandsOnDoubleClick(true);

    m_myMenu = new QMenu(this);
    m_insertRowAction   = m_myMenu->addAction("Insert Parameter", this, &Sdp2dModuleParametersSetupDlg::insertRow);
    m_removeRowAction   = m_myMenu->addAction("Remove Parameter", this, &Sdp2dModuleParametersSetupDlg::removeRow);
    m_insertChildAction = m_myMenu->addAction("Insert Child", this, &Sdp2dModuleParametersSetupDlg::insertChild);
    m_myMenu->addSeparator();
    m_myMenu->addAction("Expand all Child", this, [ = ] {m_paraTreeView->expandAll();});
    updateActions();
    connect(m_paraTreeView, &QTreeView::customContextMenuRequested, this,  &Sdp2dModuleParametersSetupDlg::onActiveMyMenu);

}

Sdp2dModuleParametersSetupDlg::~Sdp2dModuleParametersSetupDlg()
{

}

void Sdp2dModuleParametersSetupDlg::setupLayout(void)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // 1. module Name
    m_moduleNameQLE = new QLineEdit(this);
    QLabel* labeln = new QLabel(tr("Module Name:"));
    labeln->setBuddy(m_moduleNameQLE);
    QRegExp rx1("[a-z|A-Z]\\w+");
    QValidator *validator1 = new QRegExpValidator(rx1);
    m_moduleNameQLE->setValidator(validator1);

    // 2. module Version
    //m_moduleVerQLE = new QLineEdit("v0.1", this);
    //QLabel* labelv = new QLabel(tr("Version:"));
    //labelv->setAlignment(Qt::AlignRight);
    //labelv->setBuddy(m_moduleVerQLE);
    //QRegExp rx2("[v|V]\\d+.\\d+\\w+");
    //QValidator *validator2 = new QRegExpValidator(rx2);
    //m_moduleVerQLE->setValidator(validator2);

    m_saveBtn = new QPushButton(style()->standardIcon((enum QStyle::StandardPixmap)43), "");
    m_loadBtn = new QPushButton(style()->standardIcon((enum QStyle::StandardPixmap)30), "");
    m_clogBtn = new QPushButton(style()->standardIcon((enum QStyle::StandardPixmap)33), "");
    m_quitBtn = new QPushButton(style()->standardIcon((enum QStyle::StandardPixmap)60), "");
    m_valiBtn = new QPushButton(style()->standardIcon((enum QStyle::StandardPixmap)45), "");
    m_saveBtn->setMaximumWidth(32);
    m_loadBtn->setMaximumWidth(32);
    m_clogBtn->setMaximumWidth(32);
    m_quitBtn->setMaximumWidth(32);
    m_valiBtn->setMaximumWidth(32);

    QHBoxLayout* hlayout = new  QHBoxLayout();
    hlayout->addWidget(m_saveBtn);
    hlayout->addWidget(m_loadBtn);
    hlayout->addWidget(m_clogBtn);
    hlayout->addWidget(m_valiBtn);
    hlayout->addWidget(m_quitBtn);
    hlayout->setSpacing(1);

    // 3. module full name
    m_moduleScriptQLE  = new QLineEdit(this);
    QLabel* labelt = new QLabel(tr("Full Title:"));
    labelt->setBuddy(m_moduleScriptQLE);

    // 4. module creator
    m_moduleCreatorQLE  = new QLineEdit(this);
    QLabel* labelc = new QLabel(tr("Creator:"));
    labelc->setBuddy(m_moduleCreatorQLE);

    // 5. module created date
    m_moduleCreateQDE = new QDateEdit(QDate::currentDate());
    m_moduleCreateQDE->setDateRange(QDate(2020, 1, 1), QDate(2030, 12, 31));
    m_moduleCreateQDE->setDisplayFormat("yyyy-MMM-dd");
    QLabel* labeld = new QLabel(tr("Created Date:"));
    labeld->setAlignment(Qt::AlignRight);
    labeld->setBuddy(m_moduleCreateQDE);

    // 6. detail description
    m_moduleHelpQTE  = new QTextEdit(this);
    QLabel* labelh = new QLabel(tr("Description:"));
    m_moduleHelpQTE->setLineWrapMode(QTextEdit::WidgetWidth);
    m_moduleHelpQTE->setFixedHeight(60);
    m_moduleHelpQTE->setAcceptRichText(false);
    labelh->setBuddy(m_moduleHelpQTE);

    QLabel* labelp = new QLabel(tr("Parameters:"));
    labelp->setFixedHeight(20);

    m_paraTreeView = new QTreeView(this);
    m_paraTreeView->setRootIsDecorated(false);
    m_paraTreeView->setAlternatingRowColors(true);
    m_paraTreeView->setWordWrap(true);
    m_paraTreeView->setItemsExpandable(true);

    QGroupBox* gBox1 = new QGroupBox();
    QGridLayout *nameLayout = new QGridLayout();

    nameLayout->addWidget(labeln,            0, 0, 1, 1);
    nameLayout->addWidget(m_moduleNameQLE,   0, 1, 1, 1);

    nameLayout->addLayout(hlayout,            0, 2, 1, 1);
    //nameLayout->addWidget(m_moduleVerQLE,    0, 3, 1, 1);

    nameLayout->addWidget(labelt,            1, 0, 1, 1);
    nameLayout->addWidget(m_moduleScriptQLE, 1, 1, 1, 4);

    nameLayout->addWidget(labelc,            2, 0, 1, 1);
    nameLayout->addWidget(m_moduleCreatorQLE,2, 1, 1, 2);

    nameLayout->addWidget(labeld,            2, 3, 1, 1);
    nameLayout->addWidget(m_moduleCreateQDE, 2, 4, 1, 1);

    nameLayout->addWidget(labelh,            3, 0, 1, 1);
    nameLayout->addWidget(m_moduleHelpQTE,   3, 1, 1, 4);

    nameLayout->addWidget(labelp,            4, 0, 1, 1);
    nameLayout->addWidget(m_paraTreeView,    5, 0, 2, 5);
    gBox1->setLayout(nameLayout);

    mainLayout->addWidget(gBox1);

    setWindowTitle(tr("Build Processing Module Parameters"));
    resize(700, 500);

    connect(m_moduleNameQLE, &QLineEdit::editingFinished, this, &Sdp2dModuleParametersSetupDlg::moduleNameValidateChecking);
    connect(m_saveBtn, &QPushButton::clicked, this, &Sdp2dModuleParametersSetupDlg::saveModuleParameterSetting);
    connect(m_loadBtn, &QPushButton::clicked, this, &Sdp2dModuleParametersSetupDlg::loadModuleParameterSetting);
    connect(m_clogBtn, &QPushButton::clicked, this, &Sdp2dModuleParametersSetupDlg::displayModulelChangeLog);    
    connect(m_valiBtn, &QPushButton::clicked, this, &Sdp2dModuleParametersSetupDlg::validateModuleParameterSetting);
    connect(m_quitBtn, &QPushButton::clicked, this, [ this ]() {this->close(); });

    m_clogBtn->setVisible(false);
}


void Sdp2dModuleParametersSetupDlg::moduleNameValidateChecking()
{
    QString text = m_moduleNameQLE->text();
    QRegExp rx("[a-z|A-Z]\\w+");
    QValidator *validator = new QRegExpValidator(rx, this);

    int pos = 0;
    QValidator::State s = validator->validate(text, pos);
    if(s == QValidator::Invalid){
        cout << text.toStdString().c_str() << " is invalide" << endl;
        m_moduleNameQLE->clear();
    }
}

void Sdp2dModuleParametersSetupDlg::onActiveMyMenu(const QPoint &pos)
{
    QPoint globalPos = m_paraTreeView->mapToGlobal(pos);
    updateActions();
    m_myMenu->exec(globalPos);
}


void Sdp2dModuleParametersSetupDlg::insertChild()
{
    const QModelIndex index = m_paraTreeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = m_paraTreeView->model();

    QVariant type = QVariant(tr("String"));
    QVariant name = QVariant(tr("paramter"));

    int nchild = 0;
    if (index.isValid()) {
        Sdp2dParasSettingItem* item = static_cast<Sdp2dParasSettingItem*>(index.internalPointer());
        QString parentType  = item->data(1).toString();
        if(parentType.compare("ParaOptions") == 0) {
            type = QVariant("ParamsGroup");
            name = QVariant("paramsGroup");
        }
        nchild = item->childCount();
    }
    cout << "nchild=" << nchild << endl;
    if (!model->insertRow(nchild, index)) return;

    for (int column = 0; column < model->columnCount(index); column++) {
        const QModelIndex child = model->index(nchild, column, index);
        QVariant value;
        if(column == 0) value = name;
        else if(column == 1) value = type;
        else if(column == 5) value = QVariant(tr("Yes"));
        else value = QVariant(tr(""));
        model->setData(child, value, Qt::EditRole);
    }

    m_paraTreeView->setCurrentIndex(model->index(nchild, 0, index));

    updateActions();
}

void Sdp2dModuleParametersSetupDlg::insertRow()
{
    const QModelIndex index = m_paraTreeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = m_paraTreeView->model();

    if (!model->insertRow(index.row()+1, index.parent())) return;

    QVariant type = QVariant(tr("String"));
    QVariant name = QVariant(tr("paramter"));

    if(index.parent().isValid()){
        Sdp2dParasSettingItem *item = nullptr;
        item = static_cast<Sdp2dParasSettingItem*>(index.parent().internalPointer());
        QString parentType  = item->data(1).toString();
        if(parentType.compare("ParaOptions") == 0) {
            type = QVariant("ParamsGroup");
            name = QVariant(tr("paramsGroup"));
        }
    }

    updateActions();

    for (int column = 0; column < model->columnCount(index.parent()); column++) {
        const QModelIndex child = model->index(index.row() + 1, column, index.parent());
        QVariant value;
        if(column == 0) value = name;
        else if(column == 1) value = type;
        else if(column == 5) value = QVariant(tr("No"));
        else value = QVariant(tr(""));
        model->setData(child, value, Qt::EditRole);
    }
    m_paraTreeView->setCurrentIndex(model->index(index.row()+1, 0, index.parent()));
}

void Sdp2dModuleParametersSetupDlg::removeRow()
{
    const QModelIndex index = m_paraTreeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = m_paraTreeView->model();
    if (model->removeRow(index.row(), index.parent()))
        updateActions();
}

void Sdp2dModuleParametersSetupDlg::updateActions()
{
    const bool hasSelection = !m_paraTreeView->selectionModel()->selection().isEmpty();
    m_removeRowAction->setEnabled(hasSelection);

    const bool hasCurrent = m_paraTreeView->selectionModel()->currentIndex().isValid();
    m_insertRowAction->setEnabled(hasCurrent);

    //if (hasCurrent) {
    //    m_paraTreeView->closePersistentEditor(m_paraTreeView->selectionModel()->currentIndex());
    //}

    m_insertChildAction->setEnabled(hasCurrent);
    if (hasCurrent) {
        const QModelIndex index = m_paraTreeView->selectionModel()->currentIndex();
        QAbstractItemModel *model = m_paraTreeView->model();
        const QModelIndex typeIdx = model->index(index.row(), 1, index.parent());
        QString value = index.model()->data(typeIdx, Qt::EditRole).toString();
        if(value.compare("ParamsGroup") && value.compare("ParaOptions") )  m_insertChildAction->setEnabled(false);

    }

    if(m_paraTreeView->model()->rowCount() == 0) {
        m_insertRowAction->setEnabled(true);
    }
}

void Sdp2dModuleParametersSetupDlg::saveModuleParameterSetting()
{
    Sdp2dParasSettingModel* model = dynamic_cast<Sdp2dParasSettingModel*>(m_paraTreeView->model());
    bool validate = model->checkParametersValidate(model->getRootItem());
    if(!validate) return;

    QString filename = createXMLFileName(true);
    if(filename.count() < 4) return;
    cout << " Save to xml file: " << filename.toStdString().c_str() << endl;

    cleanDomDocument();
    QDomElement root = m_domdoc->createElement(m_moduleNameQLE->text());
    root.setAttribute(QString("Title"), m_moduleScriptQLE->text());
    root.setAttribute(QString("Creator"), m_moduleCreatorQLE->text());
    root.setAttribute(QString("CreatedDate"), m_moduleCreateQDE->date().toString("d-MMMM-yyyy"));
    root.setAttribute(QString("Description"), m_moduleHelpQTE->toPlainText());
    m_domdoc->appendChild(root);

    model->fillDomWithParameters(root, model->getRootItem());

    const int IndentSize = 4;
    QFile file(filename);
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream out(&file);
    m_domdoc->save(out, IndentSize);
    file.close();
}

void Sdp2dModuleParametersSetupDlg::loadModuleParameterSetting()
{    
    QString filename = createXMLFileName();
    if(filename.count() < 4) return;
    cout << " Load xml file: " << filename.toStdString().c_str() << endl;

    QString errorStr;
    int errorLine;
    int errorColumn;

    QFile file(filename);
    file.open(QFile::ReadOnly);
    cleanDomDocument();
    if (!m_domdoc->setContent(&file, true, &errorStr, &errorLine,&errorColumn)) {
        QMessageBox::information(window(), tr("XML Parameter Setting"),
                                 tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return;
    }
    file.close();

    QDomElement root = m_domdoc->documentElement();
    QString moduleName = m_moduleNameQLE->text();
    if(moduleName.compare(root.tagName()) != 0){
        QMessageBox message(QMessageBox::NoIcon,
                            "Error", QString("Something WRONG. The module name in the XML file is %1.").arg(root.tagName()), QMessageBox::Ok, NULL);
        message.exec();
        return;
    }
    //cout << " Root name : " << root.tagName().toStdString().c_str() << endl;

    if(root.hasAttributes()){
        m_moduleScriptQLE->setText(root.attribute("Title"));
        m_moduleCreatorQLE->setText(root.attribute("Creator"));
        m_moduleCreateQDE->setDate(QDate::fromString(root.attribute("CreatedDate"), QString("d-MMMM-yyyy")));
        m_moduleHelpQTE->setPlainText(root.attribute("Description"));
    }

    Sdp2dParasSettingModel* model = dynamic_cast<Sdp2dParasSettingModel*>(m_paraTreeView->model());

    model->removeRows(0, model->rowCount());
    model->fillParametersWithDom(root, model->getRootItem());

    for(int i=0; i<model->rowCount(); i++ ){
        QModelIndex idx = model->index(i, 0);
        m_paraTreeView->setExpanded(idx, true);
    }
}

void Sdp2dModuleParametersSetupDlg::validateModuleParameterSetting()
{
    Sdp2dParasSettingModel* model = dynamic_cast<Sdp2dParasSettingModel*>(m_paraTreeView->model());
    model->checkParametersValidate(model->getRootItem());
}

void Sdp2dModuleParametersSetupDlg::cleanDomDocument()
{
    QDomElement root = m_domdoc->documentElement();
    QDomNode n = root.firstChild();
    while(!n.isNull()) {
        m_domdoc->removeChild(n);
        n = n.nextSibling();
    }
    m_domdoc->removeChild(root);
}

void Sdp2dModuleParametersSetupDlg::displayModulelChangeLog()
{

}

void Sdp2dModuleParametersSetupDlg::quitParaSettingDlg()
{
    this->close();
}

bool Sdp2dModuleParametersSetupDlg::isModuleXMLExist(QString moduleName)
{
    QString fileName = QDir::homePath() + QString("/.sdp2d/modules");
    QFile moduleFile(fileName);
    if(moduleFile.exists()){
        if(moduleFile.open(QIODevice::ReadWrite | QIODevice::Text)){
            QTextStream modules(&moduleFile);
            while (!modules.atEnd()) {
                   QString qline = modules.readLine();
                   if(qline.compare(moduleName) == 0) return true;
            }
            modules << moduleName << "\n";
        }else{
            std::cout <<"the modules file cannot open!" << endl;
        }
    } else {
        if(moduleFile.open(QIODevice::WriteOnly | QIODevice::Text)){
            QTextStream modules(&moduleFile);
            modules << moduleName << "\n";
        }else{
            std::cout <<"the modules file cannot create!" << endl;
        }
    }
    moduleFile.close();
    return false;
}

QString Sdp2dModuleParametersSetupDlg::createXMLFileName(bool isSave)
{
    QString moduleName = m_moduleNameQLE->text();
    if(moduleName.compare("") == 0) {
        QMessageBox message(QMessageBox::NoIcon,
                            "Warn", QString("Please fill the Module Name."), QMessageBox::Ok, NULL);
        message.exec();
        return QString();
    }

    if(isModuleXMLExist(moduleName) && isSave){
        QMessageBox message(QMessageBox::NoIcon,
                            "Warn", QString("The module already created, click OK will overwrite the existing one."), QMessageBox::Ok | QMessageBox::Cancel, NULL);
        int result = message.exec();
        if(result != QMessageBox::Ok) return QString();
    }

    QString filename = QDir::homePath() + QString("/.sdp2d/") + moduleName + QString(".xml");
    return filename;
}
