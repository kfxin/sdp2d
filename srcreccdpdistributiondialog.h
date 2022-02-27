#ifndef SRCRECCDPDISTRIBUTIONDIALOG_H
#define SRCRECCDPDISTRIBUTIONDIALOG_H

#include "sdp2dChartView.h"

#include <QDialog>
#include <QScatterSeries>

using namespace QtCharts;

namespace Ui {
class SrcRecCdpDistributionDialog;
}

class SeismicData2DPreStack;
class SeismicDataProcessing2D;

class SrcRecCdpDistributionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SrcRecCdpDistributionDialog(SeismicData2DPreStack* p, QString sd2dName, QWidget *mainWindow, QWidget *parent = nullptr);
    ~SrcRecCdpDistributionDialog();

private:
    void scaleChart(float factorX, float factorY);
    double getScaleFactor();
    double getPlotedDataXLength();
    double getPlotedDataYLength();
    double getPlotedDataX0();
    double getPlotedDataY0();
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void on_srcCheckBox_toggled(bool checked);
    void on_recCheckBox_toggled(bool checked);
    void on_cdpCheckBox_toggled(bool checked);

    void on_zoomInBtn_clicked();
    void on_zoomOutBtn_clicked();
    void on_fit2winBtn_clicked();
    void on_closeBtn_clicked();
    void on_zoomInXBtn_pressed();
    void on_zoomInYBtn_pressed();
    void on_zoomOutXBtn_pressed();
    void on_zoomOutYBtn_pressed();

private:
    Ui::SrcRecCdpDistributionDialog *ui;
    SeismicDataProcessing2D* m_mainWindow;

    SeismicData2DPreStack* m_sd2d;

    Sdp2dChartView *m_chartView;
    QChart *m_chart;
    QList<QPointF> m_shotXY;
    QList<QPointF> m_cdpXY;
    QList<QPointF> m_recvXY;
    QScatterSeries* m_CDPSeries;
    QScatterSeries* m_ShotSeries;
    QScatterSeries* m_RecvSeries;

    double m_scaleFactorX;
    double m_scaleFactorY;
    double m_fx;
    double m_lx;
    double m_fy;
    double m_ly;
    double m_dx;  // x length
    double m_dy;  // y length
    double m_cx;  //center x
    double m_cy;  //center y
    bool m_shift;

};

#endif // SRCRECCDPDISTRIBUTIONDIALOG_H
