QT       += core gui charts xml

greaterThan(QT_MAJOR_VERSION, 4): QT += concurrent widgets printsupport

CONFIG += c++11 -stdlib=libc++ -std=c++11

#INCLUDEPATH += ${FFTWROOT}/include
#LIBS += -L${FFTWROOT}/lib -lfftw3f -lm

INCLUDEPATH += /Users/kxin/opt/fftw-3.3.8/include
LIBS += -L/Users/kxin/opt/fftw-3.3.8/lib -lfftw3f -lm

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    qcustomplot.cpp \
    sdp2dAmplitudeDisplayDock.cpp \
    sdp2dDataInfoTabs.cpp \
    sdp2dDisplayParamTab.cpp \
    sdp2dFileInformationTabs.cpp \
    main.cpp \
    sdp2dFrequencyAnalysisDock.cpp \
    sdp2dGatherDisplayArea.cpp \
    sdp2dMainGatherDisplayArea.cpp \
    sdp2dModuleParametersSetupDlg.cpp \
    sdp2dOpenedDataDock.cpp \
    sdp2dChartView.cpp \
    sdp2dMapDiaplayDock.cpp \
    sdp2dPMElevationStaticCorrection.cpp \
    sdp2dPMFilter.cpp \
    sdp2dStackVelocityAnalysis.cpp \
    sdp2dPMSuGain.cpp \
    sdp2dParasSettingDelegate.cpp \
    sdp2dParasSettingItem.cpp \
    sdp2dParasSettingModel.cpp \
    sdp2dPreStackMutePicks.cpp \
    sdp2dProcessJobDelegate.cpp \
    sdp2dProcessJobDock.cpp \
    sdp2dMainGatherDisplayPlot.cpp \
    sdp2dProcessJobTreeWidget.cpp \
    sdp2dProcessModule.cpp \
    sdp2dProcessedGatherDisplayArea.cpp \
    sdp2dQDomDocument.cpp \
    sdp2dSegy.cpp \
    sdp2dVelSemblanceDisplayArea.cpp \
    sdp2dVelSemblanceDisplayPlot.cpp \
    seismicdata2d.cpp \
    seismicdata2dprestack.cpp \
    seismicdataprocessing2d.cpp \
    srcreccdpdistributiondialog.cpp \
    sdp2dUtils.cpp

HEADERS += \
    qcustomplot.h \
    sdp2dAmplitudeDisplayDock.h \
    sdp2dDataInfoTabs.h \
    sdp2dDisplayParamTab.h \
    sdp2dFileInformationTabs.h \
    sdp2dFrequencyAnalysisDock.h \
    sdp2dGatherDisplayArea.h \
    sdp2dMainGatherDisplayArea.h \
    sdp2dModuleParametersSetupDlg.h \
    sdp2dOpenedDataDock.h \
    sdp2dChartView.h \
    sdp2dMapDiaplayDock.h \
    sdp2dPMElevationStaticCorrection.h \
    sdp2dPMFilter.h \
    sdp2dStackVelocityAnalysis.h \
    sdp2dPMSuGain.h \
    sdp2dParasSettingDelegate.h \
    sdp2dParasSettingItem.h \
    sdp2dParasSettingModel.h \
    sdp2dPreStackMutePicks.h \
    sdp2dProcessJobDelegate.h \
    sdp2dProcessJobDock.h \
    sdp2dMainGatherDisplayPlot.h \
    sdp2dProcessJobTreeWidget.h \
    sdp2dProcessModule.h \
    sdp2dProcessedGatherDisplayArea.h \
    sdp2dQDomDocument.h \
    sdp2dSegy.h \
    sdp2dVelSemblanceDisplayArea.h \
    sdp2dVelSemblanceDisplayPlot.h \
    seismicdata2d.h \
    seismicdata2dprestack.h \
    seismicdataprocessing2d.h \
    srcreccdpdistributiondialog.h \
    sdp2dUtils.h

FORMS += \
    seismicdataprocessing2d.ui \
    srcreccdpdistributiondialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += sdp2d.qrc
