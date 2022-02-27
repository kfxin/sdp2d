#ifndef SD2DPROCESSJOBDOCKWIDGET_H
#define SD2DPROCESSJOBDOCKWIDGET_H

#include <QDockWidget>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
class QComboBox;
class QPushButton;
class QDomElement;
class QTreeWidget;
class QTreeWidgetItem;
class QTextEdit;
class QCheckBox;
QT_END_NAMESPACE

class Sdp2dProcessJobTreeWidget;
class SeismicDataProcessing2D;
class SeismicData2D;
class Sdp2dQDomDocument;

class Sdp2dFilenameLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit Sdp2dFilenameLineEdit(QTreeWidgetItem* item, QWidget *parent = nullptr);
    //Sdp2dFilenameLineEdit(const QString &contents, QWidget *parent = nullptr);

private:

    void mouseDoubleClickEvent(QMouseEvent *e);

private:

    QTreeWidgetItem* m_item;

};

class Sdp2dProcessJobDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit Sdp2dProcessJobDockWidget(SeismicData2D* sd2d, SeismicDataProcessing2D *parent = nullptr);
    ~Sdp2dProcessJobDockWidget();


    void interactiveFunctionEnabled(int function);
    void interactiveFunctionDisAbled(int function);

    void setCurrentDataPointer(SeismicData2D* sd2d);
    void showOutputFileView(void);
    void cleanTreeView(void);

    Q_SLOT void runJob(void);

signals:
    //void processCurrentGather(Sdp2dQDomDocument* m_domval);
    //void processWholeData(Sdp2dQDomDocument* m_domval);

private:
    void setuplayout(void);
    void createOutputTreeitem(void);


    void setAvailableModules(void);

    void fillParametersWithDom(QTreeWidgetItem* item, QDomElement& root);
    //QTreeWidgetItem* createTreeItem(QTreeWidgetItem *parentItem, const QDomElement &element);

    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);

    bool checkItemStates(QTreeWidgetItem *item, bool quite=false);
    void setOutputParametersToSD2D(void);
    void loadDomDocument(const QString moduleName);
    void saveDomDocument(const QString moduleName);

    Q_SLOT void outputSEGYFilenameChanged();
    Q_SLOT void setProcessWholeData(int checked);

    Q_SLOT void checkRequiredParameters(bool quite=false);
    Q_SLOT void processModuleChanged(const QString moduleName);
    Q_SLOT void parameterItemClicked(QTreeWidgetItem *item, int column);

private:
    SeismicDataProcessing2D* m_mainWindow;
    SeismicData2D* m_sd2d;
    QComboBox* m_moduleList;
    QPushButton* m_runjobbtn;
    QPushButton* m_validatebtn;
    QPushButton* m_savebtn;
    QPushButton* m_loadbtn;

    QCheckBox* m_apply2all;
    Sdp2dProcessJobTreeWidget* m_paraTree;
    QTextEdit* m_infoView;

    Sdp2dQDomDocument* m_domdoc;
    Sdp2dQDomDocument* m_domval;

    bool m_processWholeData;
    int m_interactFunction;
    bool m_saveFunctionToSD2D;

    QTreeWidgetItem *m_outputitem;
    Sdp2dFilenameLineEdit* m_ofname;


};

#endif // SD2DPROCESSJOBDOCKWIDGET_H

