#ifndef SDP2DMODULEPARAMETERSSETUPDLG_H
#define SDP2DMODULEPARAMETERSSETUPDLG_H


#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTreeView;
class QTextEdit;
class QDateEdit;
class QMenu;
class QTreeWidget;
class QPushButton;
class QDomDocument;
QT_END_NAMESPACE


class Sdp2dModuleParametersSetupDlg : public QDialog
{
    Q_OBJECT
public:
    explicit Sdp2dModuleParametersSetupDlg(QWidget *parent = nullptr);
    ~Sdp2dModuleParametersSetupDlg();

signals:

private:
    void setupLayout(void);
    void moduleNameValidateChecking(void);
    void updateActions(void);
    bool isModuleXMLExist(QString moduleName);
    QString createXMLFileName(bool isSave=false);
    void cleanDomDocument();

private slots:
    void onActiveMyMenu(const QPoint &pos);
    void insertChild();
    void insertRow();
    void removeRow();
    void saveModuleParameterSetting();
    void loadModuleParameterSetting();
    void validateModuleParameterSetting();
    void displayModulelChangeLog();
    void quitParaSettingDlg();

private:
    QLineEdit* m_moduleNameQLE;
    //QLineEdit* m_moduleVerQLE;
    QLineEdit* m_moduleScriptQLE;
    QLineEdit* m_moduleCreatorQLE;
    QDateEdit* m_moduleCreateQDE;
    QTextEdit* m_moduleHelpQTE;
    QTreeView* m_paraTreeView;
    QPushButton* m_saveBtn;
    QPushButton* m_loadBtn;
    QPushButton* m_clogBtn;
    QPushButton* m_quitBtn;
    QPushButton* m_valiBtn;

    QMenu* m_myMenu;
    QAction* m_removeRowAction;
    QAction* m_insertRowAction;
    QAction* m_insertChildAction;

    QDomDocument* m_domdoc;

};

#endif // SDP2DMODULEPARAMETERSSETUPDLG_H
