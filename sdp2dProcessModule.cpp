#include "sdp2dProcessModule.h"
#include "seismicdataprocessing2d.h"

#include <QObject>

Sdp2dProcessModule::Sdp2dProcessModule(SeismicDataProcessing2D *parent) : QObject(parent)
{
    m_mainWindow = parent;
}
