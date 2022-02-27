#include "seismicdataprocessing2d.h"
#include "seismicdata2d.h"
#include "sdp2dMapDiaplayDock.h"
#include "sdp2dGatherDisplayArea.h"
#include "sdp2dMainGatherDisplayArea.h"
#include "sdp2dProcessedGatherDisplayArea.h"
#include "sdp2dDisplayParamTab.h"
#include "sdp2dFrequencyAnalysisDock.h"
#include "sdp2dQDomDocument.h"
#include "sdp2dPreStackMutePicks.h"
#include "sdp2dProcessJobDock.h"
#include "sdp2dUtils.h"

#include <QtWidgets>
#include <QObject>
#include <QtConcurrent>

#include <iostream>
#include <cmath>
#include <cfloat>
#include <functional>

using namespace std;

bool compareVAL(value_idx a, value_idx b)
{
    return (a.val < b.val);
}


/**
 * default constructor
 */
SeismicData2D::SeismicData2D() = default;


/**
 * overload constructor
 */
SeismicData2D::SeismicData2D(const string& segy_file_name, SeismicDataProcessing2D* mainwin, bool loadSEGYHeader) :
    QObject(), m_mainWindow(mainwin)
{
    m_idxPos = 0;
    m_sbar = mainwin->getStatusbarPointer();
    m_datatype = SeismicDataType::Stack;
    m_sgy = new Sdp2dSegy(segy_file_name, m_sbar);
    m_ntr = m_sgy->getNumberOfTraces();
    m_ns = m_sgy->getSamplesPerTrace();
    m_dtus = m_sgy->getTimeSampleRateInUs();

    m_discardBadTraces = false;
    m_outputApplyTopMute = false;
    m_outputApplyBtmMute = false;
    m_outputOrder = 0;

    m_mapDock = nullptr;
    m_inGthDisplay = nullptr;
    m_outGthDisplay = nullptr;
    m_displayParaTab = nullptr;    
    m_focusedDisplay =  nullptr;
    m_fanaDock = nullptr;


    m_jobdockVisible = false;
    m_processFunction = "None";
    m_processWholeData = false;
/*
    m_displayTraceHeader = false;
    m_plotTitle  = QString("Common Shot Gather 1");
    m_plotXLabel = QString("Trace Index");
    m_plotYLabel = QString("Time(s)");
    m_plotClipPercent = 99;
    m_plotGathereType = DisplayGatherType::CommonShot;
    m_plotDisplayType = SeismicDisplayTpye::Color;
    m_plotColormapIdx = 0;
    m_plotSymRange = 1;
    m_plotMaxTime = (m_ns-1)*m_dtus/1000000.0;
    m_plotGroupIndex = 1;
    m_plotGroupStep = 1;
    m_plotxscale = 3;
    m_plotyscale = 100;
    m_plotWiggleScale = 3;
    m_plotXAxisType = 1;
    m_plotRevPolarity = 0;
    m_zoomfit = true;
*/
    m_freqanaType = 0;
    m_interactiveFunction = 0;
    m_selectedRect = QRect();

    m_dcdp = 0;
    m_doff = 0;
    m_minoff = 0;
    m_maxoff = 0;

    m_slope = 0;
    m_angle = 0;
    m_intercept = 0;

    m_pcdp = new gathers(2);

    m_cdpid  = new int [m_ntr];
    m_cdpx   = new float [m_ntr];
    m_cdpy   = new float [m_ntr];

    m_trflag  = new short [m_ntr];
    m_topMute = new short [m_ntr];
    m_btmMute = new short [m_ntr];

    m_muteType = MuteType::TopMute;
    for(int i=0; i< m_ntr; i++){
        m_trflag[i] = SeismicTraceFlag::LiveTrace;
        m_topMute[i] = 0;
        m_btmMute[i] = 0;
    }

    createTheIndexFileName(segy_file_name);

    if(isIndexFileExist() == false && loadSEGYHeader) {
        loadSegyTracesHeader();
    }
}

/**
 * destructor
 */
SeismicData2D::~SeismicData2D()
{
    if (m_sgy != nullptr) delete m_sgy;

    if (m_pcdp!= nullptr) delete m_pcdp;

    if (m_cdpid != nullptr) delete [] m_cdpid;
    if (m_cdpx  != nullptr) delete [] m_cdpx;
    if (m_cdpy  != nullptr) delete [] m_cdpy;

    if (m_trflag  != nullptr) delete [] m_trflag;
    if (m_topMute != nullptr) delete [] m_topMute;
    if (m_btmMute != nullptr) delete [] m_btmMute;

    cout << "~SeismicData2D widgetList.count() = " << widgetList.count() << endl;
    for(int i=0; i< widgetList.count(); i++){
        widgetList[i]->close();
    }
    widgetList.clear();

    if(m_fanaDock != nullptr) m_fanaDock->close();
    if(m_inGthDisplay != nullptr) m_inGthDisplay->close();
}


void SeismicData2D::createTheIndexFileName(const string& sgy_file)
{
   size_t i = sgy_file.rfind('.', sgy_file.length());
   size_t j = sgy_file.rfind('/', sgy_file.length());
   m_idxfile = sgy_file.substr(0, j+1)+'.'+sgy_file.substr(j+1, i-j-1)+".idx";
   //cout << "sgy_file=" << sgy_file << endl;
   //cout << "m_idxfile=" << m_idxfile << endl;
}

