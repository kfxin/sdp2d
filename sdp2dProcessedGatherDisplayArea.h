#ifndef SDP2DPROCESSEDGATHERDISPLAYAREA_H
#define SDP2DPROCESSEDGATHERDISPLAYAREA_H

#include "sdp2dGatherDisplayArea.h"
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE
class QString;
class QStatusBar;
class QMenu;
QT_END_NAMESPACE

class SeismicDataProcessing2D;
class SeismicData2D;
class Sdp2dMainGatherDisplayArea;
class Sdp2dProcessedGatherDisplayArea;

class Sdp2dProcessedGatherDisplayPlot : public QCustomPlot
{
public:
    Sdp2dProcessedGatherDisplayPlot(Sdp2dProcessedGatherDisplayArea *parent = 0);
    ~Sdp2dProcessedGatherDisplayPlot();

private:
    void mouseMoveEvent(QMouseEvent *event) override;

    Sdp2dProcessedGatherDisplayArea* m_parent;
    QStatusBar* m_statusBar;
};

class Sdp2dProcessedGatherDisplayArea : public Sdp2dGatherDisplayArea
{
    Q_OBJECT

public:
    explicit Sdp2dProcessedGatherDisplayArea(SeismicData2D* m_sd2d, QWidget *parent = nullptr);
    ~Sdp2dProcessedGatherDisplayArea();

    void displayOneGather(float** data);

    void setDataClipPercentage(int) override;
    void setDisplayType(int) override;

    void setPlotWiggleScale(float val) override;
    void setPlotXAxisType(int val) override;
    void setReversepolarity(int val) override;
    int  setGatherType(int gType, int minNtraces = 1) override;

signals:


private:

    void getDisplayParameters(void);
    void updateWiggleTraces(void);

    void setOneGroupData(float** data);    
    void updateWiggleDisplayData(void);

    void setDataForColorDisplay(void) override;
    void replotSelection(void) override;

    Q_SLOT void displayedDataChanged(const QCPRange& range);
    Q_SLOT void onCustomContextMenuRequested(const QPoint &pos);

private:

    Sdp2dMainGatherDisplayArea* m_mainGather;
    QMenu* m_myMenu;


};


#endif // SDP2DPROCESSEDGATHERDISPLAYAREA_H
