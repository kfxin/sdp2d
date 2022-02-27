#include "sdp2dDataInfoTabs.h"
#include "sdp2dFileInformationTabs.h"
#include "sdp2dDisplayParamTab.h"
#include "sdp2dSegy.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "seismicdataprocessing2d.h"

#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QListWidget>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QTableView>
#include <QTableWidgetItem>
#include <QSpinBox>
#include <QComboBox>
#include <QHeaderView>

//All opened data share one file info table
Sdp2dDataInfoTabs::Sdp2dDataInfoTabs(SeismicData2D* sd2d, SeismicDataProcessing2D* mainWindow, QWidget *parent) :
    QTabWidget(parent)
{    
    //Sdp2dSegy* sgy = sd2d->getSegyhandle();
    m_statusbar = mainWindow->getStatusbarPointer();

    QIcon icon1;
    icon1.addFile(QString::fromUtf8(":/images/quit.png"), QSize(), QIcon::Normal, QIcon::Off);

    //EbcdicHeader* ehd = sgy->get3200TextHeader();
    //BinaryHeader* bhd = sgy->get400BinaryHeader();

    //m_ebcHeaderTab = new Sdp2dEbcdicHeaderTab(ehd);
    //addTab(m_ebcHeaderTab, "Ebcdic Header");
    //setTabIcon(indexOf(m_ebcHeaderTab), icon1);

    //m_binHeaderTab = new Sdp2dBinaryHeaderTab(bhd);
    //addTab(m_binHeaderTab, "Binary Header");
    //setTabIcon(indexOf(m_binHeaderTab), icon1);

    m_fileInfoTab = new Sdp2dFileInfoTab(sd2d);
    addTab(m_fileInfoTab, "Data Summary");
    setTabIcon(indexOf(m_fileInfoTab), icon1);

    //cout << "start m_displayParamTab" << endl;
    m_displayParamTab = new Sdp2dDisplayParamTab(sd2d, mainWindow);
    addTab(m_displayParamTab, "Display Setting");

    setCurrentIndex(indexOf(m_fileInfoTab));

    //setTabVisible(indexOf(m_ebcHeaderTab), false);
    //setTabVisible(indexOf(m_binHeaderTab), false);


    connect(this, &QTabWidget::tabBarDoubleClicked, this, &Sdp2dDataInfoTabs::setTabInvisible);
}

Sdp2dDataInfoTabs::~Sdp2dDataInfoTabs()
{

}
void Sdp2dDataInfoTabs::setTabInvisible(int index)
{
    if(index == 1) return;
    setTabVisible(index, false);
}

void Sdp2dDataInfoTabs::updateInformatiom(SeismicData2D* sd2d, bool forceFInfoVisible)
{
    //Sdp2dSegy* sgy = sd2d->getSegyhandle();
    //cout <<"SEGY file name: " << sgy->getSEGYFileName().c_str() << endl;

    //EbcdicHeader* ehd = sgy->get3200TextHeader();
    //BinaryHeader* bhd = sgy->get400BinaryHeader();

    //setTabVisible(indexOf(m_ebcHeaderTab), true);
    //setTabVisible(indexOf(m_binHeaderTab), true);
    if(forceFInfoVisible){
        setTabVisible(indexOf(m_fileInfoTab), true);
        setCurrentIndex(indexOf(m_fileInfoTab));
    }

    //if(m_ebcHeaderTab->isVisible())  m_ebcHeaderTab->updateDateInformation(ehd);
    //if(m_binHeaderTab->isVisible())  m_binHeaderTab->updateDateInformation(bhd);
    if(m_fileInfoTab->isVisible())  m_fileInfoTab->updateDateInformation(sd2d);
}


Sdp2dDisplayParamTab* Sdp2dDataInfoTabs::getDisplayParamTabPointer(void)
{
    return m_displayParamTab;
}

void Sdp2dDataInfoTabs::showDisplayParameterTab(void)
{
    cout << "Sdp2dDataInfoTabs::showDisplayParameterTab" << endl;
    setCurrentIndex(indexOf(m_displayParamTab));
}