void SeismicData2D::outputSegyWithLocalFormat(void)
{
    cout << "In SeismicData2D::outputSegyWithLocalFormat" << endl;
}

void SeismicData2D::loadSegyTracesHeader()
{
    const int traces = m_ntr;
    //cout << "m_ntr="<< m_ntr << endl;
    QVector<int> seq;
    for (int i = 0; i < traces; ++i) seq.append(i);

    QProgressDialog dialog;
    dialog.setLabelText(QString("Load trace headers..."));

    QThreadPool::globalInstance()->setMaxThreadCount(1);

    // Create a QFutureWatcher and connect signals and slots.
    QFutureWatcher<void> futureWatcher;
    QObject::connect(&futureWatcher, &QFutureWatcher<void>::finished, &dialog, &QProgressDialog::reset);
    QObject::connect(&dialog, &QProgressDialog::canceled, &futureWatcher, &QFutureWatcher<void>::cancel);
    QObject::connect(&futureWatcher,  &QFutureWatcher<void>::progressRangeChanged, &dialog, &QProgressDialog::setRange);
    QObject::connect(&futureWatcher, &QFutureWatcher<void>::progressValueChanged,  &dialog, &QProgressDialog::setValue);

    static Sdp2dSegy* s = m_sgy;

    std::function<void(int&)> load = [](int &itr) {
        s->getTraceHeader(itr);
    };

    auto future = QtConcurrent::map(seq, load);
    futureWatcher.setFuture(future);

    // Display the dialog and start the event loop.
    dialog.exec();

    futureWatcher.waitForFinished();

    //TraceHeaders** tHeader = m_sgy->getAllTracesHeader();
    //for(int i=0; i<m_ntr; i++ ){
    //    cout << "i="<< i << " ep="<< tHeader[i]->energy_source_point_number << ", stn=" << tHeader[i]->cdp_ensemble_number << endl;
    //}
}

/*
void SeismicData2D::findLineEndPoints(void)
{
    if(m_pcdp->firstGroupXY.x() < m_pcdp->firstGroupXY.x()){
        m_firstXY = m_pcdp->firstGroupXY;
    }else{
        m_firstXY = m_pcdp->firstGroupXY;
    }

    if(m_pcdp->lastGroupXY.x() > m_pcdp->lastGroupXY.x()){
        m_lastXY = m_pcdp->lastGroupXY;
    }else{
        m_lastXY = m_pcdp->lastGroupXY;
    }
}
*/
/*
void SeismicData2D::estimate2DAcquisitionGeometry(void)
{

    estimateSpacing(m_cdpx, m_cdpy, m_pcdp, m_pcdp->maxGroupSpace);

    createDataSummaryInfo();

    cout << "First CDP at  : (" << m_pcdp->firstGroupXY.x() << ", " << m_pcdp->firstGroupXY.y() <<")" <<endl;
    cout << "Last  CDP at  : (" << m_pcdp->lastGroupXY.x()  << ", " << m_pcdp->lastGroupXY.y() <<")" <<endl;


}
*/
void SeismicData2D::calculateSlopeOf2DLine(void)
{
    float mincdpy = 999999999.;
    float maxcdpy = 0.;
    double a, b;
    double xsum=0,x2sum=0,ysum=0,xysum=0;         //variables for sums/sigma of xi,yi,xi^2,xiyi etc
    for (int i=0; i<m_ntr; i++)
    {
        xsum=xsum+m_cdpx[i];                        //calculate sigma(xi)
        ysum=ysum+m_cdpy[i];                        //calculate sigma(yi)
        x2sum=x2sum+pow(m_cdpx[i],2);               //calculate sigma(x^2i)
        xysum=xysum+m_cdpx[i]*m_cdpy[i];              //calculate sigma(xi*yi)
        if(m_cdpy[i] < mincdpy) mincdpy = m_cdpy[i];
        if(m_cdpy[i] > maxcdpy) maxcdpy = m_cdpy[i];
    }
    a =(m_ntr*xysum-xsum*ysum)/(m_ntr*x2sum-xsum*xsum); //calculate slope
    b =(x2sum*ysum-xsum*xysum)/(x2sum*m_ntr-xsum*xsum);            //calculate intercept
    m_angle = atan(a);
    m_slope = a;
    m_intercept = b;
    cout << "slope="<< m_slope << ", intercept=" << m_intercept << ", angle=" << m_angle << endl;
}


void SeismicData2D::writeOneGroup2Disk(ofstream& outfile, gathers *p, int ig)
{
    float* data = new float [m_ns];
    size_t dlen = m_ns * sizeof(float);

    //for(vector<subset_info>::iterator it=p->group[ig].subset.begin(); it!=p->group[ig].subset.end(); ++it){
    int ntraces= p->group[ig].traceIdx.size();
    for(int i=0; i < ntraces; i++){
        int tid = p->group[ig].traceIdx[i];
        TraceHeaders* thdr = m_sgy->getTraceHeader(tid);
        m_sgy->getTraceDataFromIntermFile(tid, data);
        outfile.write((char*)thdr, TRACE_HEADER_SIZE);
        outfile.write((char*)data, dlen);
        //cout << " tid:" << tid << ", group=" << p->group[ig].groupValue1 << ", subset=" << it->subsetValue << endl;
        //cout << "shot station: "<< thdr->energy_source_point_number << ", receiver station: "<< thdr->cdp_ensemble_number << endl;
    }

    delete [] data;
}



