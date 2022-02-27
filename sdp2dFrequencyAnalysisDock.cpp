#include "sdp2dFrequencyAnalysisDock.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "seismicdataprocessing2d.h"
#include "sdp2dQDomDocument.h"
#include "qcustomplot.h"

using namespace std;

Sdp2dFrequencyAnalysisDock::Sdp2dFrequencyAnalysisDock(SeismicData2D* sd2d, bool visible, QWidget *parent) :
    QDockWidget(parent)
{
    setWindowTitle("Frequency Analysis");
    setContextMenuPolicy(Qt::CustomContextMenu);
    setContentsMargins(5,5,5,5);
    setMinimumSize(400,300);
    setFloating(true);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

    m_mainWindow = dynamic_cast<SeismicDataProcessing2D*>(parent);
    m_statusbar = m_mainWindow->getStatusbarPointer();

    m_isHide = true;
    m_hasComparison = false;

    m_curveType = FrequencyCurveType::Amplitude;
    m_normalizeAmp = false;
    m_noCurveType = 1;    

    m_sd2d = sd2d;
    m_sd2d->setFreqAnalysisPointer(this);
    m_sd2d->addDisplayWidgets(this);

    m_nt = sd2d->getSamplesPerTraces();
    m_dt = sd2d->getTimeSampleRateInUs()/1000000.0;

    m_nf = int(m_nt/2)+1;
    m_maxfreq = 0.5/m_dt;
    m_df = m_maxfreq/(m_nf-1.0);

    m_filterType = 1;
    m_flowcut = 0;
    m_flowpass = 10;
    m_fhighpass = m_maxfreq-10;
    m_fhighcut = m_maxfreq;


    m_fftdf  = new float [m_nt];
    m_fftdc  = fftwf_alloc_complex(m_nf);
    m_fftwfp = fftwf_plan_dft_r2c_1d(m_nt, m_fftdf, m_fftdc, FFTW_DESTROY_INPUT|FFTW_ESTIMATE);
    m_fftwbp = fftwf_plan_dft_c2r_1d(m_nt, m_fftdc, m_fftdf,  FFTW_DESTROY_INPUT|FFTW_ESTIMATE);

    m_curveIdx = 0;

    m_fftam.resize(m_nf);
    m_fftph.resize(m_nf);
    m_fftre.resize(m_nf);
    m_fftim.resize(m_nf);
    m_fftan.resize(m_nf);
    m_fftam2.resize(m_nf);
    m_fftph2.resize(m_nf);
    m_fftre2.resize(m_nf);
    m_fftim2.resize(m_nf);
    m_fftan2.resize(m_nf);
    m_freqVal.resize(m_nf);
    for(int i=0; i<m_nf; i++) m_freqVal[i] = i*m_df;

    m_customPlot = new QCustomPlot();
    m_customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    m_customPlot->plotLayout()->insertRow(0);
    m_customPlot->plotLayout()->addElement(0, 0, new QCPTextElement(m_customPlot, "Amplitude Spectrum", 18));

    m_customPlot->xAxis->setLabel("Frequency(Hz)");    
    m_customPlot->xAxis->setRange(0, m_maxfreq);

    QPen myPen;
    myPen.setColor(QColor(0, 0, 255));
    myPen.setWidthF(2);
    m_customPlot->xAxis2->setVisible(true);
    m_customPlot->xAxis2->setTickLabels(false);
    m_customPlot->yAxis2->setVisible(true);
    m_customPlot->yAxis2->setTickLabels(false);
    m_customPlot->addGraph();
    m_customPlot->graph(0)->setPen(myPen);

    myPen.setColor(QColor(230, 128, 255));
    myPen.setStyle(Qt::DashLine);    
    m_customPlot->addGraph();
    m_customPlot->graph(1)->setPen(myPen);
    m_customPlot->graph(1)->setVisible(false);
    m_customPlot->graph(1)->setName("After filtering");
    m_customPlot->graph(1)->removeFromLegend();

    setWidget(m_customPlot);
    setVisible(visible);

    m_mitemNormalize = new QAction("Normalize amplitude", this);
    m_mitemNormalize->setCheckable(true);
    m_mitemNormalize->setChecked(false);

    m_mitemShowAm = new QAction("Show Amplitude", this);
    m_mitemShowAm->setCheckable(true);
    m_mitemShowAm->setChecked(true);

    m_mitemShowPh = new QAction("Show Phase", this);
    m_mitemShowPh->setCheckable(true);
    m_mitemShowPh->setChecked(false);

    m_mitemShowRe = new QAction("Show Real", this);
    m_mitemShowRe->setCheckable(true);
    m_mitemShowRe->setChecked(false);

    m_mitemShowIm = new QAction("Show Image", this);
    m_mitemShowIm->setCheckable(true);
    m_mitemShowIm->setChecked(false);

    QActionGroup* showGroup = new QActionGroup(this);
    showGroup->addAction(m_mitemShowAm);
    showGroup->addAction(m_mitemShowPh);
    showGroup->addAction(m_mitemShowRe);
    showGroup->addAction(m_mitemShowIm);

    m_myMenu = new QMenu(this);
    m_myMenu->addAction(m_mitemNormalize);
    m_myMenu->addSeparator();
    m_myMenu->addAction(m_mitemShowAm);
    m_myMenu->addAction(m_mitemShowPh);
    m_myMenu->addAction(m_mitemShowRe);
    m_myMenu->addAction(m_mitemShowIm);
    m_myMenu->addSeparator();

    connect(m_customPlot, &QCustomPlot::customContextMenuRequested, this, &Sdp2dFrequencyAnalysisDock::on_freqAna_customContextMenuRequested);
    connect(m_customPlot, &QCustomPlot::mouseMove, this, &Sdp2dFrequencyAnalysisDock::onMouseMove);
    connect(m_mitemNormalize, &QAction::toggled, this, &Sdp2dFrequencyAnalysisDock::setNormalizeAmplitude);
    connect(m_mitemShowAm, &QAction::toggled, this, &Sdp2dFrequencyAnalysisDock::setShowAmplitude);
    connect(m_mitemShowPh, &QAction::toggled, this, &Sdp2dFrequencyAnalysisDock::setShowPhase);
    connect(m_mitemShowRe, &QAction::toggled, this, &Sdp2dFrequencyAnalysisDock::setShowReal);
    connect(m_mitemShowIm, &QAction::toggled, this, &Sdp2dFrequencyAnalysisDock::setShowImage);
}

