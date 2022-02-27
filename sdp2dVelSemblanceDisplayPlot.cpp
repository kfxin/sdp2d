#include "sdp2dVelSemblanceDisplayPlot.h"
#include "sdp2dVelSemblanceDisplayArea.h"
#include "seismicdataprocessing2d.h"
#include "seismicdata2dprestack.h"

#include <QtWidgets>
#include <QPointF>

#include <iostream>
using namespace std;


Sdp2dVelSemblanceDisplayPlot::Sdp2dVelSemblanceDisplayPlot(Sdp2dVelSemblanceDisplayArea *parent) : QCustomPlot(parent)
{
    m_parent = parent;
    m_statusBar = m_parent->getStatusBar();
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_myMenu = new QMenu(this);
    m_myMenu->addAction("Save picks to file", m_parent, &Sdp2dVelSemblanceDisplayArea::saveNMOVelocities);
    m_myMenu->addAction("Load picks from file", m_parent, &Sdp2dVelSemblanceDisplayArea::loadNMOVelocities);
    m_myMenu->addAction("Clean all picks", m_parent, &Sdp2dVelSemblanceDisplayArea::cleanAllVelocityPicks);

    connect(this, &QWidget::customContextMenuRequested, this, &Sdp2dVelSemblanceDisplayPlot::onMenuRequested);
}

Sdp2dVelSemblanceDisplayPlot::~Sdp2dVelSemblanceDisplayPlot()
{

}

void Sdp2dVelSemblanceDisplayPlot::onMenuRequested(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    m_myMenu->exec(globalPos);
}

void Sdp2dVelSemblanceDisplayPlot::mouseMoveEvent(QMouseEvent *event)
{
    QPointF tt = m_parent->convertPosToAxisValues(QPointF(event->pos().x(), event->pos().y()));
    QCPRange vrange = m_parent->getTheVelocityRange();
    QCPRange trange = m_parent->getTheTimeRange();
    float dt = m_parent->getTimeSampleRateOfSembelance();
    float dv = m_parent->getVelocityInterval();
    int ix = (tt.x() - vrange.lower)/dv;
    int it = (tt.y() - trange.lower)/dt;

    if(tt.x()< vrange.lower || tt.x() >vrange.upper ||tt.y()< trange.lower || tt.y() >trange.upper) {
        m_statusBar->clearMessage();
    } else {
        //cout << "posx="<< tt.x() << " posy="<< tt.y() << endl;
        float** data = m_parent->getDataOfVelocitySemblance();
        float val = data[ix][it];
        float time = int(tt.y()*1000 + 0.5);
        time /= 1000.0;
        m_statusBar->showMessage(QString("Velocity: %1, Time: %2s, Semblance: %3").arg(int(tt.x())).arg(time).arg(val));
    }

}

void Sdp2dVelSemblanceDisplayPlot::mousePressEvent(QMouseEvent *event)
{
    QPointF tt = m_parent->convertPosToAxisValues(QPointF(event->pos().x(), event->pos().y()));
    QCPRange vrange = m_parent->getTheVelocityRange();
    QCPRange trange = m_parent->getTheTimeRange();

    if(tt.x()< vrange.lower || tt.x() >vrange.upper ||tt.y()< trange.lower || tt.y() >trange.upper) {
        return;
    }

    int inttime = int(tt.y() * 1000);
    float velocity = tt.x();

    if(event->button() == Qt::LeftButton){
        m_parent->insertOnePick(inttime, velocity);
    }else if(event->button() == Qt::MidButton){
        m_parent->removeOnePick(inttime, velocity);
    }else if(event->button() == Qt::RightButton){
        //Usually right mouse click for popup menu
    }
    QCustomPlot::mousePressEvent(event);
}
