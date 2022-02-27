#include "sdp2dFileInformationTabs.h"
#include "sdp2dSegy.h"
#include "sdp2dUtils.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"

#include <cmath>
#include <cfloat>

#include <QTabWidget>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QSlider>


Sdp2dFileInformationTabs::Sdp2dFileInformationTabs(SeismicData2D* sd2d, int ntrheader, QTabWidget *parent) : QTabWidget(parent)
{
    Sdp2dSegy* sgy = sd2d->getSegyhandle();

    EbcdicHeader* ehd = sgy->get3200TextHeader();
    BinaryHeader* bhd = sgy->get400BinaryHeader();

    m_ebcHeaderTab = new Sdp2dEbcdicHeaderTab(ehd);
    this->addTab(m_ebcHeaderTab, "Ebcdic Header");

    m_binHeaderTab = new Sdp2dBinaryHeaderTab(bhd);
    this->addTab(m_binHeaderTab, "Binary Header");

    m_fileInfoTab = new Sdp2dFileInfoTab(sd2d);
    this->addTab(m_fileInfoTab, "Data Summary");

    m_trcHeaderTab = new Sdp2dTraceHeaderTab(sd2d, ntrheader);
    this->addTab(m_trcHeaderTab, "Trace Header");

    m_trcDataTab = new Sdp2dTraceDataTab(sd2d, 1);
    this->addTab(m_trcDataTab, "Trace Data");
}

void Sdp2dFileInformationTabs::updateDataPointer(SeismicData2D* sd2d)
{
    Sdp2dSegy* sgy = sd2d->getSegyhandle();

    EbcdicHeader* ehd = sgy->get3200TextHeader();
    BinaryHeader* bhd = sgy->get400BinaryHeader();
    m_ebcHeaderTab->updateDateInformation(ehd);
    m_binHeaderTab->updateDateInformation(bhd);
    m_fileInfoTab->updateDateInformation(sd2d);
    m_trcHeaderTab->updateDataPointer(sd2d);
    m_trcDataTab->updateDataPointer(sd2d);
}

Sdp2dFileInformationTabs::~Sdp2dFileInformationTabs()
{
    delete m_ebcHeaderTab;
    delete m_binHeaderTab;
    delete m_fileInfoTab;
    delete m_trcHeaderTab;
    delete m_trcDataTab;
}

void Sdp2dFileInformationTabs::setTraceDataTableValues(bool isFirstTime)
{
    m_trcDataTab->setTableValues(isFirstTime);
}

Sdp2dEbcdicHeaderTab::Sdp2dEbcdicHeaderTab(EbcdicHeader* ehd, QWidget *parent)
    : QWidget(parent)
{
    QString ehdstr = QString::fromUtf8((const char *)ehd->text_header);
    for(int i=0; i< TEXT_HEADER_SIZE; i++){
        if(ehdstr[i].isLetterOrNumber() == false){
            ehdstr[i] = ' ';
        }
    }
    m_list = new QListWidget(this);
    for(int i=0; i< 40; i++){
        QString epart = ehdstr.mid(i*80, 80);
        m_list->addItem(epart);
    }

    m_layout = new QVBoxLayout;
    m_layout->addWidget(m_list);
    setLayout(m_layout);
}

Sdp2dEbcdicHeaderTab::~Sdp2dEbcdicHeaderTab()
{
    delete m_list;
}

void Sdp2dEbcdicHeaderTab::updateDateInformation(EbcdicHeader* ehd)
{
    QString ehdstr = QString::fromUtf8((const char *)ehd->text_header);
    for(int i=0; i< TEXT_HEADER_SIZE; i++){
        if(ehdstr[i].isLetterOrNumber() == false){
            ehdstr[i] = ' ';
        }
    }

    for(int i=0; i< 40; i++){
        QString epart = ehdstr.mid(i*80, 80);
        m_list->item(i)->setText(epart);
    }
}