Sdp2dFrequencyAnalysisDock::~Sdp2dFrequencyAnalysisDock()
{
    fftwf_destroy_plan(m_fftwfp);
    fftwf_destroy_plan(m_fftwbp);
    fftwf_free(m_fftdc);
    delete [] m_fftdf;
    m_fftam.clear();
    m_fftan.clear();
    m_fftph.clear();
    m_fftre.clear();
    m_fftim.clear();
    m_fftam2.clear();
    m_fftan2.clear();
    m_fftph2.clear();
    m_fftre2.clear();
    m_fftim2.clear();

}

void Sdp2dFrequencyAnalysisDock::closeEvent(QCloseEvent *event)
{
    m_isHide = true;
    QDockWidget::closeEvent(event);
}


void Sdp2dFrequencyAnalysisDock::showEvent(QShowEvent *event)
{
    m_isHide = false;
    QDockWidget::showEvent(event);
}


void Sdp2dFrequencyAnalysisDock::onMouseMove(QMouseEvent *event)
{
    double x, y, key, val;
    x = event->pos().x();
    y = event->pos().y();

    m_customPlot->graph(0)->pixelsToCoords(x, y, key, val);
    if(key < m_customPlot->xAxis->range().lower || key > m_customPlot->xAxis->range().upper) return;
    if(val < m_customPlot->yAxis->range().lower || val > m_customPlot->yAxis->range().upper) return;
    m_statusbar->showMessage(QString("Frequency: %1,  Value: %2.").arg(key).arg(val));
    //cout << " x="<< x << " y=" << y << " key=" << key << " val=" << val << endl;
}

void Sdp2dFrequencyAnalysisDock::setGraphyNameWithIdx(QString legendName, int idx)
{
    m_customPlot->graph(idx)->setName(legendName);
    m_customPlot->replot();
}