QStringList& SeismicData2D::getDataSummary(void)
{
    return m_datasum;
}

void SeismicData2D::setCDPSpacing(float value)
{
    m_dcdp = value;
}

void SeismicData2D::setOffsetSpacing(float value)
{
    m_doff = value;
}

void SeismicData2D::setMinOffset(float value)
{
    m_minoff  = value;
}

void SeismicData2D::setMaxOffset(float value)
{
    m_maxoff  = value;
}

std::string& SeismicData2D::getSEGYFileName()
{
    return m_sgy->getSEGYFileName();
}

void SeismicData2D::setRegenerateIndexFile(bool regenerate)
{
    if(regenerate){
        QFile idxf(QString::fromUtf8(m_idxfile.data(), m_idxfile.size()));
        if(idxf.exists()){
            idxf.remove();
        }
    }
}


bool SeismicData2D::isIndexFileExist()
{
    struct stat buffer;
    if(stat (m_idxfile.c_str(), &buffer)) {
        return false;
    } else{
        return true;
    }
}


void SeismicData2D::getAllCDPXY(QList<QPointF>& data)
{
    return m_sgy->getAllCDPXY(data);
}


void SeismicData2D::sortSubsetOfGathers(gathers *p, float* sortKey)
{
    std::vector<value_idx> valIdx;
    value_idx vi;
    for(int ig = 0; ig < p->ngroups; ig++){
        valIdx.clear();
        int ntraces = p->group[ig].traceIdx.size();

        for(int i=0; i< ntraces; i++){
            vi.idx = p->group[ig].traceIdx[i];
            vi.val = sortKey[vi.idx];
            valIdx.push_back(vi);
        }
        std::sort(valIdx.begin(), valIdx.end(), compareVAL);

        for(int i=0; i< ntraces; i++){
            p->group[ig].traceIdx[i] = valIdx[i].idx;
        }
    }
}

void SeismicData2D::buildIndexUsingOneCoordinate(float* x, int* id, gathers* gth)
{
    vector<value_idx> newx;
    value_idx vi;
    for(int i=0; i< m_ntr; i++){
        vi.val = x[i];
        vi.idx = i;
        newx.push_back(vi);
    }
    std::sort(newx.begin(), newx.end(), compareVAL);

    int tid = newx[0].idx;
    int idx = 1;
    id[tid] = idx;
    for(int i=1; i< m_ntr; i++){
        if(newx[i].val > newx[i-1].val) idx++;
        tid = newx[i].idx;
        id[tid] = idx;
    }
    gth->minGroupValue = 1;
    gth->maxGroupValue = idx;
    gth->ngroups = idx;
    newx.clear();
}

void SeismicData2D::collectTraceHeaderInfo(void)
{
    cout << " in SeismicData2D" << endl;
    TraceHeaders** tHeader = m_sgy->getAllTracesHeader();

    float xyscale = tHeader[0]->scalar_for_coordinates;
    if(xyscale < 0) xyscale = 1./fabs(xyscale);
    if(fabs(xyscale) < 0.0000001) xyscale=1;

    //float elscale = tHeader[0]->scalar_for_elevations_and_depths;
    //if(elscale < 0) elscale = 1./fabs(elscale);
    //if(fabs(elscale) < 0.000001) elscale=1;
    float sx, sy, rx, ry;

    for(int i=0; i < m_ntr; i++ ){
        sx = tHeader[i]->x_source_coordinate * xyscale;
        sy = tHeader[i]->y_source_coordinate * xyscale;
        rx = tHeader[i]->x_receiver_group_coordinate * xyscale;
        ry = tHeader[i]->y_receiver_group_coordinate * xyscale;
        m_cdpx[i] = (sx+rx)/2.0;
        m_cdpy[i] = (sy+ry)/2.0;
    }
}


void SeismicData2D::updateTraceHeaderWithEditedInfo()
{
    TraceHeaders** tHeader = m_sgy->getAllTracesHeader();
    for(int idx=0; idx < m_ntr; idx++ ){
        tHeader[idx]->data_use = m_trflag[idx];
        tHeader[idx]->mute_time_start = m_topMute[idx];
        tHeader[idx]->mute_time_end = m_btmMute[idx];
        //tHeader[idx]->distance_from_source_point_to_receiver_group = int(m_offset[idx]+0.5);
    }
}

void SeismicData2D::estimateSpacing(float *x, float *y, gathers *p, float uplimit)
{
    vector<value_idx> newx;
    float dis, vx, vy;
    float dsum = 0;
    int   dnum = 0;
    float minspacing = 99999.;
    float maxspacing = 0.;
    int i1, i2;
    value_idx vi;
    for(int i=0; i< m_ntr; i++){
        vi.val = x[i];
        vi.idx = i;
        newx.push_back(vi);
    }
    std::sort(newx.begin(), newx.end(), compareVAL);
    for(int i=1; i< m_ntr; i++){
        i1 = newx[i-1].idx;
        i2 = newx[i].idx;
        vx = fabsf(x[i2]-x[i1]);
        if(vx < FLT_EPSILON ) continue;
        vy = y[i2]-y[i1];
        dis = sqrtf(vx*vx + vy*vy);
        if( dis > 1 && dis < uplimit){
            dsum += dis;
            dnum++;
            if(dis < minspacing) minspacing = dis;
            if(dis > maxspacing) maxspacing = dis;
        }
    }
    if(dnum >1){
        p->aveGroupSpace = dsum/dnum;
        p->minGroupSpace = minspacing;
        p->maxGroupSpace = maxspacing;
        p->firstGroupXY = QPointF(x[newx[0].idx], y[newx[0].idx]);
        p->lastGroupXY  = QPointF(x[newx[m_ntr-1].idx], y[newx[m_ntr-1].idx]);
    }

    newx.clear();
}