Sdp2dBinaryHeaderTab::Sdp2dBinaryHeaderTab(BinaryHeader* bhd, QWidget *parent)
    : QWidget(parent)
{
    const QStringList headers({tr("Header Name"), tr("Value")});
    m_table = new QTableWidget(25,2);
    m_table->setHorizontalHeaderLabels(headers);
    //m_table->setFocusPolicy(Qt::NoFocus);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //m_table->setStyleSheet("QTableView { border: none}");


    int i=0;
    QTableWidgetItem* t1;
    QTableWidgetItem* t2;
    t1 = new QTableWidgetItem(QString("job_id_number"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->job_id_number));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("line_number"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->line_number));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("reel_number"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->reel_number));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("traces_per_record"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->traces_per_record));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("aux_traces_per_record"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->aux_traces_per_record));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("sample_data_interval_ms"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->sample_data_interval_ms));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("samples_per_trace"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->samples_per_trace));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("data_sample_format_code"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->data_sample_format_code));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("CDP_fold"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->CDP_fold));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("trace_sorting_code"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->trace_sorting_code));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("vertical_sum_code"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->vertical_sum_code));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("sweep_frequency_start_hz"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->sweep_frequency_start_hz));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("sweep_frequency_end_hz"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->sweep_frequency_end_hz));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("sweep_length_ms"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->sweep_length_ms));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("sweep_type_code"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->sweep_type_code));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("trace_number_of_sweep_channel"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->trace_number_of_sweep_channel));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("sweep_trace_taper_length_start_ms"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->sweep_trace_taper_length_start_ms));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("sweep_trace_taper_length_end_ms"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->sweep_trace_taper_length_end_ms));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("taper_type_code"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->taper_type_code));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("correlated_data_traces_flag"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->correlated_data_traces_flag));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("binary_gain_recovered_flag"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->binary_gain_recovered_flag));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("amplitude_recovery_method_code"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->amplitude_recovery_method_code));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("measurement_system"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->measurement_system));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("impulse_signal_polarity"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->impulse_signal_polarity));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    t1 = new QTableWidgetItem(QString("vibratory_polarity_code"));
    t2 = new QTableWidgetItem(QString("%1").arg(bhd->vibratory_polarity_code));
    m_table->setItem(i, 0, t1);
    m_table->setItem(i, 1, t2);
    i++;

    for(int j=0; j<i; j++){
        m_table->item(j, 0)->setFlags(t1->flags() ^ Qt::ItemIsEditable);
        m_table->item(j, 1)->setFlags(t1->flags() ^ Qt::ItemIsEditable);
    }

    m_layout = new QVBoxLayout;
    m_layout->addWidget(m_table);
    setLayout(m_layout);
}

Sdp2dBinaryHeaderTab::~Sdp2dBinaryHeaderTab()
{
    delete m_table;
}

void Sdp2dBinaryHeaderTab::updateDateInformation(BinaryHeader* bhd)
{    
    m_table->item(0,  1)->setText(QString("%1").arg(bhd->job_id_number));
    m_table->item(1,  1)->setText(QString("%1").arg(bhd->line_number));
    m_table->item(2,  1)->setText(QString("%1").arg(bhd->reel_number));
    m_table->item(3,  1)->setText(QString("%1").arg(bhd->traces_per_record));
    m_table->item(4,  1)->setText(QString("%1").arg(bhd->aux_traces_per_record));
    m_table->item(5,  1)->setText(QString("%1").arg(bhd->sample_data_interval_ms));
    m_table->item(6,  1)->setText(QString("%1").arg(bhd->samples_per_trace));
    m_table->item(7,  1)->setText(QString("%1").arg(bhd->data_sample_format_code));
    m_table->item(8,  1)->setText(QString("%1").arg(bhd->CDP_fold));
    m_table->item(9,  1)->setText(QString("%1").arg(bhd->trace_sorting_code));
    m_table->item(10, 1)->setText(QString("%1").arg(bhd->vertical_sum_code));
    m_table->item(11, 1)->setText(QString("%1").arg(bhd->sweep_frequency_start_hz));
    m_table->item(12, 1)->setText(QString("%1").arg(bhd->sweep_frequency_end_hz));
    m_table->item(13, 1)->setText(QString("%1").arg(bhd->sweep_length_ms));
    m_table->item(14, 1)->setText(QString("%1").arg(bhd->sweep_type_code));
    m_table->item(15, 1)->setText(QString("%1").arg(bhd->trace_number_of_sweep_channel));
    m_table->item(16, 1)->setText(QString("%1").arg(bhd->sweep_trace_taper_length_start_ms));
    m_table->item(17, 1)->setText(QString("%1").arg(bhd->sweep_trace_taper_length_end_ms));
    m_table->item(18, 1)->setText(QString("%1").arg(bhd->taper_type_code));
    m_table->item(19, 1)->setText(QString("%1").arg(bhd->correlated_data_traces_flag));
    m_table->item(20, 1)->setText(QString("%1").arg(bhd->binary_gain_recovered_flag));
    m_table->item(21, 1)->setText(QString("%1").arg(bhd->amplitude_recovery_method_code));
    m_table->item(22, 1)->setText(QString("%1").arg(bhd->measurement_system));
    m_table->item(23, 1)->setText(QString("%1").arg(bhd->impulse_signal_polarity));
    m_table->item(24, 1)->setText(QString("%1").arg(bhd->vibratory_polarity_code));
}

