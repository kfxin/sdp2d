#include "sdp2dDisplayParamTab.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "seismicdataprocessing2d.h"

#include <math.h>
#include <QWidget>
#include <QObject>
#include <QVBoxLayout>
#include <QTableWidgetItem>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>

Sdp2dDisplayParamTab::Sdp2dDisplayParamTab(SeismicData2D* sd2d, SeismicDataProcessing2D* mainWindow, QWidget *parent) : QWidget(parent)
{
    double tlen = sd2d->getSegyhandle()->getTimeSampleRateInUs()/1000000.* sd2d->getSegyhandle()->getSamplesPerTrace();
    m_mainWindow = mainWindow;

    m_table =  new QTableWidget(16,2);

    const QStringList headers({tr("Display parameter"), tr("Value")});
    m_table->setHorizontalHeaderLabels(headers);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    //m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    int i=0;
    QTableWidgetItem* t1;
    t1 = new QTableWidgetItem(QString("Gather Type"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_gathertype = new QComboBox(m_table);
    m_gathertype->setFrame(false);
    QStringList strList;
    strList << "Common Shot" << "CDP" << "Common Offset"<< "Common Receiver" ;
    m_gathertype->addItems(strList);
    m_table->setCellWidget(i, 1, m_gathertype);
    i++;

    t1 = new QTableWidgetItem(QString("Display Type"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_displaytype = new QComboBox();
    m_displaytype->setFrame(false);
    strList.clear();
    strList << "Color" << "Wiggle" << "Both";
    m_displaytype->addItems(strList);
    m_table->setCellWidget(i, 1, m_displaytype);
    i++;

    t1 = new QTableWidgetItem(QString("Color Map"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_colormap = new QComboBox(m_table);
    m_colormap->setFrame(false);
    strList.clear();
    strList << "Grayscale" <<"RedBlue" <<"Hot"<< "Cold" << "Night"<< "Candy"<< "Geography" << "Ion"<< "Thermal" << "Polar"<< "Spectrum"<< "Jet" << "Hues";
    m_colormap->addItems(strList);
    m_table->setCellWidget(i, 1, m_colormap);
    i++;

    t1 = new QTableWidgetItem(QString("Wiggle scale"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_wigglescale = new QDoubleSpinBox(parent);
    m_wigglescale->setFrame(false);
    m_wigglescale->setMinimum(0.1);
    m_wigglescale->setMaximum(99);
    m_wigglescale->setValue(3);
    m_wigglescale->setDecimals(1);
    m_wigglescale->setSingleStep(0.2);
    m_table->setCellWidget(i, 1, m_wigglescale);
    i++;

    t1 = new QTableWidgetItem(QString("Clip Percentile"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_cpercentage = new QSpinBox(parent);
    m_cpercentage->setFrame(false);
    m_cpercentage->setMinimum(0);
    m_cpercentage->setMaximum(100);
    m_cpercentage->setValue(99);
    m_table->setCellWidget(i, 1, m_cpercentage);
    i++;

    t1 = new QTableWidgetItem(QString("Reverse Polarity"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_reversepolarity = new QComboBox(m_table);
    m_reversepolarity->setFrame(false);
    strList.clear();
    strList << "No" <<"Yes";
    m_reversepolarity->addItems(strList);
    m_table->setCellWidget(i, 1, m_reversepolarity);
    i++;


    t1 = new QTableWidgetItem(QString("Gather Index"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);
    m_gatherIndex = new QLineEdit();
    QIntValidator *valid = new QIntValidator(1, std::numeric_limits<int>::max());
    m_gatherIndex->setValidator(valid);
    m_table->setCellWidget(i, 1, m_gatherIndex);
    i++;

    t1 = new QTableWidgetItem(QString("Title"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);
    m_title = new QLineEdit("Shot Point 1");
    m_table->setCellWidget(i, 1, m_title);
    i++;

    t1 = new QTableWidgetItem(QString("X Label"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);
    m_xlabel = new QLineEdit("Trace Index");
    m_table->setCellWidget(i, 1, m_xlabel);
    i++;

    t1 = new QTableWidgetItem(QString("Y Label"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);
    m_ylabel = new QLineEdit("Time(s)");
    m_table->setCellWidget(i, 1, m_ylabel);
    i++;

    t1 = new QTableWidgetItem(QString("Time Length(s)"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_maxtime = new QLineEdit(m_table);
    QDoubleValidator *validator = new QDoubleValidator(m_maxtime);
    validator->setRange(0, 100, 3);
    validator->setNotation(QDoubleValidator::StandardNotation);
    //cout << "tlen="<<tlen << endl;
    m_maxtime->setValidator(validator);
    m_maxtime->setText(QString().setNum(tlen, 'f', 3));
    m_table->setCellWidget(i, 1, m_maxtime);
    i++;

    t1 = new QTableWidgetItem(QString("Display range symmetry"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_symmetryrange = new QComboBox(m_table);
    m_symmetryrange->setFrame(false);
    strList.clear();
    strList << "No" << "Yes";
    m_symmetryrange->addItems(strList);
    m_table->setCellWidget(i, 1, m_symmetryrange);
    i++;

    t1 = new QTableWidgetItem(QString("Group move step"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_groupStep = new QSpinBox(parent);
    m_groupStep->setFrame(false);
    m_groupStep->setMinimum(1);
    m_groupStep->setMaximum(999);
    m_groupStep->setValue(1);
    m_table->setCellWidget(i, 1, m_groupStep);
    i++;

    t1 = new QTableWidgetItem(QString("Points per trace"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_xscale = new QLineEdit(m_table);
    QDoubleValidator *validator2 = new QDoubleValidator(0.1, 999, 2, m_xscale);
    validator->setNotation(QDoubleValidator::StandardNotation);
    m_xscale->setValidator(validator2);
    m_xscale->setText(QString().setNum(tlen, 'f', 2));
    m_table->setCellWidget(i, 1, m_xscale);
    i++;

    t1 = new QTableWidgetItem(QString("Points per second"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_yscale = new QLineEdit(m_table);
    QDoubleValidator *validator3 = new QDoubleValidator(10, 9999, 2, m_yscale);
    validator->setNotation(QDoubleValidator::StandardNotation);
    m_yscale->setValidator(validator3);
    m_yscale->setText(QString().setNum(tlen, 'f', 2));
    m_table->setCellWidget(i, 1, m_yscale);
    i++;

    t1 = new QTableWidgetItem(QString("X Axis Type"));
    t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    m_table->setItem(i, 0, t1);

    m_xaxistype = new QComboBox(m_table);
    m_xaxistype->setFrame(false);
    strList.clear();
    strList << "Trace Index" << "Offset";
    m_xaxistype->addItems(strList);
    m_table->setCellWidget(i, 1, m_xaxistype);
    m_xaxistype->setCurrentIndex(0);
    i++;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_table);
    setLayout(layout);

    m_gathertype->setCurrentIndex(sd2d->getGatherType()-1);
    m_displaytype->setCurrentIndex(sd2d->getDisplayType()-1);
    m_colormap->setCurrentIndex(sd2d->getColorMapIndex());
    m_cpercentage->setValue(sd2d->getDataClipPercentage());
    m_maxtime->setText(QString().setNum(sd2d->getMaxDisplayTime(), 'f', 3));
    m_ylabel->setText(sd2d->getPlotYLabel());
    m_xlabel->setText(sd2d->getPlotXLabel());
    m_title->setText(sd2d->getPlotTitle());
    m_wigglescale->setValue(sd2d->getPlotWiggleScale());
    m_gatherIndex->setText(QString().setNum(sd2d->getPlotGroupIndex()));

    connect(m_cpercentage, &QSpinBox::textChanged, this, &Sdp2dDisplayParamTab::changePlotDataClipPercentage);
    connect(m_title, &QLineEdit::textChanged, this, &Sdp2dDisplayParamTab::changePlotTitle);
    connect(m_xlabel, &QLineEdit::textChanged, this, &Sdp2dDisplayParamTab::changePlotXLabel);
    connect(m_ylabel, &QLineEdit::textChanged, this, &Sdp2dDisplayParamTab::changePlotYLabel);
    connect(m_gathertype, &QComboBox::currentTextChanged, this, &Sdp2dDisplayParamTab::changePlotGatherType);
    connect(m_displaytype, &QComboBox::currentTextChanged, this, &Sdp2dDisplayParamTab::changePlotDisplayType);
    connect(m_colormap, &QComboBox::currentTextChanged, this, &Sdp2dDisplayParamTab::changePlotColormapType);
    connect(m_maxtime, &QLineEdit::editingFinished, this, &Sdp2dDisplayParamTab::changePlotMaxTime);
    connect(m_symmetryrange, &QComboBox::currentTextChanged, this, &Sdp2dDisplayParamTab::changePlotSymmetryRange);
    connect(m_groupStep, &QSpinBox::textChanged, this, &Sdp2dDisplayParamTab::changePlotGroupStep);

    connect(m_xscale, &QLineEdit::editingFinished, this, &Sdp2dDisplayParamTab::changePlotXScale);
    connect(m_yscale, &QLineEdit::editingFinished, this, &Sdp2dDisplayParamTab::changePlotYScale);

    connect(m_wigglescale, &QDoubleSpinBox::textChanged, this, &Sdp2dDisplayParamTab::changeWiggleScale);

    connect(m_xaxistype, &QComboBox::currentTextChanged, this, &Sdp2dDisplayParamTab::changeXAxisType);
    connect(m_gatherIndex, &QLineEdit::editingFinished, this, &Sdp2dDisplayParamTab::changePlotGatherIndex);
    connect(m_reversepolarity, &QComboBox::currentTextChanged, this, &Sdp2dDisplayParamTab::changePlotPolarity);
}


Sdp2dDisplayParamTab::~Sdp2dDisplayParamTab()
{

}

QString Sdp2dDisplayParamTab::getPlotTitle(void)
{
    return m_title->text();
}

QString Sdp2dDisplayParamTab::getPlotXLabel(void)
{
    return m_xlabel->text();
}

QString Sdp2dDisplayParamTab::getPlotYLabel(void)
{
    return m_ylabel->text();
}

int Sdp2dDisplayParamTab::getDataClipPercentage(void)
{
    return m_cpercentage->value();
}

int Sdp2dDisplayParamTab::getGatherType(void)
{    
    return m_gathertype->currentIndex()+1;
}

int Sdp2dDisplayParamTab::getDisplayType(void)
{    
    return m_displaytype->currentIndex()+1;
}

int Sdp2dDisplayParamTab::getColorMapIndex(void)
{    
    return m_colormap->currentIndex();
}

int Sdp2dDisplayParamTab::getSymmetryRange(void)
{    
    return m_symmetryrange->currentIndex();
}

int Sdp2dDisplayParamTab::getGroupStep(void)
{
    //cout << "getGroupStep = "<< m_groupStep->value()<< endl;
    return m_groupStep->value();
}

float Sdp2dDisplayParamTab::getMaxDisplayTime(void)
{
    return m_maxtime->text().toFloat();
}

int Sdp2dDisplayParamTab::getGatherToPlot(void)
{
    return m_xscale->text().toFloat();
}

float Sdp2dDisplayParamTab::getPlotXScale(void)
{
    return m_gatherIndex->text().toInt();
}

float Sdp2dDisplayParamTab::getPlotYScale(void)
{
    return m_yscale->text().toFloat();
}

float Sdp2dDisplayParamTab::getPlotWiggleScale(void)
{
    return m_wigglescale->value();
}

int Sdp2dDisplayParamTab::getXAxisType(void)
{
    return m_xaxistype->currentIndex()+1;
}

int Sdp2dDisplayParamTab::getReversepolarity(void)
{
    return m_reversepolarity->currentIndex();
}

void Sdp2dDisplayParamTab::setPlotTitle(QString text)
{
    if(getPlotTitle().compare(text) == 0) return;
    //cout << "m_title set text:" << text.toStdString().c_str() << endl;
    m_title->setText(text);
}

void Sdp2dDisplayParamTab::setPlotXLabel(QString text)
{
    if(getPlotXLabel().compare(text) == 0) return;
    m_xlabel->setText(text);
}

void Sdp2dDisplayParamTab::setPlotYLabel(QString text)
{
    //cout << "set Y label as " << text.toStdString().c_str()<< endl;
    if(getPlotYLabel().compare(text) == 0) return;
    m_ylabel->setText(text);
}

void Sdp2dDisplayParamTab::setDataClipPercentage(int value)
{
    if(getDataClipPercentage() == value) return;
    m_cpercentage->setValue(value);
}

void Sdp2dDisplayParamTab::setGatherType(int gatherType)
{
    if(m_gathertype->currentIndex() == (gatherType-1)) return;
    m_gathertype->setCurrentIndex(gatherType-1);
}

void Sdp2dDisplayParamTab::setDisplayType(int displayType)
{
    if(m_displaytype->currentIndex() == (displayType-1)) return;
    m_displaytype->setCurrentIndex(displayType-1);
}

void Sdp2dDisplayParamTab::setColorMapIndex(int colormapIdx)
{
    if(m_colormap->currentIndex() == colormapIdx) return;
    m_colormap->setCurrentIndex(colormapIdx);
}


void Sdp2dDisplayParamTab::setSymmetryRange(int symmetryRange)
{
    if(m_symmetryrange->currentIndex() == symmetryRange) return;
    m_symmetryrange->setCurrentIndex(symmetryRange);
}


void Sdp2dDisplayParamTab::setGroupStep(int value)
{
    if(m_groupStep->value() == value) return;
    m_groupStep->setValue(value);
}


void Sdp2dDisplayParamTab::setMaxDisplayTime(float tlen)
{
    if(abs(m_maxtime->text().toFloat() - tlen) < 0.0001) return;
    m_maxtime->setText(QString().setNum(tlen, 'f', 3));
}

void Sdp2dDisplayParamTab::setGatherToPlot(int val)
{
    if((m_gatherIndex->text().toInt() - val) == 0) return;
    m_gatherIndex->setText(QString().setNum(val));
}

void Sdp2dDisplayParamTab::setPlotXScale(float val)
{
    if(abs(m_xscale->text().toFloat() - val) < 0.001) return;
    m_xscale->setText(QString().setNum(val, 'f', 2));
}

void Sdp2dDisplayParamTab::setPlotYScale(float val)
{
    if(abs(m_yscale->text().toFloat() - val) < 0.001) return;
    m_yscale->setText(QString().setNum(val, 'f', 2));
}

void Sdp2dDisplayParamTab::setPlotWiggleScale(float val)
{
    if(m_wigglescale->value() == val) return;
    m_wigglescale->setValue(val);
}

void Sdp2dDisplayParamTab::setXAxisTypex(int xaxisType)
{
    if(m_xaxistype->currentIndex() == (xaxisType-1)) return;
    m_xaxistype->setCurrentIndex(xaxisType-1);
}

void Sdp2dDisplayParamTab::setReversepolarity(int polarity)
{
    if(m_symmetryrange->currentIndex() == polarity) return;
    m_reversepolarity->setCurrentIndex(polarity);
}

void Sdp2dDisplayParamTab::changePlotDataClipPercentage(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    sd2d->setDataClipPercentage(text.toInt());
    //cout << "new spinbox value: "<<text.toStdString().c_str() << endl;
}

void Sdp2dDisplayParamTab::changePlotTitle(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    sd2d->setPlotTitle(text);
    //cout << "changePlotTitle value: " << text.toStdString().c_str() << endl;
}

void Sdp2dDisplayParamTab::changePlotXLabel(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    sd2d->setPlotXLabel(text);
    //cout << "changePlotXLabel value: " << text.toStdString().c_str() << endl;
}

void Sdp2dDisplayParamTab::changePlotYLabel(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    sd2d->setPlotYLabel(text);
    //cout << "changePlotYLabel value: " << text.toStdString().c_str() << endl;
}

void Sdp2dDisplayParamTab::changePlotGatherType(QString text)
{
    // current gather type is only work for prestack data.
    // need further implementation for stack and attributes.
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    int gatherType = 1;        
    if(m_mainWindow->getInteractiveFunction() == InteractiveFunctions::StackVelAnalysis){
        gatherType=2;
        m_gathertype->setCurrentIndex(gatherType-1);
        sd2d->setGatherType(gatherType, 10);
    } else {
        if(text.compare("CDP")==0) gatherType=2;
        else if(text.compare("Common Offset")==0) gatherType=3;
        else if(text.compare("Common Receiver")==0) gatherType=4;

        if(text.compare("Common Offset")==0){
            setXAxisTypex(1);
        }
        sd2d->setGatherType(gatherType);
    }

}

void Sdp2dDisplayParamTab::changePlotDisplayType(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    //cout << "changePlotDisplayType value: " << text.toStdString().c_str() << endl;
    int displayType = 1;
    if(text.compare("Wiggle")==0) displayType=2;
    if(text.compare("Both")==0) displayType=3;
    if(text.compare("Wiggle") != 0 ) setXAxisTypex(1);
    sd2d->setDisplayType(displayType);
}

void Sdp2dDisplayParamTab::changePlotColormapType(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    int colormapIdx = 0;
    if(text.compare("RedBlue")==0)        colormapIdx=1;
    else if(text.compare("Hot")==0)       colormapIdx=2;
    else if(text.compare("Cold")==0)      colormapIdx=3;
    else if(text.compare("Night")==0)     colormapIdx=4;
    else if(text.compare("Candy")==0)     colormapIdx=5;
    else if(text.compare("Geography")==0) colormapIdx=6;
    else if(text.compare("Ion")==0)       colormapIdx=7;
    else if(text.compare("Thermal")==0)   colormapIdx=8;
    else if(text.compare("Polar")==0)     colormapIdx=9;
    else if(text.compare("Spectrum")==0)  colormapIdx=10;
    else if(text.compare("Jet")==0)       colormapIdx=11;
    else if(text.compare("Hues")==0)      colormapIdx=12;
    //cout << "changePlotColormapType="<< text.toStdString().c_str() << "  value: " << colormapIdx << endl;
    sd2d->setColorMapIndex(colormapIdx);
}


void Sdp2dDisplayParamTab::changePlotSymmetryRange(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    //cout << "changePlotDisplayType value: " << text.toStdString().c_str() << endl;
    int symmetryRange = 1;
    if(text.compare("No")==0) symmetryRange=0;
    sd2d->setSymmetryRange(symmetryRange);
}

void Sdp2dDisplayParamTab::changePlotGroupStep(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    sd2d->setGroupStep(text.toInt());
    //cout << "new spinbox value: "<<text.toStdString().c_str() << endl;
}

void Sdp2dDisplayParamTab::changePlotMaxTime()
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    //cout << "setPlotMaxTimetoDataPointer: " << m_maxtime->text().toFloat() << endl;
    float tlen = m_maxtime->text().toFloat();
    float maxTime = (sd2d->getSamplesPerTraces()-1) * sd2d->getTimeSampleRateInUs() /1000000.;
    if(tlen > maxTime || tlen < 0.004){
        QMessageBox message(QMessageBox::NoIcon,
                            "Warn", QString("The max display time should be smaller than %1s").arg(maxTime), QMessageBox::Ok, NULL);
        message.exec();
        tlen = sd2d->getMaxDisplayTime();
        this->setMaxDisplayTime(tlen);
    } else {
        sd2d->setMaxDisplayTime(tlen);
    }
}

void Sdp2dDisplayParamTab::changePlotGatherIndex()
{    
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    int gIdx =  m_gatherIndex->text().toInt();

    if(m_mainWindow->getInteractiveFunction() == InteractiveFunctions::StackVelAnalysis){
        checkCDPGatherIndexForVelAna(gIdx);
    } else {
        sd2d->setPlotGroupIndex(gIdx);
    }
}

void Sdp2dDisplayParamTab::changePlotXScale()
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    float scale = m_xscale->text().toFloat();
    float lower = 0.1;
    float upper = 999;
    if(sd2d->getDisplayType() == 1){
        lower = 0.1;
    }else{
        lower = 3;
    }
    if(scale > upper || scale < lower){
        QMessageBox message(QMessageBox::NoIcon,
                            "Warn", QString("The max display time should be inbetween %1~%2").arg(lower).arg(upper), QMessageBox::Ok, NULL);
        message.exec();
        scale = sd2d->getPlotXScale();
        this->setPlotXScale(scale);
    }else {
        sd2d->setPlotXScale(scale);
    }
}

void Sdp2dDisplayParamTab::changePlotYScale()
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    //cout << "Sdp2dDisplayParamTab::setPlotYScaleDataPointer scale=" <<  m_yscale->text().toFloat() << endl;
    float scale = m_yscale->text().toFloat();
    float lower = 10;
    float upper = 9999;
    if(scale > upper || scale < lower){
        QMessageBox message(QMessageBox::NoIcon,
                            "Warn", QString("The max display time should be sinbetween %1~%2").arg(lower).arg(upper), QMessageBox::Ok, NULL);
        message.exec();
        scale = sd2d->getPlotYScale();
        this->setPlotYScale(scale);
    } else {
        sd2d->setPlotYScale(scale);
    }
}

void Sdp2dDisplayParamTab::changeWiggleScale(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    sd2d->setPlotWiggleScale(text.toFloat());
    //cout << "new spinbox value: "<<text.toStdString().c_str() << endl;
}

void Sdp2dDisplayParamTab::changeXAxisType(QString text)
{
    if(m_gathertype->currentIndex() == 2){
        m_xaxistype->setCurrentIndex(0);
        return;
    }
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    int xaxisType = 1;
    if(text.compare("Offset")==0) {
        xaxisType=2;
        setDisplayType(2);
        setPlotXLabel(QString("Offset(m)"));
    } else {
        setPlotXLabel(QString("Trace Index"));
    }
    sd2d->setXAxisType(xaxisType);
}

void Sdp2dDisplayParamTab::changePlotPolarity(QString text)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    int polarity = 0;
    if(text.compare("Yes")==0) polarity=1;
    sd2d->setReversepolarity(polarity);
}

void Sdp2dDisplayParamTab::checkCDPGatherIndexForVelAna(int gIdx)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    if(sd2d == nullptr) return;

    if(sd2d->getDataType() != SeismicDataType::PreStack) return;
    SeismicData2DPreStack* sd2dp = dynamic_cast<SeismicData2DPreStack*>(sd2d);
    gathers* gpcdp = sd2dp->getCDPGatherIndex();

    int minNtraces = 10;

    int gIdxMin = gpcdp->minGroupValue;
    while( sd2dp->getNumberOfTracesOfGather(DisplayGatherType::CommonDepthPoint, gIdxMin) <= minNtraces ) gIdxMin++;

    int gIdxMax = gpcdp->maxGroupValue;
    while( sd2dp->getNumberOfTracesOfGather(DisplayGatherType::CommonDepthPoint, gIdxMax) <= minNtraces ) gIdxMax--;

    if(gIdx == 0) gIdx = sd2dp->getPlotGroupIndex();
    if(gIdx < gIdxMin){
        sd2dp->setPlotGroupIndex(gIdxMin);
    } else if(gIdx > gIdxMax) {
        sd2dp->setPlotGroupIndex(gIdxMax);
    } else {
        sd2dp->setPlotGroupIndex(gIdx);
    }
}