void Sdp2dFrequencyAnalysisDock::calculateAndShowSpectrum(int ntraces, float** data, QString& label)
{
    m_fftam.fill(0);
    m_fftph.fill(0);
    m_fftre.fill(0);
    m_fftim.fill(0);
    m_fftan.fill(0);
    complex<float> value;

    for(int ix=0; ix< ntraces; ix++){
        memcpy((void*)m_fftdf, (void*)data[ix], sizeof(float)*m_nt);
        memset((void*)m_fftdc, 0, sizeof(fftwf_complex)*m_nf);
        fftwf_execute(m_fftwfp);
        for(int ifreq=0; ifreq<m_nf; ifreq++){
            value = complex<float>(m_fftdc[ifreq][0], m_fftdc[ifreq][1]);
            m_fftre[ifreq] += real(value);
            m_fftim[ifreq] += imag(value);
            m_fftam[ifreq] += abs(value);
            m_fftph[ifreq] += arg(value);
        }
    }
    //cout << "calculated FFT" << endl;
    m_ammax = -std::numeric_limits<float>::max();
    m_phmax = -std::numeric_limits<float>::max();
    m_remax = -std::numeric_limits<float>::max();
    m_immax = -std::numeric_limits<float>::max();

    m_ammin = std::numeric_limits<float>::max();
    m_phmin = std::numeric_limits<float>::max();
    m_remin = std::numeric_limits<float>::max();
    m_immin = std::numeric_limits<float>::max();

    for(int ifreq=0; ifreq<m_nf; ifreq++){
        if(m_fftre[ifreq] > m_remax) m_remax = m_fftre[ifreq];
        if(m_fftre[ifreq] < m_remin) m_remin = m_fftre[ifreq];
        if(m_fftim[ifreq] > m_immax) m_immax = m_fftim[ifreq];
        if(m_fftim[ifreq] < m_immin) m_immin = m_fftim[ifreq];
        if(m_fftam[ifreq] > m_ammax) m_ammax = m_fftam[ifreq];
        if(m_fftam[ifreq] < m_ammin) m_ammin = m_fftam[ifreq];
        if(m_fftph[ifreq] > m_phmax) m_phmax = m_fftph[ifreq];
        if(m_fftph[ifreq] < m_phmin) m_phmin = m_fftph[ifreq];
    }
    for(int ifreq=0; ifreq<m_nf; ifreq++){
        m_fftan[ifreq] = m_fftam[ifreq]/m_ammax;
    }

    showFrequencyCurve(label);
    raise();
}

void Sdp2dFrequencyAnalysisDock::showFrequencyCurve()
{
    QString label = m_customPlot->graph(m_curveIdx)->name();
    showFrequencyCurve(label);
}

void Sdp2dFrequencyAnalysisDock::showFrequencyCurve(QString& label)
{
    if(m_noCurveType == 0){
        QMessageBox message(QMessageBox::NoIcon,
                            "Warn", QString("Please select curve type for the Frequency Analysis"), QMessageBox::Ok, NULL);
        message.exec();
        return;
    }
    QCPTextElement* textItem = dynamic_cast<QCPTextElement*>(m_customPlot->plotLayout()->element(0, 0));
    m_customPlot->graph(m_curveIdx)->data().clear();

    if(m_curveType == FrequencyCurveType::Amplitude){
        textItem->setText("Amplitude Spectrum");
        if(m_normalizeAmp){
            m_customPlot->yAxis->setLabel("Amplitude(Noemalized)");
            m_customPlot->yAxis->setRange(0, 1.2);
            m_customPlot->graph(m_curveIdx)->setData(m_freqVal, m_fftan);
        } else {
            m_customPlot->yAxis->setLabel("Amplitude(Absolute)");
            m_customPlot->yAxis->setRange(0, m_ammax*1.2);
            m_customPlot->graph(m_curveIdx)->setData(m_freqVal, m_fftam);
        }

    }else if(m_curveType == FrequencyCurveType::Phase){
        textItem->setText("Phase of the Selected Seismic Data");
        m_customPlot->yAxis->setLabel("Phase value");
        m_customPlot->yAxis->setRange(m_phmin*1.2, m_phmax*1.2);
        m_customPlot->graph(m_curveIdx)->setData(m_freqVal, m_fftph);
    }else if(m_curveType == FrequencyCurveType::Real){
        textItem->setText("Real Part");
        m_customPlot->yAxis->setLabel("Real value");
        m_customPlot->yAxis->setRange(m_remin*1.2, m_remax*1.2);
        m_customPlot->graph(m_curveIdx)->setData(m_freqVal, m_fftre);
    }else if(m_curveType == FrequencyCurveType::Image){
        textItem->setText("Imaginary Part");
        m_customPlot->yAxis->setLabel("Imaginary value");
        m_customPlot->yAxis->setRange(m_immin*1.2, m_immax*1.2);
        m_customPlot->graph(m_curveIdx)->setData(m_freqVal, m_fftim);
    }
    m_customPlot->legend->setVisible(true);
    m_customPlot->graph(m_curveIdx)->setName(label);
    m_customPlot->graph(m_curveIdx)->rescaleAxes(true);

    showFiltedFrequencyCurve();

    m_customPlot->replot();
    this->show();
}