void SeismicData2D::fillIndexStructure(void)
{
    cout << " SHOULD NOT come here. WRONG!!!! " << endl;
    // virtual function. Currently only implemented for the pre-stack data
    // for stack data and velocity data, need to be implenmented as to store the inline/crossline.
}

void SeismicData2D::collectGathersInfo(void)
{

}

void SeismicData2D::createDataSummaryInfo(bool moreflag)
{
    if(moreflag) cout << " in SeismicData2D::createDataSummaryInfo" << endl;
    m_datasum << QString("Number of traces: %1").arg(m_ntr);
    m_datasum << QString("Number of samples per trace: %1").arg(m_ns);
    m_datasum << QString("Sample rate in Micrometer: %1").arg(m_dtus);
}


void SeismicData2D::checkIndexStructure(gathers* p)
{
    cout << "ngroups = " << p->ngroups << endl;
    for(int ig = 0; ig < p->ngroups; ig++){
        cout << p->gather_type<< " group.x "<< p->group[ig].groupValue1 << " group.y "<< p->group[ig].groupValue2<< " has traces: " << p->group[ig].ntraces << endl;
        for(int is = 0; is < p->group[ig].ntraces; is++){
            cout << "     is="<<is<<"    traceIdx: "<< p->group[ig].traceIdx[is] <<endl;
        }
    }
}

void SeismicData2D::showAllDisplayWidgets()
{    
    for(int i=0; i< widgetList.count(); i++){
        if(widgetList[i] == m_fanaDock){
            if(m_fanaDock->isFrequencyAnaDockHide()) continue;
        }
        widgetList[i]->show();
    }
    m_focusedDisplay = m_inGthDisplay;
}

void SeismicData2D::hideAllDisplayWidgets()
{
    for(int i=0; i< widgetList.count(); i++){
        widgetList[i]->hide();
    }
}

void SeismicData2D::createFrequencyAnaDock(bool visible)
{
    if(m_fanaDock != nullptr) {
        m_fanaDock->setVisible(visible);
    } else {
        m_fanaDock = new Sdp2dFrequencyAnalysisDock(this, visible, m_mainWindow);
    }
}


bool SeismicData2D::isFrequencyAnaDockHide()
{
    return m_fanaDock->isFrequencyAnaDockHide();
}

void SeismicData2D::addDisplayWidgets(QWidget* widget)
{
    widgetList.append(widget);
}

void SeismicData2D::removeAdjunctiveDisplays(void)
{
    if(m_outGthDisplay != nullptr) {
        widgetList.takeAt(widgetList.indexOf(m_outGthDisplay));
        m_outGthDisplay->close();
        m_outGthDisplay = nullptr;
    }
}

void SeismicData2D::setInteractiveFunction(int val)
{
    m_interactiveFunction = val;
    //cout << "setInteractiveFunction = " << m_interactiveFunction << endl;
    if(m_interactiveFunction == InteractiveFunctions::FrequencyAnalysis){
        m_fanaDock->setVisible(true);
    } else{
        m_fanaDock->setVisible(false);
    }
}

void SeismicData2D::setInputGatherDisplayPointer(Sdp2dMainGatherDisplayArea* p)
{
    m_inGthDisplay = p;
    m_focusedDisplay = p;
}

void SeismicData2D::setDisplayParamTabPointer(Sdp2dDisplayParamTab* p)
{
    m_displayParaTab = p;    
}

void SeismicData2D::updateXLableForDisplayParaTab(QString text)
{    
    if(m_displayParaTab !=  nullptr) {
        m_displayParaTab->setPlotXLabel(text);
    }
    /*
    if(m_focusedDisplay !=  nullptr) {
        m_focusedDisplay->setPlotXLabel(text);
    }
    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setPlotXLabel(text);
    }*/
}

void SeismicData2D::updateYLableForDisplayParaTab(QString text)
{
    //m_plotYLabel = text;
    if(m_displayParaTab !=  nullptr){
       m_displayParaTab->setPlotYLabel(text);
    }
    /*
    if(m_focusedDisplay !=  nullptr) {
        m_focusedDisplay->setPlotYLabel(text);
    }

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setPlotYLabel(text);
    }
    */
}

void SeismicData2D::updatePlotTitleForDisplayParaTab(QString text)
{    
    if(m_displayParaTab ==  nullptr) return;
    m_displayParaTab->setPlotTitle(text);

    /*
    if(m_outGthDisplay != nullptr){
        QString title = text + QString(" : ") + m_processFunction;
        m_outGthDisplay->setPlotTitle(title);
    }
    */
}

bool SeismicData2D::getTraceHeaderDisplay(void)
{
    if(m_focusedDisplay ==  nullptr)  return false;
    return m_focusedDisplay->getTraceHeaderDisplay();
}

