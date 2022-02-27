#ifndef SDP2DGATHERDISPLAYAREA_H
#define SDP2DGATHERDISPLAYAREA_H


#include <QScrollArea>

QT_BEGIN_NAMESPACE
class QString;
class QResizeEvent;
class QStatusBar;
QT_END_NAMESPACE

class SeismicData2D;
class SeismicData2DPreStack;
class SeismicDataProcessing2D;
class Sdp2dQCPColorMap;
class Sdp2dMainGatherDisplayPlot;
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

class Sdp2dGatherDisplayArea: public QScrollArea
{
    Q_OBJECT

public:
    explicit Sdp2dGatherDisplayArea(SeismicData2D* m_sd2d, QWidget *parent = nullptr);
    ~Sdp2dGatherDisplayArea();

    QStatusBar* getStatusBar(void) const { return m_statusbar; }

    void resizeToFitTheDisplay(void);
    void resizeWithXYScales();
    QPointF getTopLeftOfPlotArea(void);
    QPointF getBtmRightOfPlotArea(void);

    void setTraceHeaderDisplay(bool checked);
    void setPlotTitle(QString text);
    void setPlotXLabel(QString text);
    void setPlotYLabel(QString text);

    virtual void setDataClipPercentage(int val) = 0;
    virtual int setGatherType(int val, int minNtraces=1) = 0;
    virtual void setDisplayType(int val) = 0;
    virtual void setPlotWiggleScale(float val) = 0;
    virtual void setPlotXAxisType(int val) = 0;
    virtual void setReversepolarity(int val) = 0;

    void setMaxDisplayTime(float val);
    void setColorMapIndex(int val);
    void setSymmetryRange(int val);
    void setGroupStep(int val);

    void setPlotXScale(float val, QPointF plotEdge, bool replot=true);
    void setPlotYScale(float val, QPointF plotEdge, bool replot=true);

    void setPlotFitZoom(bool zoomfit) { m_zoomfit=zoomfit; };

    bool getTraceHeaderDisplay(void) const { return m_displayTraceHeader; }
    int getNumTracesOfCurrentGather(void) const { return m_ntr; }
    int getTimeSamplesOfCurrentView(void) const {return int((1000*m_plotMaxTime)/(1000*m_dt)+1); }
    float getTimeSampRateOfCurrentGather(void) const {return m_dt; }

    int getGatherType()  const {return m_plotGathereType;}
    int getGatherIndex(void) const {return m_gatherIdx; }
    float getMaxDisplayTime() const {return m_plotMaxTime;}

    QString getPlotTitle() const {return m_plotTitle;}
    QString getPlotXLabel() const {return m_plotXLabel;}
    QString getPlotYLabel() const {return m_plotYLabel;}
    int getDataClipPercentage() const {return m_plotClipPercent;}

    int getDisplayType() const {return m_plotDisplayType;}
    int getColorMapIndex() const {return m_plotColormapType; }

    int getSymmetryRange(void) const {return m_plotSymRange;}
    int getGroupStep(void) const {return m_plotGroupStep;}
    float getPlotXScale(void) const { return m_xscale; }
    float getPlotYScale(void) const { return m_yscale; }
    float getPlotWiggleScale(void) const { return m_plotWiggleScale; }
    int getPlotXAxisType(void) const { return  m_plotXAxisType;}
    int getReversepolarity(void) const { return m_plotRevPolarity; }
    bool getPlotFitZoom(void) const { return m_zoomfit; }

    int getOffsetSpacing(void) const { return m_offEdge; }

    QCPRange* getOffsetValueRange(void) const { return m_offRange; }
    QCPRange* getElevationValueRange(void) const { return m_eleRange; }

    QVector<double>& getOffsetOfTheGather(void) { return m_offset; }
    QVector<double>& getElevationOfTheGather(void) { return m_elevation; }

    QPointF convertPosToAxisValues(QPointF pos);
    void convertAxisValuesToPos(double key, double value, double& x, double& y);

    float *getDataOfASeismicTrace(void) const { return m_traceIn; }
    float** getDataOfInputSeismicGather(void) const { return m_gatherIn; }

    void hideInputGather(bool hide);

    QRect getSeismicDataRange(QPoint& s, QPoint& e);

    QCPColorMap* getGatherView(void) const { return m_colorMap; }

    QCPRange getDisplayedDataValueRange(void);

    bool isApplyTopMute(void) const { return m_applyTopMute; }
    bool isApplyBtmMute(void) const { return m_applyBtmMute; }

protected:

    void setupCustomPlot(void);
    void createDisplayElements(void);
    void createGatherDisplayAxisRect(void);
    void createOffsetDisplayAxisRect(void);
    void createGatherColorDisplayBase(void);

    void calculatePlotScales(void);

    void setDataForDisplayedSingleTrace(QCPGraph* wavtr, QCPGraph* reftr, int refIdx);    

    void setColorDisplay(void);
    void setHeaderDisplay(void);
    void setWiggleDisplay(void);

    void resizeEvent(QResizeEvent *event) override;

    virtual void setDataForColorDisplay(void) = 0;
    virtual void replotSelection(void) = 0;

protected:
    SeismicDataProcessing2D* m_mainWindow;
    SeismicData2DPreStack* m_sd2d;
    QStatusBar* m_statusbar;

    //Sdp2dMainGatherDisplayPlot* m_customPlot;
    QCustomPlot* m_customPlot;
    QCPMarginGroup* m_marginVGroup;
    QCPAxisRect* m_offsetAxisRect;
    QCPAxisRect* m_gatherAxisRect;

    QCPColorMap* m_colorMap;
    QCPColorScale* m_colorScale;

    QVector<QCPGraph*> m_wigTrcGraphs;
    QVector<QCPGraph*> m_wigRefGraphs;

    QCPGraph* m_offgraph;
    QCPGraph* m_elegraph;

    QCPRange* m_offRange;
    QCPRange* m_eleRange;
    int m_offEdge;

    float** m_gatherIn;
    float* m_traceIn;

    QVector<double> m_offset;
    QVector<double> m_elevation;
    QVector<double> m_traceidx;
    QVector<double> m_time;
    QVector<double> m_tval;
    QVector<double> m_tref;

    int m_gatherIdx;
    int m_ns;
    int m_ntr;
    float m_dt;

    bool m_displayTraceHeader;
    int m_plotGathereType;
    int m_plotDisplayType;
    int m_plotClipPercent;
    int m_plotSymRange;
    int m_plotGroupStep;
    int m_plotXAxisType;
    int m_plotRevPolarity;
    float m_plotMaxTime;
    QString m_plotTitle;
    QString m_plotXLabel;
    QString m_plotYLabel;
    bool m_zoomfit;

    float m_plotWiggleScale;

    int m_plotColormapType;

    float m_xscale;
    float m_yscale;

    int m_displayedtraceIdx;

    QColor m_colorWigTrace;
    QColor m_colorEleCurve;
    QColor m_colorOffCurve;

    bool m_applyTopMute;
    bool m_applyBtmMute;
    bool m_hideBadTraces;

};

#endif // SDP2DGATHERDISPLAYAREA_H
