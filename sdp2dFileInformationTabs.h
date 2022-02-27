#ifndef FILEINFORMATIONTABS_H
#define FILEINFORMATIONTABS_H

#include "sdp2dSegy.h"
#include <QTabWidget>

QT_BEGIN_NAMESPACE
class QSlider;
class QTableWidget;
class QStringList;
class QListWidget;
class QVBoxLayout;
class QTabWidget;
QT_END_NAMESPACE

class SeismicData2D;

class Sdp2dEbcdicHeaderTab : public QWidget
{
    Q_OBJECT

public:
    explicit Sdp2dEbcdicHeaderTab(EbcdicHeader* ehd, QWidget *parent = nullptr);
    ~Sdp2dEbcdicHeaderTab();

    void updateDateInformation(EbcdicHeader* ehd);    

private:
    QListWidget* m_list;
    QVBoxLayout *m_layout;

};



class Sdp2dBinaryHeaderTab : public QWidget
{
    Q_OBJECT

public:
    explicit Sdp2dBinaryHeaderTab(BinaryHeader* bhd, QWidget *parent = nullptr);
    ~Sdp2dBinaryHeaderTab();

    void updateDateInformation(BinaryHeader* bhd);

private:
    QTableWidget* m_table;
    QVBoxLayout *m_layout;
};



class Sdp2dFileInfoTab : public QWidget
{
    Q_OBJECT

public:
    explicit Sdp2dFileInfoTab(SeismicData2D* sd2d, QWidget *parent = nullptr);
    ~Sdp2dFileInfoTab();

    void updateDateInformation(SeismicData2D* sd2d);

private:
    QTableWidget* m_table;
    QVBoxLayout *m_layout;
};




class Sdp2dTraceHeaderTab : public QWidget
{
    Q_OBJECT

public:
    explicit Sdp2dTraceHeaderTab(SeismicData2D* sd2d, int ntrheader=10, QWidget *parent = nullptr);
    ~Sdp2dTraceHeaderTab();
    void updateDataPointer(SeismicData2D* sd2d);

private:
    int m_ntrheader;
    int m_pos;
    int m_ntr;

    QSlider* m_slider;
    QTableWidget *m_table;
    QVBoxLayout *m_layout;
    Sdp2dSegy* m_sgy;

    void setTableValues(bool isFirstTime);
    void fillTraceheaderTable(int value);
};


class Sdp2dTraceDataTab : public QWidget
{
    Q_OBJECT

public:
    explicit Sdp2dTraceDataTab(SeismicData2D* sd2d, int ntrheader=10, QWidget *parent = nullptr);
    ~Sdp2dTraceDataTab();

    void setTableValues(bool isFirstTime);
    void updateDataPointer(SeismicData2D* sd2d);

private:
    int m_ntrheader;
    int m_pos;
    int m_ntr;
    int m_ns;
    float *m_data;

    QVBoxLayout *m_layout;
    QSlider* m_slider;
    QTableWidget *m_table;
    Sdp2dSegy* m_sgy;

    void fillTraceDataTable(int value);

};



class Sdp2dFileInformationTabs : public QTabWidget
{
    Q_OBJECT
public:
    explicit Sdp2dFileInformationTabs(SeismicData2D* sd2d, int ntrheader=10, QTabWidget *parent = nullptr);
    ~Sdp2dFileInformationTabs();

    void setTraceDataTableValues(bool isFirstTime = false);

    void updateDataPointer(SeismicData2D* sd2d);

private:
    Sdp2dEbcdicHeaderTab* m_ebcHeaderTab;
    Sdp2dBinaryHeaderTab* m_binHeaderTab;
    Sdp2dFileInfoTab* m_fileInfoTab;
    Sdp2dTraceHeaderTab* m_trcHeaderTab;
    Sdp2dTraceDataTab* m_trcDataTab;

signals:

};

#endif // FILEINFORMATIONTABS_H
