#ifndef SDP2DVELSEMBLANCEDISPLAYAREA_H
#define SDP2DVELSEMBLANCEDISPLAYAREA_H

#include <QScrollArea>
#include <QMultiMap>
#include <QList>
#include "seismicdata2d.h"

QT_BEGIN_NAMESPACE
class QString;
class QResizeEvent;
class QStatusBar;
QT_END_NAMESPACE

class SeismicData2DPreStack;
class SeismicDataProcessing2D;
class Sdp2dQCPColorMap;
class Sdp2dMainGatherDisplayArea;
class Sdp2dProcessedGatherDisplayArea;
class Sdp2dStackVelocityAnalysis;
class Sdp2dQDomDocument;
class QCPColorScale;
class QCPColorGradient;
class QCPRange;
class QCPCurve;
class QCPCurveData;
class QCPMarginGroup;
class QCPColorMap;
class QCPGraph;
class QCPAxisRect;
class QCPLayoutGrid;
class QCPItemText;
class QCustomPlot;


class Sdp2dVelSemblanceDisplayArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit Sdp2dVelSemblanceDisplayArea(SeismicDataProcessing2D *mainWin, QWidget* parent= nullptr);
    ~Sdp2dVelSemblanceDisplayArea();

    QStatusBar* getStatusBar(void) const { return m_statusbar; }

    QString getPlotTitle() const {return m_plotTitle;}
    QString getPlotXLabel() const {return m_plotXLabel;}
    QString getPlotYLabel() const {return m_plotYLabel;}
    float getPlotXScale(void) const { return m_xscale; }
    float getPlotYScale(void) const { return m_yscale; }
    int getColorMapIndex() const {return m_plotColormapType; }
    int getDataClipPercentage() const {return m_plotClipPercent;}
    bool getPlotFitZoom(void) const { return m_zoomfit; }

    QCPColorMap* getGatherView(void) const { return m_colorMap; }

    void setNMOGatherDisplayPointer(Sdp2dProcessedGatherDisplayArea* p) { m_nmoGather = p; }

    Sdp2dStackVelocityAnalysis* setupSembDisplay(Sdp2dQDomDocument* para);
    void setDataOfSemblance();

    void setColorMapIndex(int val);
    void setDataClipPercentage(int val);

    void setPlotFitZoom(bool zoomfit) { m_zoomfit=zoomfit; }

    Sdp2dStackVelocityAnalysis* getStackVelAnaWorker(void) const { return m_velana;}

    QPointF convertPosToAxisValues(QPointF pos);

    float** getDataOfVelocitySemblance();
    int getNumberOfVelocities();
    int getNumberOfTimeSamples();

    QCPRange getTheVelocityRange();
    QCPRange getTheTimeRange();
    float getTimeSampleRateOfSembelance();
    float getFirstVelocity();
    float getVelocityInterval();

    void setParametersToDom(Sdp2dQDomDocument* para);
    bool getParametersFromDom(Sdp2dQDomDocument* domDoc);

    void processCurrentGather();
    void processWholeData(Sdp2dQDomDocument* para);

    void insertOnePick(int itsamp, float velocity);
    void removeOnePick(int itsamp, float velocity);

    Q_SLOT void saveNMOVelocities(void);
    Q_SLOT void loadNMOVelocities(void);
    Q_SLOT void cleanAllVelocityPicks(void);

private:
    void resizeEvent(QResizeEvent *event) override;
    void setupCustomPlot(void);
    void createDisplayElements(void);
    void createSembDisplayAxisRect(void);
    void createSembColorDisplayBase(void);

    void calculatePlotScales(void);
    void resizeToFitTheDisplay();

    void createNMOVelocityPicksCurve(void);
    void setVelocitySembalance(void);

    void updatePicksCurveAndNMOGather(float** indata=nullptr);
    void plotPickedNMPVelocityCurve();
    void plotLeftNMPVelocityCurve();
    void plotRightNMPVelocityCurve();
    bool generateNMOVelocityOfCurrentCDP(float* vnmo);
    bool generateNMOVelocityOfCDP(int gIndex, float* ovv);
    void fillTVPairs(QList<value_idx>& tvPairs, QVector<float>& qtim, QVector<float>& qvel, int maxTime, int sampTol);

private:
    SeismicDataProcessing2D* m_mainWindow;
    SeismicData2DPreStack* m_sd2d;
    Sdp2dStackVelocityAnalysis* m_velana;
    Sdp2dMainGatherDisplayArea* m_mainGather;
    Sdp2dProcessedGatherDisplayArea* m_nmoGather;
    QStatusBar* m_statusbar;

    QCustomPlot* m_customPlot;
    QCPMarginGroup* m_marginVGroup;
    QCPAxisRect* m_sembAxisRect;

    QCPColorMap* m_colorMap;
    QCPColorScale* m_colorScale;

    float** m_sembData;

    QString m_plotTitle;
    QString m_plotXLabel;
    QString m_plotYLabel;
    int m_plotClipPercent;
    int m_plotColormapType;
    bool m_zoomfit;    

    float m_xscale;
    float m_yscale;

    float m_defaultVelAtTop;
    float m_defaultVelAtBtm;
    bool m_stackGather;
    float m_normpow;

    QCPGraph* m_btm;
    QCPGraph* m_top;
    QCPGraph* m_mid;
    QCPGraph* m_lft;
    QCPGraph* m_rht;
    QCPGraph* m_ped;

    int m_lftCDP;
    int m_rhtCDP;

};

#endif // SDP2DVELSEMBLANCEDISPLAYAREA_H