QString SeismicData2D::getPlotTitle()
{
    if(m_focusedDisplay ==  nullptr)  return QString("Common Shot Gather 1");
    return m_focusedDisplay->getPlotTitle();
}

QString SeismicData2D::getPlotXLabel()
{
    if(m_focusedDisplay ==  nullptr)  return QString("Trace Index");
    return m_focusedDisplay->getPlotXLabel();
}

QString SeismicData2D::getPlotYLabel()
{
    if(m_focusedDisplay ==  nullptr)  return QString("Time(s)");
    return m_focusedDisplay->getPlotYLabel();
}

int SeismicData2D::getDataClipPercentage()
{
    if(m_focusedDisplay ==  nullptr)  return 99;
    return m_focusedDisplay->getDataClipPercentage();
}

int SeismicData2D::getGatherType()
{
    if(m_focusedDisplay ==  nullptr)  return DisplayGatherType::CommonShot;
    return m_focusedDisplay->getGatherType();
}

int SeismicData2D::getDisplayType()
{
    if(m_focusedDisplay ==  nullptr)  return SeismicDisplayTpye::Color;
    return m_focusedDisplay->getDisplayType();
}

int SeismicData2D::getColorMapIndex()
{
    if(m_focusedDisplay ==  nullptr)  return 0;
    return m_focusedDisplay->getColorMapIndex();
}

float SeismicData2D::getMaxDisplayTime()
{
    if(m_focusedDisplay ==  nullptr)  return (m_ns-1)*m_dtus/1000000.0;
    return m_focusedDisplay->getMaxDisplayTime();
}

int SeismicData2D::getSymmetryRange(void)
{
    if(m_focusedDisplay ==  nullptr) return 1;
    return m_focusedDisplay->getSymmetryRange();
}

int SeismicData2D::getGroupStep(void)
{
    if(m_focusedDisplay ==  nullptr) return 1;
    return m_focusedDisplay->getGroupStep();
}

int SeismicData2D::getPlotGroupIndex(void)
{
    if(m_focusedDisplay ==  nullptr) return 1;
    return m_focusedDisplay->getGatherIndex();
}

float SeismicData2D::getPlotXScale(void)
{
    if(m_focusedDisplay ==  nullptr) return 3;
    return m_focusedDisplay->getPlotXScale();
}

float SeismicData2D::getPlotYScale(void)
{
    if(m_focusedDisplay ==  nullptr) return 100;
    return m_focusedDisplay->getPlotYScale();
}

bool SeismicData2D::getPlotFitZoom(void)
{
    if(m_focusedDisplay ==  nullptr) return true;
    return m_focusedDisplay->getPlotFitZoom();
}

float SeismicData2D::getPlotWiggleScale(void)
{
    if(m_focusedDisplay ==  nullptr) return 3;
    return m_focusedDisplay->getPlotWiggleScale();
}

int SeismicData2D::getXAxisType(void)
{
    if(m_focusedDisplay ==  nullptr) return 1;
    return m_focusedDisplay->getPlotXAxisType();
}

int SeismicData2D::getReversepolarity(void)
{
    if(m_focusedDisplay ==  nullptr) return 0;
    return m_focusedDisplay->getReversepolarity();
}

void SeismicData2D::setTraceHeaderDisplay(bool checked)
{    
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setTraceHeaderDisplay(checked);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setTraceHeaderDisplay(checked);
    }
}

void SeismicData2D::setPlotTitle(QString text)
{    
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setPlotTitle(text);
    if(m_outGthDisplay != nullptr){
        QString title = text + QString(" : ") + m_processFunction;
        m_outGthDisplay->setPlotTitle(title);
    }
}

void SeismicData2D::setPlotXLabel(QString text)
{
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setPlotXLabel(text);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setPlotXLabel(text);
    }
}

void SeismicData2D::setPlotYLabel(QString text)
{
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setPlotYLabel(text);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setPlotYLabel(text);
    }
}

void SeismicData2D::setDataClipPercentage(int value)
{   
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setDataClipPercentage(value);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setDataClipPercentage(value);
    }
}

void SeismicData2D::setGatherType(int gatherType, int minNtraces)
{
    if(m_focusedDisplay ==  nullptr) return;

    int gIdx = m_focusedDisplay->setGatherType(gatherType, minNtraces);

    m_displayParaTab->setGatherToPlot(gIdx);

    updateDisplays();
}

void SeismicData2D::setDisplayType(int displayType)
{
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setDisplayType(displayType);
    m_mainWindow->setToolBarDisplayType(displayType);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setDisplayType(displayType);
    }
}

void SeismicData2D::setColorMapIndex(int colormapIdx)
{
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setColorMapIndex(colormapIdx);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setColorMapIndex(colormapIdx);
    }
}

void SeismicData2D::setSymmetryRange(int symmetryRange)
{
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setSymmetryRange(symmetryRange);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setSymmetryRange(symmetryRange);
    }
}

void SeismicData2D::setGroupStep(int value)
{
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setGroupStep(value);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setGroupStep(value);
    }
}

void SeismicData2D::setMaxDisplayTime(float tlen)
{        
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setMaxDisplayTime(tlen);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setMaxDisplayTime(tlen);
    }
}

void SeismicData2D::setPlotXScale(float val)
{    
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setPlotXScale(val, m_plotEdge);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setPlotXScale(val, m_plotEdge);
    }
}