void Sdp2dFrequencyAnalysisDock::showFiltedFrequencyCurve(void)
{
    if(!m_hasComparison ){
        m_customPlot->graph(1)->removeFromLegend();
        m_customPlot->graph(1)->setVisible(false);
        return;
    }

    if(m_curveType == FrequencyCurveType::Amplitude){
        if(m_normalizeAmp){
            m_customPlot->graph(1)->setData(m_freqVal, m_fftan2);
        } else {
            m_customPlot->graph(1)->setData(m_freqVal, m_fftam2);
        }
    }else if(m_curveType == FrequencyCurveType::Phase){
        m_customPlot->graph(1)->setData(m_freqVal, m_fftph2);
    }else if(m_curveType == FrequencyCurveType::Real){
        m_customPlot->graph(1)->setData(m_freqVal, m_fftre2);
    }else if(m_curveType == FrequencyCurveType::Image){
        m_customPlot->graph(1)->setData(m_freqVal, m_fftim2);
    }
    m_customPlot->graph(1)->addToLegend();
    m_customPlot->graph(1)->setVisible(true);
}


void Sdp2dFrequencyAnalysisDock::on_freqAna_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    m_myMenu->exec(globalPos);
}


void Sdp2dFrequencyAnalysisDock::setNormalizeAmplitude(bool checked)
{
    if(checked){
        m_normalizeAmp = true;        
    }else{
        m_normalizeAmp = false;
    }
    showFrequencyCurve();
}

void Sdp2dFrequencyAnalysisDock::setShowAmplitude(bool checked)
{
    if(checked){
        m_curveType = FrequencyCurveType::Amplitude;
        m_mitemNormalize->setEnabled(true);
        m_noCurveType += 1;
        showFrequencyCurve();
    } else {
        m_noCurveType -= 1;
    }
    //cout << "setShowAmplitude=" << checked << " m_noCurveType=" << m_noCurveType<< endl;
}

void Sdp2dFrequencyAnalysisDock::setShowPhase(bool checked)
{    
    if(checked){
        m_curveType = FrequencyCurveType::Phase;
        m_mitemNormalize->setEnabled(false);
        m_noCurveType += 1;
        showFrequencyCurve();
    }else {
        m_noCurveType -= 1;
    }
    //cout << "setShowPhase=" << checked << " m_noCurveType=" << m_noCurveType<< endl;
}

void Sdp2dFrequencyAnalysisDock::setShowReal(bool checked)
{    
    if(checked){
        m_curveType = FrequencyCurveType::Real;
        m_mitemNormalize->setEnabled(false);
        m_noCurveType += 1;
        showFrequencyCurve();
    }else {
        m_noCurveType -= 1;
    }
    //cout << "setShowReal=" << checked << " m_noCurveType=" << m_noCurveType<< endl;
}

void Sdp2dFrequencyAnalysisDock::setShowImage(bool checked)
{    
    if(checked){
        m_curveType = FrequencyCurveType::Image;
        m_mitemNormalize->setEnabled(false);
        m_noCurveType += 1;
        showFrequencyCurve();
    }else {
        m_noCurveType -= 1;
    }
    //cout << "setShowImage=" << checked << " m_noCurveType=" << m_noCurveType<< endl;
}