Sdp2dFileInfoTab::Sdp2dFileInfoTab(SeismicData2D* sd2d, QWidget *parent)
    : QWidget(parent)
{
    QStringList& datainfo = sd2d->getDataSummary();
    int rows = datainfo.count();

    m_table = new QTableWidget(rows,2);
    m_table->setMouseTracking(true);
    //m_table->setFocusPolicy(Qt::NoFocus);
    m_table->verticalHeader()->hide();
    const QStringList headers({tr("File Inforamation"), tr("Value")});
    m_table->setHorizontalHeaderLabels(headers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    //m_table->horizontalHeader()->hide();
    //m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //m_table->setStyleSheet("QTableView { border: none}");

    for(int i=0; i<rows; i++){
        QStringList tmp = datainfo[i].split(':');
        QTableWidgetItem* t1 = new QTableWidgetItem(tmp[0].trimmed());
        QTableWidgetItem* t2 = new QTableWidgetItem(tmp[1].trimmed());
        t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
        t2->setFlags(t2->flags() ^ Qt::ItemIsEditable);
        m_table->setItem(i, 0, t1);
        m_table->setItem(i, 1, t2);        
        t1->setToolTip(tmp[0]);
        t2->setToolTip(tmp[1]);
        t1->setStatusTip(tmp[0]);
        t2->setStatusTip(tmp[1]);
    }

    m_layout = new QVBoxLayout;
    m_layout->addWidget(m_table);
    setLayout(m_layout);
}

Sdp2dFileInfoTab::~Sdp2dFileInfoTab()
{
    delete m_table;
    delete m_layout;
}

void Sdp2dFileInfoTab::updateDateInformation(SeismicData2D* sd2d)
{
    QStringList& datainfo = sd2d->getDataSummary();
    int rows = datainfo.count();
    int oldrow = m_table->rowCount();
    m_table->setRowCount(rows);

    for(int i=0; i < rows; i++){
        QStringList tmp = datainfo[i].split(':');
        if(i < oldrow){
            QTableWidgetItem* t1 =  m_table->item(i, 0);
            QTableWidgetItem* t2 =  m_table->item(i, 1);
            t1->setText(tmp[0]);
            t2->setText(tmp[1]);
            t1->setToolTip(tmp[0]);
            t2->setToolTip(tmp[1]);
            t1->setStatusTip(tmp[0]);
            t2->setStatusTip(tmp[1]);
            //cout << "set "<< tmp[0].toStdString().c_str() << " to " << tmp[1].toStdString().c_str() << endl;
        }else{
            QTableWidgetItem* t1 = new QTableWidgetItem(tmp[0]);
            QTableWidgetItem* t2 = new QTableWidgetItem(tmp[1]);
            t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
            t2->setFlags(t1->flags() ^ Qt::ItemIsEditable);
            t1->setToolTip(tmp[0]);
            t2->setToolTip(tmp[1]);
            t1->setStatusTip(tmp[0]);
            t2->setStatusTip(tmp[1]);
            m_table->setItem(i, 0, t1);
            m_table->setItem(i, 1, t2);
        }
    }
}

Sdp2dTraceHeaderTab::Sdp2dTraceHeaderTab(SeismicData2D* sd2d, int ntrheader, QWidget *parent)
    : QWidget(parent)
{
    m_ntrheader = ntrheader;
    m_sgy = sd2d->getSegyhandle();

    m_pos = 1;

    QStringList trhedName ={
        "trace_sequence_number_within_line",                      //1   1
        "trace_sequence_number_within_reel",                      //2   2
        "original_field_record_number",                           //3   3
        "trace_sequence_number_within_original_field_record",     //4   4
        "energy_source_point_number",                             //5   5
        "cdp_ensemble_number",                                    //6   6
        "trace_sequence_number_within_cdp_ensemble",              //7   7
        "trace_identification_code",                              //8   8
        "number_of_vertically_summed_traces",                     //9   9
        "number_of_horizontally_stacked_traces",                  //10
        "data_use",                                               //11
        "distance_from_source_point_to_receiver_group",           //12
        "receiver_group_elevation",                               //13
        "surface_elevation_at_source",                            //14
        "source_depth_below_surface",                             //15
        "datum_elevation_at_receiver_group",                      //16
        "datum_elevation_at_source",                              //17
        "water_depth_at_source",                                  //18
        "water_depth_at_receiver_group",                          //19
        "scalar_for_elevations_and_depths",                       //20
        "scalar_for_coordinates",                                 //21
        "x_source_coordinate",                                    //22
        "y_source_coordinate",                                    //23
        "x_receiver_group_coordinate",                            //24
        "y_receiver_group_coordinate",                            //25
        "coordinate_units",                                       //26
        "weathering_velocity",                                    //27
        "subweathering_velocity",                                 //28
        "uphole_time_at_source",                                  //29
        "uphole_time_at_group",                                   //30
        "source_static_correction",                               //31
        "group_static_correction",                                //32
        "total_static_applied",                                   //33   33
        "delay_according_time",                                   //36   34
        "mute_time_start",                                        //37   35
        "mute_time_end",                                          //38   36
        "samples_in_this_trace",                                  //39   37
        "sample_intervall",                                       //40   38
        "x_cdp_coordinate",                                       //72   39
        "y_cdp_coordinate",                                       //73   40
        "num_in_line",                                            //74   41
        "num_cross_line",                                         //75   42
        "num_shot_point",                                         //76   43
        "ave_amplitude",                                          //77   44
        "abs_amplitude",                                          //78   45
        "rms_amplitude"                                           //79   46
    };

    int ntr = sd2d->getNumberOfTraces();
    m_ntr = ntr;

    m_layout = new QVBoxLayout;

    m_table = new QTableWidget(trhedName.size(),m_ntrheader+1);
    //m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QStringList headers({"Trace Header"});
    for(int i=0; i<m_ntrheader; i++) headers = headers << QString("%1").arg(i+1);
    m_table->setHorizontalHeaderLabels(headers);

    //m_table->setVerticalHeaderLabels(trhedName);

    int j=0;
    for(int i=0; i< trhedName.size(); i++){
        QTableWidgetItem* h1 = new QTableWidgetItem();
        h1->setText(trhedName.at(i));
        m_table->setItem(i, j, h1);
    }


    m_layout->addWidget(m_table);

    if(m_ntrheader > 1){
        m_slider = new QSlider(Qt::Horizontal);
        m_slider->setFocusPolicy(Qt::StrongFocus);
        m_slider->setTickPosition(QSlider::TicksBothSides);
        m_slider->setTickInterval(50);
        m_slider->setSingleStep(10);
        m_slider->setMinimum(1);
        m_slider->setMaximum(ntr);
        m_layout->addWidget(m_slider);
        connect(m_slider, &QSlider::valueChanged, this, &Sdp2dTraceHeaderTab::fillTraceheaderTable);
    }
    setLayout(m_layout);

    setTableValues(true);
}
Sdp2dTraceHeaderTab::~Sdp2dTraceHeaderTab()
{
    delete m_slider;
    delete m_table;
    delete m_layout;
}

void Sdp2dTraceHeaderTab::updateDataPointer(SeismicData2D* sd2d)
{
    m_sgy = sd2d->getSegyhandle();
    m_pos = 1;
    setTableValues(false);
}

void Sdp2dTraceHeaderTab::fillTraceheaderTable(int value)
{
    m_pos = value;
    int start_pos = (m_pos/m_ntrheader)*m_ntrheader;
    if(start_pos + m_ntrheader > m_ntr) start_pos = m_ntr-m_ntrheader+1;
    if(start_pos <= 0) start_pos = 1;

    QStringList headers({"Trace Header"});
    for(int i=0; i<m_ntrheader; i++) headers = headers << QString("%1").arg(start_pos+i);
    m_table->setHorizontalHeaderLabels(headers);

    setTableValues(false);
}

void Sdp2dTraceHeaderTab::setTableValues(bool isFirstTime)
{
    QTableWidgetItem* h1;
    int start_pos = (m_pos/m_ntrheader)*m_ntrheader;
    if(start_pos +m_ntrheader > m_ntr) start_pos = m_ntr-m_ntrheader+1;
    if(start_pos <= 0) start_pos = 1;

    if(isFirstTime){

        for (int j = 0; j < m_ntrheader; j++){
            TraceHeaders *trh = m_sgy->getTraceHeader(start_pos+j-1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->trace_sequence_number_within_line));
            m_table->setItem(0, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->trace_sequence_number_within_reel));
            m_table->setItem(1, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->original_field_record_number));
            m_table->setItem(2, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->trace_sequence_number_within_original_field_record));
            m_table->setItem(3, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->energy_source_point_number));
            m_table->setItem(4, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->cdp_ensemble_number));
            m_table->setItem(5, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->trace_sequence_number_within_cdp_ensemble));
            m_table->setItem(6, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->trace_identification_code));
            m_table->setItem(7, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->number_of_vertically_summed_traces_yielding_this_trace));
            m_table->setItem(8, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->number_of_horizontally_stacked_traced_yielding_this_trace));
            m_table->setItem(9, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->data_use));
            m_table->setItem(10, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->distance_from_source_point_to_receiver_group));
            m_table->setItem(11, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->receiver_group_elevation));
            m_table->setItem(12, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->surface_elevation_at_source));
            m_table->setItem(13, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->source_depth_below_surface));
            m_table->setItem(14, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->datum_elevation_at_receiver_group));
            m_table->setItem(15, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->datum_elevation_at_source));
            m_table->setItem(16, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->water_depth_at_source));
            m_table->setItem(17, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->water_depth_at_receiver_group));
            m_table->setItem(18, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->scalar_for_elevations_and_depths));
            m_table->setItem(19, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->scalar_for_coordinates));
            m_table->setItem(20, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->x_source_coordinate));
            m_table->setItem(21, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->y_source_coordinate));
            m_table->setItem(22, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->x_receiver_group_coordinate));
            m_table->setItem(23, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->y_receiver_group_coordinate));
            m_table->setItem(24, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->coordinate_units));
            m_table->setItem(25, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->weathering_velocity));
            m_table->setItem(26, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->subweathering_velocity));
            m_table->setItem(27, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->uphole_time_at_source));
            m_table->setItem(28, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->uphole_time_at_group));
            m_table->setItem(29, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->source_static_correction));
            m_table->setItem(30, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->group_static_correction));
            m_table->setItem(31, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->total_static_applied));
            m_table->setItem(32, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->delay_according_time));
            m_table->setItem(33, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->mute_time_start));
            m_table->setItem(34, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->mute_time_end));
            m_table->setItem(35, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->samples_in_this_trace));
            m_table->setItem(36, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->sample_intervall));
            m_table->setItem(37, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->x_cdp_coordinate));
            m_table->setItem(38, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->y_cdp_coordinate));
            m_table->setItem(39, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->num_in_line));
            m_table->setItem(40, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->num_cross_line));
            m_table->setItem(41, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->num_shot_point));
            m_table->setItem(42, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->ave_amplitude));
            m_table->setItem(43, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->abs_amplitude));
            m_table->setItem(44, j+1, h1);

            h1 = new QTableWidgetItem(QString("%1").arg(trh->rms_amplitude));
            m_table->setItem(45, j+1, h1);
        }

        for(int i=0; i< 46; i++){
            for(int j=0; j< m_ntrheader+1; j++){
                QTableWidgetItem* t1 = m_table->item(i, j);
                t1->setFlags(t1->flags() ^ Qt::ItemIsEditable);
            }
        }


    } else {
        for (int j = 0; j < m_ntrheader; j++){
            TraceHeaders *trh = m_sgy->getTraceHeader(start_pos+j-1);

            m_table->item(0,  j+1)->setText(QString("%1").arg(trh->trace_sequence_number_within_line));
            m_table->item(1,  j+1)->setText(QString("%1").arg(trh->trace_sequence_number_within_reel));
            m_table->item(2,  j+1)->setText(QString("%1").arg(trh->original_field_record_number));
            m_table->item(3,  j+1)->setText(QString("%1").arg(trh->trace_sequence_number_within_original_field_record));
            m_table->item(4,  j+1)->setText(QString("%1").arg(trh->energy_source_point_number));
            m_table->item(5,  j+1)->setText(QString("%1").arg(trh->cdp_ensemble_number));
            m_table->item(6,  j+1)->setText(QString("%1").arg(trh->trace_sequence_number_within_cdp_ensemble));
            m_table->item(7,  j+1)->setText(QString("%1").arg(trh->trace_identification_code));
            m_table->item(8,  j+1)->setText(QString("%1").arg(trh->number_of_vertically_summed_traces_yielding_this_trace));
            m_table->item(9,  j+1)->setText(QString("%1").arg(trh->number_of_horizontally_stacked_traced_yielding_this_trace));
            m_table->item(10, j+1)->setText(QString("%1").arg(trh->data_use));
            m_table->item(11, j+1)->setText(QString("%1").arg(trh->distance_from_source_point_to_receiver_group));
            m_table->item(12, j+1)->setText(QString("%1").arg(trh->receiver_group_elevation));
            m_table->item(13, j+1)->setText(QString("%1").arg(trh->surface_elevation_at_source));
            m_table->item(14, j+1)->setText(QString("%1").arg(trh->source_depth_below_surface));
            m_table->item(15, j+1)->setText(QString("%1").arg(trh->datum_elevation_at_receiver_group));
            m_table->item(16, j+1)->setText(QString("%1").arg(trh->datum_elevation_at_source));
            m_table->item(17, j+1)->setText(QString("%1").arg(trh->water_depth_at_source));
            m_table->item(18, j+1)->setText(QString("%1").arg(trh->water_depth_at_receiver_group));
            m_table->item(19, j+1)->setText(QString("%1").arg(trh->scalar_for_elevations_and_depths));
            m_table->item(20, j+1)->setText(QString("%1").arg(trh->scalar_for_coordinates));
            m_table->item(21, j+1)->setText(QString("%1").arg(trh->x_source_coordinate));
            m_table->item(22, j+1)->setText(QString("%1").arg(trh->y_source_coordinate));
            m_table->item(23, j+1)->setText(QString("%1").arg(trh->x_receiver_group_coordinate));
            m_table->item(24, j+1)->setText(QString("%1").arg(trh->y_receiver_group_coordinate));
            m_table->item(25, j+1)->setText(QString("%1").arg(trh->coordinate_units));
            m_table->item(26, j+1)->setText(QString("%1").arg(trh->weathering_velocity));
            m_table->item(27, j+1)->setText(QString("%1").arg(trh->subweathering_velocity));
            m_table->item(28, j+1)->setText(QString("%1").arg(trh->uphole_time_at_source));
            m_table->item(29, j+1)->setText(QString("%1").arg(trh->uphole_time_at_group));
            m_table->item(30, j+1)->setText(QString("%1").arg(trh->source_static_correction));
            m_table->item(31, j+1)->setText(QString("%1").arg(trh->group_static_correction));
            m_table->item(32, j+1)->setText(QString("%1").arg(trh->total_static_applied));
            m_table->item(33, j+1)->setText(QString("%1").arg(trh->delay_according_time));
            m_table->item(34, j+1)->setText(QString("%1").arg(trh->mute_time_start));
            m_table->item(35, j+1)->setText(QString("%1").arg(trh->mute_time_end));
            m_table->item(36, j+1)->setText(QString("%1").arg(trh->samples_in_this_trace));
            m_table->item(37, j+1)->setText(QString("%1").arg(trh->sample_intervall));
            m_table->item(38, j+1)->setText(QString("%1").arg(trh->x_cdp_coordinate));
            m_table->item(39, j+1)->setText(QString("%1").arg(trh->y_cdp_coordinate));
            m_table->item(40, j+1)->setText(QString("%1").arg(trh->num_in_line));
            m_table->item(41, j+1)->setText(QString("%1").arg(trh->num_cross_line));
            m_table->item(42, j+1)->setText(QString("%1").arg(trh->num_shot_point));
            m_table->item(43, j+1)->setText(QString("%1").arg(trh->ave_amplitude));
            m_table->item(44, j+1)->setText(QString("%1").arg(trh->abs_amplitude));
            m_table->item(45, j+1)->setText(QString("%1").arg(trh->rms_amplitude));
        }
    }

}