void SeismicData2D::setPlotYScale(float val)
{
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setPlotYScale(val, m_plotEdge);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setPlotYScale(val, m_plotEdge);
    }
}

void SeismicData2D::setPlotFitZoom(bool zoomfit)
{
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setPlotFitZoom(zoomfit);
}

void SeismicData2D::setPlotWiggleScale(float val)
{
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setPlotWiggleScale(val);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setPlotWiggleScale(val);
    }
}

void SeismicData2D::setDisplayScales(float xscale, float yscale, QPointF plotEdge)
{
    if(m_focusedDisplay ==  nullptr) return;

    //m_plotxscale = xscale;
    //m_plotyscale = yscale;
    m_plotEdge = plotEdge;

    //cout << "m_plotxscale=" << m_plotxscale << " m_plotyscale=" << m_plotyscale << endl;

    m_displayParaTab->setPlotXScale(xscale);
    m_displayParaTab->setPlotYScale(yscale);
}

bool SeismicData2D::setPlotGroupIndex(int index)
{    
    int backup = m_focusedDisplay->getGatherIndex();
    //cout << " index=" << index << " backup="<< backup << endl;
    if(m_focusedDisplay ==  nullptr) return false;

    if(!m_focusedDisplay->setPlotGroupIndex(index)){
        m_displayParaTab->setGatherToPlot(backup);
        return false;
    }else{
        m_displayParaTab->setGatherToPlot(index);
    }
    updateDisplays();

    return true;
}

void SeismicData2D::setXAxisType(int xaxisType)
{    
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setPlotXAxisType(xaxisType);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setPlotXAxisType(xaxisType);
    }
}

void SeismicData2D::setReversepolarity(int polarity)
{    
    if(m_focusedDisplay ==  nullptr) return;
    m_focusedDisplay->setReversepolarity(polarity);

    if(m_outGthDisplay != nullptr){
        m_outGthDisplay->setReversepolarity(polarity);
    }
}

void SeismicData2D::updateDisplayParameters(void)
{
    if(m_displayParaTab == nullptr || m_focusedDisplay == nullptr) return;    

    m_displayParaTab->setPlotTitle(m_focusedDisplay->getPlotTitle());
    m_displayParaTab->setPlotXLabel(m_focusedDisplay->getPlotXLabel());
    m_displayParaTab->setPlotYLabel(m_focusedDisplay->getPlotYLabel());
    m_displayParaTab->setDataClipPercentage(m_focusedDisplay->getDataClipPercentage());
    m_displayParaTab->setGatherType(m_focusedDisplay->getGatherType());
    m_displayParaTab->setDisplayType(m_focusedDisplay->getDisplayType());
    m_displayParaTab->setColorMapIndex(m_focusedDisplay->getColorMapIndex());
    m_displayParaTab->setMaxDisplayTime(m_focusedDisplay->getMaxDisplayTime());
    m_displayParaTab->setSymmetryRange(m_focusedDisplay->getSymmetryRange());
    m_displayParaTab->setGroupStep(m_focusedDisplay->getGroupStep());
    m_displayParaTab->setPlotXScale(m_focusedDisplay->getPlotXScale());
    m_displayParaTab->setPlotYScale(m_focusedDisplay->getPlotYScale());
    m_displayParaTab->setPlotWiggleScale(m_focusedDisplay->getPlotWiggleScale());
    m_displayParaTab->setXAxisTypex(m_focusedDisplay->getPlotXAxisType());
    m_displayParaTab->setGatherToPlot(m_focusedDisplay->getGatherIndex());
    m_displayParaTab->setReversepolarity(m_focusedDisplay->getReversepolarity());
}


int SeismicData2D::setPlotGroupToTheFirst(int minNtraces)
{
    if(m_focusedDisplay ==  nullptr) return 0;
    int plotGroupIndex = m_focusedDisplay->setPlotGroupToTheFirst(minNtraces);

    if(plotGroupIndex>0) updateDisplays();

    return plotGroupIndex;
}

int SeismicData2D::setPlotGroupToTheLast(int minNtraces)
{
    if(m_focusedDisplay ==  nullptr) return 0;
    int plotGroupIndex = m_focusedDisplay->setPlotGroupToTheLast(minNtraces);

    if(plotGroupIndex>0)  updateDisplays();

    return plotGroupIndex;
}

void SeismicData2D::cleanMainDisplay(bool funChanged)
{
    if(m_inGthDisplay != nullptr){
        m_inGthDisplay->cleanDisplay(funChanged);
    }
}

void SeismicData2D::getAveageAmplitude(double* amp)
{
    TraceHeaders** p = m_sgy->getAllTracesHeader();
    QPointF aveAmp, absAmp, rmsAmp;
    m_sgy->getRangeOfAmplitude(aveAmp, absAmp, rmsAmp);
    //cout << "aveAmp lower="<< aveAmp.x() << " upper=" << aveAmp.y() << endl;
    for(int i=0; i< m_ntr; i++){
        amp[i] = p[i]->ave_amplitude;
    }
}

void SeismicData2D::getAbsoluteAmplitude(double* amp)
{
    TraceHeaders** p = m_sgy->getAllTracesHeader();
    for(int i=0; i< m_ntr; i++){
        amp[i] = p[i]->abs_amplitude;
    }
}