void Sdp2dFrequencyAnalysisDock::applyBandPassFilter(float** data, int ntraces, bool applyFilter)
{
    m_fftam2.fill(0);
    m_fftph2.fill(0);
    m_fftre2.fill(0);
    m_fftim2.fill(0);
    m_fftan2.fill(0);
    complex<float> value;

    int filterType = m_filterType;
    float flowcut   = m_flowcut;
    float flowpass  = m_flowpass;
    float fhighpass = m_fhighpass;
    float fhighcut  = m_fhighcut;

    if(!applyFilter){
        flowcut = 0;
        flowpass = 0;
        fhighpass = m_maxfreq;
        fhighcut = m_maxfreq;
    }

    int f1 = int(flowcut   / m_df + 0.5);
    int f2 = int(flowpass  / m_df + 0.5);
    int f3 = int(fhighpass / m_df + 0.5);
    int f4 = int(fhighcut  / m_df + 0.5);

    if(filterType != 1){
        f1 = int(flowpass  / m_df + 0.5);
        f2 = int(flowcut   / m_df + 0.5);
        f3 = int(fhighcut  / m_df + 0.5);
        f4 = int(fhighpass / m_df + 0.5);
    }
    if(f3 > m_nf) f3=m_nf;
    if(f4 > m_nf) f4=m_nf;
    //cout << "m_df="<< m_df <<" m_nf=" << m_nf << " f1=" << f1 << " f2="<< f2 << " f3=" << f3 << " f4=" << f4 << endl;
    float flft = f2 - f1;
    float frht = f4 - f3;
    if(f2 != f1) flft = 1.0/flft;
    if(f3 != f4) frht = 1.0/frht;

    float ammax = -std::numeric_limits<float>::max();

    for(int ix=0; ix< ntraces; ix++){
        memcpy((void*)m_fftdf, (void*)data[ix], sizeof(float)*m_nt);
        memset((void*)m_fftdc, 0, sizeof(fftwf_complex)*m_nf);

        fftwf_execute(m_fftwfp);

        if(filterType == 1){
            for(int ifreq=0; ifreq<f1; ifreq++){
                m_fftdc[ifreq][0] = 0;
                m_fftdc[ifreq][1] = 0;
            }

            for(int ifreq=f1; ifreq<f2; ifreq++){
                float factor = (ifreq - f1)*flft;
                m_fftdc[ifreq][0] *= factor;
                m_fftdc[ifreq][1] *= factor;
            }

            for(int ifreq=f3; ifreq<f4; ifreq++){
                float factor = (f4 - ifreq)*frht;
                m_fftdc[ifreq][0] *= factor;
                m_fftdc[ifreq][1] *= factor;
            }

            for(int ifreq=f4; ifreq<m_nf; ifreq++){
                m_fftdc[ifreq][0] = 0;
                m_fftdc[ifreq][1] = 0;
            }
        } else {
            for(int ifreq=f1; ifreq<f2; ifreq++){
                float factor = (f2 - ifreq)*flft;
                m_fftdc[ifreq][0] *= factor;
                m_fftdc[ifreq][1] *= factor;
            }

            for(int ifreq=f2; ifreq<f3; ifreq++){
                m_fftdc[ifreq][0] = 0.0;
                m_fftdc[ifreq][1] = 0.0;
            }

            for(int ifreq=f3; ifreq<f4; ifreq++){
                float factor = (ifreq - f3)*frht;
                m_fftdc[ifreq][0] *= factor;
                m_fftdc[ifreq][1] *= factor;
            }
        }
        for(int ifreq=0; ifreq<m_nf; ifreq++){
            value = complex<float>(m_fftdc[ifreq][0], m_fftdc[ifreq][1]);
            m_fftre2[ifreq] += real(value);
            m_fftim2[ifreq] += imag(value);
            m_fftam2[ifreq] += abs(value);
            m_fftph2[ifreq] += arg(value);
            if(m_fftam2[ifreq] > ammax) ammax = m_fftam2[ifreq];
        }

        fftwf_execute(m_fftwbp);
        float factor = 1.0/float(m_nt);
        for(int it=0; it<m_nt; it++) m_fftdf[it] *= factor;

        memcpy((void*)data[ix], (void*)m_fftdf, sizeof(float)*m_nt);
    }

    for(int ifreq=0; ifreq<m_nf; ifreq++){
        m_fftan2[ifreq] = m_fftam2[ifreq]/ammax;
    }

    m_hasComparison = true;

    if(isVisible()){
        showFiltedFrequencyCurve();
        m_customPlot->replot();   
    }
}

void Sdp2dFrequencyAnalysisDock::setParametersToDom(Sdp2dQDomDocument* domDoc)
{
    QString filtTypeStr = QString("BandPass");
    if(m_filterType == 2){
        filtTypeStr = QString("BandReject");
    }
    domDoc->setOptionValue("FilterType", filtTypeStr);
    domDoc->setParameterInGroup(QString("flowcut"), QString::number(m_flowcut), filtTypeStr);
    domDoc->setParameterInGroup(QString("flowpass"), QString::number(m_flowpass), filtTypeStr);
    domDoc->setParameterInGroup(QString("fhighpass"), QString::number(m_fhighpass), filtTypeStr);
    domDoc->setParameterInGroup(QString("fhighcut"), QString::number(m_fhighcut), filtTypeStr);
}

bool Sdp2dFrequencyAnalysisDock::getParametersFromDom(Sdp2dQDomDocument* domVal)
{
    QString filtTypeStr = domVal->getOptionValue("FilterType");
    m_filterType = 1;
    if(filtTypeStr.compare("BandPass") != 0) m_filterType = 2;

    m_flowcut = domVal->getParameterInOption(QString("flowcut"), filtTypeStr).toFloat();
    m_flowpass = domVal->getParameterInOption(QString("flowpass"), filtTypeStr).toFloat();
    m_fhighpass = domVal->getParameterInOption(QString("fhighpass"), filtTypeStr).toFloat();
    m_fhighcut = domVal->getParameterInOption(QString("fhighcut"), filtTypeStr).toFloat();

    return true;
}
