#ifndef SDP2DDISPLAYPARAMTAB_H
#define SDP2DDISPLAYPARAMTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QTabWidget;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QLineEdit;
class QStatusBar;
QT_END_NAMESPACE

class SeismicDataProcessing2D;
class SeismicData2D;

class Sdp2dDisplayParamTab : public QWidget
{
    Q_OBJECT
public:
    explicit Sdp2dDisplayParamTab(SeismicData2D* sd2d, SeismicDataProcessing2D* mainWindow, QWidget *parent = nullptr);
    ~Sdp2dDisplayParamTab();

    QString getPlotTitle(void);
    QString getPlotXLabel(void);
    QString getPlotYLabel(void);
    int getDataClipPercentage(void);
    int getGatherType(void);
    int getDisplayType(void);
    int getColorMapIndex(void);
    float getMaxDisplayTime(void);
    int getSymmetryRange(void);
    int getGroupStep(void);
    float getPlotXScale(void);
    float getPlotYScale(void);
    float getPlotWiggleScale(void);
    int getXAxisType(void);
    int getGatherToPlot(void);
    int getReversepolarity(void);


    void setPlotTitle(QString text);
    void setPlotXLabel(QString text);
    void setPlotYLabel(QString text);
    void setDataClipPercentage(int);
    void setGatherType(int);
    void setDisplayType(int);
    void setColorMapIndex(int);
    void setMaxDisplayTime(float);
    void setSymmetryRange(int);
    void setGroupStep(int);
    void setPlotXScale(float);
    void setPlotYScale(float);
    void setPlotWiggleScale(float);
    void setXAxisTypex(int);
    void setGatherToPlot(int);
    void setReversepolarity(int);

    void checkCDPGatherIndexForVelAna(int gIdx=0);

private slots:
    void changePlotDataClipPercentage(QString text);
    void changePlotTitle(QString text);
    void changePlotXLabel(QString text);
    void changePlotYLabel(QString text);
    void changePlotGatherType(QString text);
    void changePlotDisplayType(QString text);
    void changePlotColormapType(QString text);

    void changePlotSymmetryRange(QString text);
    void changePlotGroupStep(QString text);
    void changePlotMaxTime();

    void changePlotXScale();
    void changePlotYScale();

    void changeWiggleScale(QString text);
    void changeXAxisType(QString text);
    void changePlotGatherIndex();
    void changePlotPolarity(QString text);

private:
    SeismicDataProcessing2D* m_mainWindow;
    QStatusBar* m_statusbar;
    QTableWidget* m_table;

    QSpinBox* m_groupStep;
    QSpinBox* m_cpercentage;    
    QComboBox* m_gathertype;
    QComboBox* m_displaytype;
    QComboBox* m_colormap;
    QComboBox* m_symmetryrange;
    QComboBox* m_reversepolarity;
    QLineEdit* m_maxtime;
    QLineEdit* m_gatherIndex;
    QLineEdit* m_title;
    QLineEdit* m_xlabel;
    QLineEdit* m_ylabel;
    QLineEdit* m_xscale;
    QLineEdit* m_yscale;
    QDoubleSpinBox* m_wigglescale;
    QComboBox* m_xaxistype;

};

#endif // SDP2DDISPLAYPARAMTAB_H