void SeismicData2D::getRMSAmplitude(double* amp)
{
    TraceHeaders** p = m_sgy->getAllTracesHeader();
    for(int i=0; i< m_ntr; i++){
        amp[i] = p[i]->rms_amplitude;
    }
}

void SeismicData2D::setGatherDisplayWithHelightTrace(int gatherType, int gatherIdx, int traceIdx)
{    
    m_focusedDisplay->setGatherDisplayWithHelightTrace(gatherType, gatherIdx, traceIdx);
    updateMapAndFreqAnaWithGatherTrace(traceIdx);
}


void SeismicData2D::updateMapAndFreqAnaWithGatherTrace(int traceIdx)
{
    if(m_focusedDisplay ==  nullptr) return;    
    updateDisplayParameters();
    updateDisplays(traceIdx);
}

void SeismicData2D::updateDisplays(int traceIdx)
{
    if(m_focusedDisplay ==  nullptr) return;
    int plotGathereType = m_focusedDisplay->getGatherType();
    int plotGroupIndex  = m_focusedDisplay->getGatherIndex();
    //cout << "updateDisplays" << " gType=" <<plotGathereType << " gIdx=" << plotGroupIndex << endl;
    if(m_datatype == SeismicDataType::PreStack) m_mapDock->displayPreStackMap(plotGathereType, plotGroupIndex);

    if(m_outGthDisplay != nullptr){
        if(m_interactiveFunction != InteractiveFunctions::StackVelAnalysis){
            m_mainWindow->processCurrentGather(m_processFunction);
        }
    }

    if(m_fanaDock!= nullptr && m_fanaDock->isVisible()) {
        QString label;
        float** data = nullptr;
        int ntr = 1;
        if(m_freqanaType == FreqAnaType::WholeGather){
            label = QString("Gather %1").arg(plotGroupIndex);
            ntr = m_focusedDisplay->getNumTracesOfCurrentGather();
            m_selectedRect = QRect(QPoint(1, 0), QPoint(ntr, m_ns-1));
            data = Sdp2dUtils::alloc2float(m_ns, ntr);
            m_focusedDisplay->getDataOfInputSeismicGather(data);
        } else if(traceIdx > 0){
            label = QString("Gather %1, Trace %2").arg(plotGroupIndex).arg(traceIdx);
            m_selectedRect = QRect(QPoint(traceIdx, 0), QPoint(traceIdx, m_ns-1));
            data = Sdp2dUtils::alloc2float(m_ns, 1);
            float* trace = m_focusedDisplay->getDataOfASeismicTrace(traceIdx);
            memcpy((void*)data[0], (void*)trace, sizeof(float)*m_ns);
        }

        if(data != nullptr){
            frequencyAnalysis(m_selectedRect, data, label);
            Sdp2dUtils::free2float(data);
        }
    }
}


void SeismicData2D::frequencyAnalysis(int nx, int nt, float** data, QString& label)
{
     if(m_ns != nt){
         cout << "Number of sample of current data is "<< m_ns << ", BUT ns of the frequency analysis is " << nt<<". Somethinf is WRONG!!!" << endl;
         return;
     }

     createFrequencyAnaDock(true);

     int plotGathereType = m_focusedDisplay->getGatherType();

     QString newLabel;
     switch(plotGathereType){
     case DisplayGatherType::CommonShot:
         newLabel = QString("Shot ")+label;
         break;
     case DisplayGatherType::CommonDepthPoint:
         newLabel = QString("CDP ")+label;
         break;
     case DisplayGatherType::CommonOffset:
         newLabel = QString("Offset ")+label;
         break;
     case DisplayGatherType::CommonReceiver:
         newLabel = QString("Receiver ")+label;
         break;
     }

     m_fanaDock->calculateAndShowSpectrum(nx, data, newLabel);

}

void SeismicData2D::frequencyAnalysis(QRect selectRect, float** data, QString& label)
{
    m_selectedRect = selectRect;
    int x1 = selectRect.left() - 1;
    int x2 = selectRect.right() - 1;
    int nx = x2 - x1 +1;
    frequencyAnalysis(nx, m_ns, data, label);

    if(m_outGthDisplay != nullptr){
        float** data2 = Sdp2dUtils::alloc2float(m_ns, nx);
        //memset((void*)data2[0], 0, m_ns*nx*sizeof(float));
        getProcessedSeismicDataInRect(selectRect, data2);
        m_fanaDock->applyBandPassFilter(data2, nx, false);
        m_fanaDock->setGraphyNameWithIdx(QString("After ")+m_processFunction, 1);
        Sdp2dUtils::free2float(data2);
    }

}


void SeismicData2D::calculateTraceWeightValues(int top, int btm, float* weight, int ttap, int btap)
{
    int t1 = top - ttap;
    int t2 = top;
    int t3 = btm;
    int t4 = btm + btap;

    if(t1 < 0) t1 = 0;
    if(t4 > m_ns) t4 = m_ns;

    for(int it = 0; it < t1; it++) weight[it]= 0;
    float nt = t2 - t1;
    for(int it = t1; it < t2; it++) weight[it]= (it - t1 + 1.0)/nt;

    for(int it = t2; it < t3; it++) weight[it]= 1;

    nt = t4 - t3;
    for(int it = t3; it < t4; it++) weight[it]= (t4 - it)/nt;
    for(int it = t4; it < m_ns; it++) weight[it]= 0;

    //for(int i=0; i<m_ns; i++) cout << "i="<< i << " weight=" << weight[i] << endl;
}