Sdp2dTraceDataTab::Sdp2dTraceDataTab(SeismicData2D* sd2d, int ntrheader, QWidget *parent)
    : QWidget(parent)
{
    m_ntrheader = ntrheader;
    m_sgy = sd2d->getSegyhandle();

    m_pos = 1;

    m_ntr = sd2d->getNumberOfTraces();
    m_ns = sd2d->getSamplesPerTraces();
    if(m_ntr < m_ntrheader) m_ntrheader=m_ntr;

    m_data = new float[m_ns*2];

    m_layout = new QVBoxLayout;

    m_table = new QTableWidget(m_ns, m_ntrheader);
    //m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QStringList headers(QString("Trace %1").arg(m_pos));
    for(int i=1; i<m_ntrheader; i++) headers = headers << QString("Trace %1").arg(i+m_pos);
    m_table->setHorizontalHeaderLabels(headers);

    m_layout->addWidget(m_table);

    if(m_ntrheader >= 1){
        m_slider = new QSlider(Qt::Horizontal);
        m_slider->setFocusPolicy(Qt::StrongFocus);
        m_slider->setTickPosition(QSlider::TicksBothSides);
        m_slider->setTickInterval(50);
        m_slider->setSingleStep(10);
        m_slider->setMinimum(1);
        m_slider->setMaximum(m_ntr);
        m_layout->addWidget(m_slider);
        connect(m_slider, &QSlider::valueChanged, this, &Sdp2dTraceDataTab::fillTraceDataTable);
    }
    setLayout(m_layout);

    setTableValues(true);
}

Sdp2dTraceDataTab::~Sdp2dTraceDataTab()
{
    //delete [] m_data;
    delete m_slider;
    delete m_table;
    delete m_layout;
}


void Sdp2dTraceDataTab::updateDataPointer(SeismicData2D* sd2d)
{
    m_sgy = sd2d->getSegyhandle();
    m_pos = 1;
    m_ntr = sd2d->getNumberOfTraces();
    m_ns = sd2d->getSamplesPerTraces();
    if(m_ntr < m_ntrheader) m_ntrheader=m_ntr;
    setTableValues(false);
}

void Sdp2dTraceDataTab::fillTraceDataTable(int value)
{
    m_pos = value;
    int start_pos = (m_pos/m_ntrheader)*m_ntrheader;
    if(start_pos +m_ntrheader >= m_ntr) start_pos = m_ntr- m_ntrheader+1;
    if(start_pos <= 0 ) start_pos = 1;


    QStringList headers(QString("Trace %1").arg(start_pos));
    for(int i=1; i<m_ntrheader; i++) headers = headers << QString("Trace %1").arg(i+start_pos);
    m_table->setHorizontalHeaderLabels(headers);

    setTableValues(false);
}

void Sdp2dTraceDataTab::setTableValues(bool isFirstTime)
{
    QTableWidgetItem* h1;
    if(m_ntrheader < 1) m_ntrheader = 1;

    int start_pos = (m_pos/m_ntrheader)*m_ntrheader;
    if(start_pos + m_ntrheader > m_ntr) start_pos = m_ntr - m_ntrheader + 1;
    if(start_pos <= 0 ) start_pos = 1;

    //cout << "value=" << m_pos <<" start_pos=" <<   start_pos << " m_ns = " << m_ns << endl;

    for (int j = 0; j < m_ntrheader; j++){
        m_sgy->getTraceData(start_pos+j-1, m_data);
        for(int i=0; i< m_ns; i++){
            if(std::fpclassify(m_data[i]) !=  FP_NORMAL) m_data[i] = 0;
            if(fabs(m_data[i]) > 1.0e+20) m_data[i] = 0;
            //m_data[i] = 0;
        }
        if(isFirstTime){
            for(int i=0; i< m_ns; i++){
                h1 = new QTableWidgetItem(QString::number(m_data[i], 'g', 8));
                m_table->setItem(i, j, h1);
                h1->setFlags(h1->flags() ^ Qt::ItemIsEditable);
            }
        } else {
            for(int i=0; i< m_ns; i++){
                QString str = QString::number(m_data[i], 'g', 8);
                m_table->item(i, j)->setData(Qt::DisplayRole, str);
            }
        }
    }

}