void SeismicData2D::setFreqComparisonFlag(bool flag)
{
    m_fanaDock->setFreqComparisonFlag(flag);
}

void SeismicData2D::setTopMute()
{
    m_muteType = MuteType::TopMute;
    m_currentMute = m_tMute;
    m_focusedDisplay->setMutePicksDisplay();
    m_mainWindow->getProcessJobPointer()->interactiveFunctionEnabled(InteractiveFunctions::PickingMute);
}

void SeismicData2D::setBtmMute()
{
    m_muteType = MuteType::BottomMute;
    m_currentMute = m_bMute;
    m_focusedDisplay->setMutePicksDisplay();
    m_mainWindow->getProcessJobPointer()->interactiveFunctionEnabled(InteractiveFunctions::PickingMute);
}

int SeismicData2D::getMuteTaperLength(void)
{
    return m_currentMute->getMuteTaperLength();
}

void SeismicData2D::getSeismicDataInRect(QRect selectedRect, float** data, gathers* gp)
{
    int x1 = selectedRect.left();
    int x2 = selectedRect.right();
    int nx = x2 - x1 + 1;

    float* weight = new float [m_ns];
    calculateTraceWeightValues(selectedRect.top(), selectedRect.bottom(), weight);

    int plotGroupIndex  = m_focusedDisplay->getGatherIndex();

    int gidx = plotGroupIndex - gp->minGroupValue;
    for(int ix = 0; ix < nx; ix++){
        int idx = x1 + ix - 1;
        int tid = gp->group[gidx].traceIdx[idx];
        //cout << "ix=" << ix << " idx="<< idx << " tid="<< tid << " gidx=" << gidx << endl;
        m_sgy->getTraceDataFromIntermFile(tid, data[ix]);
        for(int it = 0; it < m_ns; it++){
            data[ix][it] =  data[ix][it] * weight[it];
        }
    }
    delete [] weight;

}

void SeismicData2D::getProcessedSeismicDataInRect(QRect selectedRect, float** data)
{
    int x1 = selectedRect.left();
    int x2 = selectedRect.right();
    int nx = x2 - x1 + 1;
    float* weight = new float [m_ns];
    calculateTraceWeightValues(selectedRect.top(), selectedRect.bottom(), weight);
    float** din = m_outGthDisplay->getDataOfInputSeismicGather();
    for(int ix = 0; ix < nx; ix++){
        int idx = x1 + ix - 1;
        for(int it = 0; it < m_ns; it++){
            data[ix][it] =  din[idx][it] * weight[it];
        }
    }
    delete [] weight;

}


void SeismicData2D::updateMuteParameters(Sdp2dQDomDocument* domVal)
{
    bool extrapolation = domVal->getParameterInGroup("Extrapolate").compare("False");
    //cout << "extrapolation = " << extrapolation << endl;
    bool interpolation = domVal->getParameterInGroup("Interpolate").compare("False");
    //cout << "interpolation = " << interpolation << endl;
    int taperLength = domVal->getParameterInGroup("taperLength").toInt();

    m_currentMute->setExtrapolateMute(extrapolation);
    m_currentMute->setInterpolateMute(interpolation);
    m_currentMute->setMuteTaperLength(taperLength);
}

void SeismicData2D::bandPassFilterSelectedDataAndDisplay(Sdp2dQDomDocument* domval, gathers* gp)
{
    if(!m_fanaDock->getParametersFromDom(domval)) return;

    QString filtTypeStr = QString("BandPass");
    int filterType = m_fanaDock->getFilterType();
    if(filterType ==2) filtTypeStr = QString("BandReject");

    //cout << "module name=" << domval->getModuleName().toStdString().c_str() << endl;
    //cout << "filterType=" << filtTypeStr.toStdString().c_str() << endl;
    //cout << "m_flowcut=" << m_fanaDock->getFreqLowCut() << endl;
    //cout << "m_flowpass=" << m_fanaDock->getFreqLowPass() << endl;
    //cout << "m_fhighpass=" << m_fanaDock->getFreqHighPass() << endl;
    //cout << "m_fhighcut=" << m_fanaDock->getFreqHighCut() << endl;

    QRect selectedRect = m_inGthDisplay->getSelectedRect();
    if(selectedRect.height() < 15) return;

    m_fanaDock->setFreqComparisonFlag(true);

    int x1 = selectedRect.left();
    int x2 = selectedRect.right();
    int nx = x2 - x1 + 1;

    float** data = Sdp2dUtils::alloc2float(m_ns, nx);
    memset((void*)data[0], 0, m_ns*nx*sizeof(float));

    getSeismicDataInRect(selectedRect, data, gp);

    int plotGroupIndex  = m_focusedDisplay->getGatherIndex();

    if(m_freqanaType == FreqAnaType::WholeGather){
        QString label = QString("Gather %1").arg(plotGroupIndex);
        frequencyAnalysis(selectedRect, data, label);
    }
    m_fanaDock->applyBandPassFilter(data, nx);
    m_fanaDock->setGraphyNameWithIdx(QString("After ")+filtTypeStr+QString(" filtering"), 1);
    m_inGthDisplay->displayProcessedSeismicTraces(selectedRect, data);

    Sdp2dUtils::free2float(data);
}
