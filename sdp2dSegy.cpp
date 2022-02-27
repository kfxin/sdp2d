#include "sdp2dSegy.h"
#include "sdp2dUtils.h"
#include "seismicdata2d.h"

#include <QStatusBar>
#include <QtGlobal>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <math.h>

using namespace std;

/**
 * default constructor
 */
Sdp2dSegy::Sdp2dSegy() = default;


/**
 * overload constructor
 */
Sdp2dSegy::Sdp2dSegy(const string& segy_file_name, QStatusBar *sbar):
    QObject(), m_filename(segy_file_name), m_sbar(sbar)
{
    m_eHeader = nullptr;
    m_bHeader = nullptr;
    m_tHeader = nullptr;

    m_interFileReady = false;
    m_ampcalculated = false;

    m_minaveamp = 0;
    m_maxaveamp = 0;
    m_minabsamp = 0;
    m_maxabsamp = 0;
    m_minrmsamp = 0;
    m_maxrmsamp = 0;

    m_segyin.open(m_filename,ios::in|ios::binary);

    if (!m_segyin.is_open()){
       cerr << "the file cannot open, Please double check!" << endl;
    }

    readTextHeader();
    readBinaryHeader();
    calNumberOfTraces();
    allocateTraceHeaders();

    //std::ofstream data_output("test.dat", ios::out|ios::binary);
    //float *trace = new float[m_ns];
    //for(int i=121; i<240; i++){
    //    getTraceData(i, trace);
    //    data_output.write((char *)trace, m_ns*sizeof(float));
    //}
    //delete [] trace;
    //data_output.close();

    //TraceHeaders *trh = getTraceHeader(100);
    //cout << "Number of samples per trace: "<< trh->samples_in_this_trace << endl;
    //cout << "Smaple rate: " << trh->sample_intervall << endl;
    //cout << "m_dt: " << m_dt << endl;
    //cout << "m_ns: " << m_ns << endl;
    //cout << "m_dtype: "<< m_dtype << endl;
}


Sdp2dSegy::Sdp2dSegy(Sdp2dSegy* sp, QStatusBar *sbar) :
    QObject(), m_sbar(sbar)
{
    m_filename = sp->getSEGYFileName();
    m_ntr = sp->getNumberOfTraces();
    m_ns  = sp->getSamplesPerTrace();
    m_dt  = sp->getTimeSampleRateInUs();
    m_dtype = sp->getDataType();

    m_eHeader = new EbcdicHeader;
    memcpy((void*)m_eHeader, (void*)(sp->get3200TextHeader()), TEXT_HEADER_SIZE);

    m_bHeader = new BinaryHeader;
    memcpy((void*)m_bHeader, (void*)(sp->get400BinaryHeader()), BINARY_HEADER_SIZE);
    this->setTraceDataSwapbytes(sp->getTraceDataSwapbytes());

    QPointF aveAmp, absAmp, rmsAmp;
    sp->getRangeOfAmplitude(aveAmp, absAmp, rmsAmp);
    setRangeOfAmplitude(aveAmp, absAmp, rmsAmp);
    setAmplitudeCalculated(sp->getAmplitudeCalculated());

    m_segyin.open(m_filename,ios::in|ios::binary);

    if (!m_segyin.is_open()){
       cerr << "the file cannot open, Please double check!" << endl;
    }

    allocateTraceHeaders();

    for(int i=0; i< m_ntr; i++){
        TraceHeaders* tmp_thdr = new TraceHeaders;
        memcpy((void*)tmp_thdr, (void*)sp->getTraceHeader(i), TRACE_HEADER_SIZE);
        m_tHeader[i] = tmp_thdr;
    }
}

/**
 * destructor
 */
Sdp2dSegy::~Sdp2dSegy()
{
    if (m_segyin.is_open()) m_segyin.close();

    if (m_bHeader != nullptr) delete m_bHeader;
    if (m_eHeader != nullptr) delete m_eHeader;

    if (!m_tHeader)        {
        for(int i=0; i< m_ntr; i++){
            if(m_tHeader[i] != nullptr) {
                //cout << "find m_tHeader: "<< i << endl;
                delete m_tHeader[i];
            }
        }
        delete [] m_tHeader;
    }
}


/**
 * read the text header from the segy file
 */
void Sdp2dSegy::readTextHeader(void)
{
    bool isCloseFile =  false;

    if (!m_segyin.is_open())
    {
        isCloseFile = true;
        m_segyin.open(m_filename,ios::in|ios::binary);
    }

    m_segyin.seekg(0, ios::beg);

    const auto tmp_ehdr = new EbcdicHeader;
    m_segyin.read(reinterpret_cast<char *>(tmp_ehdr), TEXT_HEADER_SIZE);

    if(isCloseFile) {
        m_segyin.close();
    }

    Sdp2dUtils::ebcdic2Ascii(tmp_ehdr);
    m_eHeader= tmp_ehdr;
}


/**
 * read binary header from segy file
 */
void Sdp2dSegy::readBinaryHeader(void)
{
    bool isCloseFile =  false;

    if (!m_segyin.is_open())
    {
        isCloseFile = true;
        m_segyin.open(m_filename,ios::in|ios::binary);
    }

    //move the pointer to the beginning of the binary header
    m_segyin.seekg(TEXT_HEADER_SIZE, ios::beg);

    BinaryHeader* tmp_bhdr = new BinaryHeader;

    //assignment
    m_segyin.read((char *)tmp_bhdr, BINARY_HEADER_SIZE);

    //move the file pointer to the original position
    if(isCloseFile) {
        m_segyin.close();
    }

    m_headswapbytes = checkEndian(tmp_bhdr);
    m_dataswapbytes = m_headswapbytes;
    cout <<"m_swapbytes = " << m_headswapbytes << endl;
    //swap the binary header
    if(m_headswapbytes) swapBinaryHeader(tmp_bhdr);

    m_bHeader = tmp_bhdr;

    m_ns = static_cast<int>(m_bHeader->samples_per_trace);
    m_dtype = static_cast<int>(m_bHeader->data_sample_format_code);
    m_dt = static_cast<int>(m_bHeader->sample_data_interval_ms);
    cout << "m_dtype = " << m_dtype << endl;
}

void Sdp2dSegy::allocateTraceHeaders(void)
{
    //m_tHeader.resize(m_ntr,nullptr);
    m_tHeader = new TraceHeaders* [m_ntr];
    for(int i=0; i< m_ntr; i++){
        m_tHeader[i] = nullptr;
    }
}

void Sdp2dSegy::keepSEGYTraceHeader(int trace_idx, TraceHeaders* trh)
{
    if (trace_idx<m_ntr && trace_idx>=0){
        if(m_tHeader[trace_idx] != nullptr) delete m_tHeader[trace_idx];
        m_tHeader[trace_idx] = trh;
    }
}

TraceHeaders* Sdp2dSegy::getTraceHeader(const int trace_idx)
{
    if (trace_idx<m_ntr && trace_idx>=0){
        if(m_tHeader[trace_idx] == nullptr) {
            readTraceHeader(trace_idx);
            //int i = trace_idx;
            //std::cout << "shot station: "<< m_tHeader[i]->energy_source_point_number << ", receiver station: "<< m_tHeader[i]->cdp_ensemble_number << std::endl;
        }
        return m_tHeader[trace_idx];
    } else {
        return nullptr;
    }
}

void Sdp2dSegy::readAllTracesHeader(void)
{
    for(int i=0; i< m_ntr; i++){
        if(m_tHeader[i] == nullptr) readTraceHeader(i);        
    }
}

TraceHeaders** Sdp2dSegy::getAllTracesHeader(void)
{    
    return m_tHeader;
}

/**
 * read the (trace_idx)th trace header
 */
void Sdp2dSegy::readTraceHeader(const int trace_idx)
{
    if(m_tHeader[trace_idx] != nullptr) delete m_tHeader[trace_idx];

    bool isCloseFile =  false;

    if (!m_segyin.is_open()){
        isCloseFile = true;
        m_segyin.open(m_filename,ios::in|ios::binary);
    }

    TraceHeaders* tmp_thdr = new TraceHeaders;
    size_t byte_offset = SEGY_HEADER_SIZE + trace_idx*(TRACE_HEADER_SIZE + 4 * m_ns);
    m_segyin.seekg(byte_offset, ios::beg);
    m_segyin.read(reinterpret_cast<char*>(tmp_thdr), TRACE_HEADER_SIZE);

    if(isCloseFile) {
        m_segyin.close();
    }

    if(m_headswapbytes) swapTraceHeader(tmp_thdr);
    m_tHeader[trace_idx] = tmp_thdr;
    //int i = trace_idx;
    //cout << "shot station: "<< m_tHeader[i]->energy_source_point_number << ", receiver station: "<< m_tHeader[i]->cdp_ensemble_number << endl;
}


bool Sdp2dSegy::checkEndian(BinaryHeader* bhdr)
{
    unsigned short fmtcode = bhdr->data_sample_format_code;
    cout << "fmtcode 1 = " << fmtcode << endl;
    Sdp2dUtils::integerSwapByte(&fmtcode, 2);
    cout << "fmtcode 2 = " << fmtcode << endl;
    if(fmtcode > 0 && fmtcode < 10) return true;

    return false;
}

void Sdp2dSegy::calNumberOfTraces()
{
    bool isCloseFile =  false;
    size_t pos;
    if (!m_segyin.is_open())
    {
        isCloseFile = true;
        m_segyin.open(m_filename,ios::in|ios::binary);
    } else {
        pos = m_segyin.tellg();
    }

    //get the size of the file
    m_segyin.seekg(0, ios::end);
    const size_t file_size = m_segyin.tellg();//bytes

    m_ntr = (file_size - SEGY_HEADER_SIZE) / (TRACE_HEADER_SIZE + m_ns * 4);

    if(isCloseFile) {
        m_segyin.close();
    }else{
        m_segyin.seekg(pos, ios::beg);
    }
}


/**
 * get one sample at the position of (trace_idx,sample_idx) from the file
 */
float Sdp2dSegy::getSpecificData(const int trace_idx, const int sample_idx)
{

    if ((trace_idx >= 0) &&(trace_idx < m_ntr) &&(sample_idx >= 0)&&(sample_idx < m_ns))
    {
        float result;
        const size_t byte_offset = SEGY_HEADER_SIZE + trace_idx *(TRACE_HEADER_SIZE + 4 * m_ns) + TRACE_HEADER_SIZE
                                   + sample_idx * 4 ;
        m_segyin.seekg(byte_offset, ios::beg);
        m_segyin.read((char*)&result, 4);

        if (m_dtype == DataType::IBM) {
            Sdp2dUtils::ibm2Ieee(&result, true);
        } else if(m_dataswapbytes){
            Sdp2dUtils::swapByte4(reinterpret_cast<unsigned int *>(&result));
        }
        return result;
    }
    else
    {
        cerr << "trace_idx or sample_idx out of range.Please check." << endl;
        return 0;
    }
}

/**
 * get one trace at the position of trace_idx from the file
 */
bool Sdp2dSegy::getTraceData(const int trace_idx, float *trace)
{
    if(trace == nullptr) return false;

    //cout << "trace_idx=" << trace_idx << " m_dtype=" << m_dtype << " m_dataswapbytes=" << m_dataswapbytes << endl;
    //m_dtype = 5;

    if (trace_idx >= 0 && trace_idx < m_ntr ) {
        const size_t byte_offset = SEGY_HEADER_SIZE + trace_idx *(TRACE_HEADER_SIZE + 4 * m_ns) + TRACE_HEADER_SIZE;
        m_segyin.seekg(byte_offset, ios::beg);
        m_segyin.read((char*)trace, m_ns*sizeof(float));

        //cout << "m_dtype="<<m_dtype<< " m_swapbytes="<<m_dataswapbytes<<endl;

        /*
        float minv = 0;
        float maxv = 0;

        for(int j=0; j< m_ns; j++){
                if(trace[j] < minv) minv = trace[j];
                if(trace[j] > maxv) maxv = trace[j];
        }
        //cout << "i =" << trace_idx <<  " minv=" << minv << " maxv=" << maxv << " m_dtype="<< m_dtype<< " m_headswapbytes="<< m_headswapbytes<< endl;
        */

        if (m_dtype == DataType::IBM) {
            for(int i=0; i<m_ns; i++)
                Sdp2dUtils::ibm2Ieee(&trace[i], m_dataswapbytes);
        } else if(m_dataswapbytes){
            for(int i=0; i<m_ns; i++)
                Sdp2dUtils::swapByte4(reinterpret_cast<unsigned int *>(&trace[i]));
        }

        /*
        minv = 0;
        maxv = 0;
        for(int j=0; j< m_ns; j++){
                if(trace[j] < minv) minv = trace[j];
                if(trace[j] > maxv) maxv = trace[j];
        }
        //cout << "i =" << trace_idx <<  " minv=" << minv << " maxv=" << maxv << endl;
        */


        return true;
    } else {
        cerr << "trace_idx is out of range. Please check." << endl;
        return false;
    }
}

bool Sdp2dSegy::getTraceDataFromIntermFile(const int trace_idx, float *trace)
{
    if(trace == nullptr) return false;
    if(!m_interFileReady) {
        return getTraceData(trace_idx, trace);
    }
    qint64 pos = trace_idx *(TRACE_HEADER_SIZE + 4 * m_ns) + TRACE_HEADER_SIZE;
    qint64 size = 4 * m_ns;
    m_interFile->seek(pos);
    qint64 nin = m_interFile->read((char*)trace, size);
    if(nin == 0) return false;
    return true;
}

bool Sdp2dSegy::writeTraceDataToIntermFile(const int trace_idx, float *trace)
{
    qint64 pos = trace_idx *(TRACE_HEADER_SIZE + 4 * m_ns) + TRACE_HEADER_SIZE;
    qint64 size = 4 * m_ns;
    m_interFile->seek(pos);
    qint64 nin = m_interFile->write((char*)trace, size);
    if(nin == -1) return false;
    return true;
}


void Sdp2dSegy::swapBinaryHeader(BinaryHeader* bh)
{
    Sdp2dUtils::integerSwapByte(&(bh->job_id_number), 4);
    Sdp2dUtils::integerSwapByte(&(bh->line_number), 4);
    Sdp2dUtils::integerSwapByte(&(bh->reel_number), 4);

    Sdp2dUtils::integerSwapByte(&(bh->traces_per_record), 2);
    Sdp2dUtils::integerSwapByte(&(bh->aux_traces_per_record), 2);
    Sdp2dUtils::integerSwapByte(&(bh->sample_data_interval_ms), 2);
    Sdp2dUtils::integerSwapByte(&(bh->original_data_interval_ms), 2);
    Sdp2dUtils::integerSwapByte(&(bh->samples_per_trace), 2);
    Sdp2dUtils::integerSwapByte(&(bh->original_samples_per_trace), 2);
    Sdp2dUtils::integerSwapByte(&(bh->data_sample_format_code), 2);
    Sdp2dUtils::integerSwapByte(&(bh->CDP_fold), 2);
    Sdp2dUtils::integerSwapByte(&(bh->trace_sorting_code), 2);
    Sdp2dUtils::integerSwapByte(&(bh->vertical_sum_code), 2);
    Sdp2dUtils::integerSwapByte(&(bh->sweep_frequency_start_hz), 2);
    Sdp2dUtils::integerSwapByte(&(bh->sweep_frequency_end_hz), 2);
    Sdp2dUtils::integerSwapByte(&(bh->sweep_length_ms), 2);
    Sdp2dUtils::integerSwapByte(&(bh->sweep_type_code), 2);
    Sdp2dUtils::integerSwapByte(&(bh->trace_number_of_sweep_channel), 2);
    Sdp2dUtils::integerSwapByte(&(bh->sweep_trace_taper_length_start_ms), 2);
    Sdp2dUtils::integerSwapByte(&(bh->sweep_trace_taper_length_end_ms), 2);
    Sdp2dUtils::integerSwapByte(&(bh->taper_type_code), 2);
    Sdp2dUtils::integerSwapByte(&(bh->correlated_data_traces_flag), 2);
    Sdp2dUtils::integerSwapByte(&(bh->binary_gain_recovered_flag), 2);
    Sdp2dUtils::integerSwapByte(&(bh->amplitude_recovery_method_code), 2);
    Sdp2dUtils::integerSwapByte(&(bh->measurement_system), 2);
    Sdp2dUtils::integerSwapByte(&(bh->impulse_signal_polarity), 2);
    Sdp2dUtils::integerSwapByte(&(bh->vibratory_polarity_code), 2);
    //cout <<"m_ns: " << bh->m_ns << endl;
    //cout <<"sample_data_interval_ms: " << bh->sample_data_interval_ms<< endl;

}

/**
 * swap the segy file trace header
 */
void Sdp2dSegy::swapTraceHeader(TraceHeaders * th)
{
    Sdp2dUtils::integerSwapByte(&(th->trace_sequence_number_within_line), 4);
    Sdp2dUtils::integerSwapByte(&(th->trace_sequence_number_within_reel), 4);
    Sdp2dUtils::integerSwapByte(&(th->original_field_record_number), 4);

    Sdp2dUtils::integerSwapByte(&(th->trace_sequence_number_within_original_field_record), 4);
    Sdp2dUtils::integerSwapByte(&(th->energy_source_point_number), 4);
    Sdp2dUtils::integerSwapByte(&(th->cdp_ensemble_number), 4);
    Sdp2dUtils::integerSwapByte(&(th->trace_sequence_number_within_cdp_ensemble), 4);

    Sdp2dUtils::integerSwapByte(&(th->trace_identification_code), 2);
    Sdp2dUtils::integerSwapByte(&(th->number_of_vertically_summed_traces_yielding_this_trace), 2);
    Sdp2dUtils::integerSwapByte(&(th->number_of_horizontally_stacked_traced_yielding_this_trace), 2);
    Sdp2dUtils::integerSwapByte(&(th->data_use), 2);

    Sdp2dUtils::integerSwapByte(&(th->distance_from_source_point_to_receiver_group), 4);
    Sdp2dUtils::integerSwapByte(&(th->receiver_group_elevation), 4);
    Sdp2dUtils::integerSwapByte(&(th->surface_elevation_at_source), 4);
    Sdp2dUtils::integerSwapByte(&(th->source_depth_below_surface), 4);
    Sdp2dUtils::integerSwapByte(&(th->datum_elevation_at_receiver_group), 4);
    Sdp2dUtils::integerSwapByte(&(th->datum_elevation_at_source), 4);
    Sdp2dUtils::integerSwapByte(&(th->water_depth_at_source), 4);
    Sdp2dUtils::integerSwapByte(&(th->water_depth_at_receiver_group), 4);

    Sdp2dUtils::integerSwapByte(&(th->scalar_for_elevations_and_depths), 2);
    Sdp2dUtils::integerSwapByte(&(th->scalar_for_coordinates), 2);

    Sdp2dUtils::integerSwapByte(&(th->x_source_coordinate), 4);
    Sdp2dUtils::integerSwapByte(&(th->y_source_coordinate), 4);
    Sdp2dUtils::integerSwapByte(&(th->x_receiver_group_coordinate), 4);
    Sdp2dUtils::integerSwapByte(&(th->y_receiver_group_coordinate), 4);

    Sdp2dUtils::integerSwapByte(&(th->coordinate_units), 2);
    Sdp2dUtils::integerSwapByte(&(th->weathering_velocity), 2);
    Sdp2dUtils::integerSwapByte(&(th->subweathering_velocity), 2);
    Sdp2dUtils::integerSwapByte(&(th->uphole_time_at_source), 2);
    Sdp2dUtils::integerSwapByte(&(th->uphole_time_at_group), 2);
    Sdp2dUtils::integerSwapByte(&(th->source_static_correction), 2);
    Sdp2dUtils::integerSwapByte(&(th->group_static_correction), 2);
    Sdp2dUtils::integerSwapByte(&(th->total_static_applied), 2);
    Sdp2dUtils::integerSwapByte(&(th->lag_time_a), 2);
    Sdp2dUtils::integerSwapByte(&(th->lag_time_b), 2);
    Sdp2dUtils::integerSwapByte(&(th->delay_according_time), 2);
    Sdp2dUtils::integerSwapByte(&(th->mute_time_start), 2);
    Sdp2dUtils::integerSwapByte(&(th->mute_time_end), 2);
    Sdp2dUtils::integerSwapByte(&(th->samples_in_this_trace), 2);
    Sdp2dUtils::integerSwapByte(&(th->sample_intervall), 2);
    Sdp2dUtils::integerSwapByte(&(th->gain_type_instruments), 2);
    Sdp2dUtils::integerSwapByte(&(th->igc), 2);
    Sdp2dUtils::integerSwapByte(&(th->igi), 2);
    Sdp2dUtils::integerSwapByte(&(th->corr), 2);
    Sdp2dUtils::integerSwapByte(&(th->sfs), 2);
    Sdp2dUtils::integerSwapByte(&(th->sfe), 2);
    Sdp2dUtils::integerSwapByte(&(th->slen), 2);
    Sdp2dUtils::integerSwapByte(&(th->styp), 2);
    Sdp2dUtils::integerSwapByte(&(th->stas), 2);
    Sdp2dUtils::integerSwapByte(&(th->stae), 2);
    Sdp2dUtils::integerSwapByte(&(th->tatyp), 2);
    Sdp2dUtils::integerSwapByte(&(th->afilf), 2);
    Sdp2dUtils::integerSwapByte(&(th->afils), 2);
    Sdp2dUtils::integerSwapByte(&(th->nofilf), 2);
    Sdp2dUtils::integerSwapByte(&(th->nofils), 2);
    Sdp2dUtils::integerSwapByte(&(th->lcf), 2);
    Sdp2dUtils::integerSwapByte(&(th->hcf), 2);
    Sdp2dUtils::integerSwapByte(&(th->lcs), 2);
    Sdp2dUtils::integerSwapByte(&(th->hcs), 2);
    Sdp2dUtils::integerSwapByte(&(th->year), 2);
    Sdp2dUtils::integerSwapByte(&(th->day), 2);
    Sdp2dUtils::integerSwapByte(&(th->hour), 2);
    Sdp2dUtils::integerSwapByte(&(th->minute), 2);
    Sdp2dUtils::integerSwapByte(&(th->sec), 2);
    Sdp2dUtils::integerSwapByte(&(th->timbas), 2);
    Sdp2dUtils::integerSwapByte(&(th->trwf), 2);
    Sdp2dUtils::integerSwapByte(&(th->grnors), 2);
    Sdp2dUtils::integerSwapByte(&(th->grnofr), 2);
    Sdp2dUtils::integerSwapByte(&(th->grnlof), 2);
    Sdp2dUtils::integerSwapByte(&(th->gaps), 2);
    Sdp2dUtils::integerSwapByte(&(th->otrav), 2);

    Sdp2dUtils::integerSwapByte(&(th->x_cdp_coordinate), 4);
    Sdp2dUtils::integerSwapByte(&(th->y_cdp_coordinate), 4);
    Sdp2dUtils::integerSwapByte(&(th->num_in_line), 4);
    Sdp2dUtils::integerSwapByte(&(th->num_cross_line), 4);
    Sdp2dUtils::integerSwapByte(&(th->num_shot_point), 4);
    Sdp2dUtils::integerSwapByte(&(th->ave_amplitude), 4);
    Sdp2dUtils::integerSwapByte(&(th->abs_amplitude), 4);
    Sdp2dUtils::integerSwapByte(&(th->rms_amplitude), 4);
}


void Sdp2dSegy::getAllCDPXY(QList<QPointF>& data)
{
    //cout << "Number of shot gathers: " << m_pcdp->ngroups << endl;
    float xyscale = m_tHeader[0]->scalar_for_coordinates;
    if(xyscale < 0) xyscale = 1./fabs(xyscale);
    if(fabs(xyscale) < 0.0000001) xyscale=1;

    float cdpx, cdpy;

    for(int i=0; i < m_ntr; i++){
        cdpx = (m_tHeader[i]->x_source_coordinate + m_tHeader[i]->x_receiver_group_coordinate) * 0.5 * xyscale;
        cdpy = (m_tHeader[i]->y_source_coordinate + m_tHeader[i]->y_receiver_group_coordinate) * 0.5 * xyscale;
        const QPointF value(cdpx, cdpy);
        data.append(value);
        //cout << "i=" << i << " cdpx=" << cdpx << " cdpy=" << cdpy << endl;
    }
}

void Sdp2dSegy::setTraceDataSwapbytes(int swapbytes)
{
    cout << " swapbytes = " << swapbytes << endl;
    switch(swapbytes){
    case 0:
        m_dataswapbytes = m_headswapbytes;
        break;
    case 1:
        m_dataswapbytes = 1;
        m_dtype = DataType::IBM;
        break;
    case 2:
        m_dataswapbytes = 0;
        m_dtype = DataType::IBM;
        break;
    case 3:
        m_dataswapbytes = 1;
        m_dtype = DataType::IEEE;
        break;
    case 4:
        m_dataswapbytes = 0;
        m_dtype = DataType::IEEE;
        break;
    default:
        m_dataswapbytes = m_headswapbytes;
    }
}

int Sdp2dSegy::getTraceDataSwapbytes()
{
    int swapbytes = 0;
    if(m_dataswapbytes == 0){
        if(m_dtype == DataType::IBM){
            swapbytes = 2;
        }else {
            swapbytes = 4;
        }
    } else {
        if(m_dtype == DataType::IBM){
            swapbytes = 1;
        }else {
            swapbytes = 3;
        }
    }
    return swapbytes;
}

void Sdp2dSegy::calculateAmplitudePutToHeader(Sdp2dSegy* sp)
{
    if(m_ampcalculated) return;

    m_minaveamp = std::numeric_limits<float>::max();
    m_maxaveamp = -std::numeric_limits<float>::max();
    m_minabsamp = std::numeric_limits<float>::max();
    m_maxabsamp = -std::numeric_limits<float>::max();
    m_minrmsamp = std::numeric_limits<float>::max();
    m_maxrmsamp = -std::numeric_limits<float>::max();

    TraceHeaders* trhead = nullptr;

    float* trace = new float [m_ns];

    for(int i=0; i<m_ntr; i++){
         getTraceData(i, trace);
         double aveamp = 0;
         double absamp = 0;
         double rmsamp = 0;
         for(int j=0; j< m_ns; j++){
             aveamp += trace[j];
             absamp += abs(trace[j]);
             rmsamp += trace[j]*trace[j];
         }
         aveamp = aveamp/m_ns;
         absamp = absamp/m_ns;
         rmsamp = sqrt(rmsamp/m_ns);
         m_tHeader[i]->ave_amplitude = aveamp;
         m_tHeader[i]->abs_amplitude = absamp;
         m_tHeader[i]->rms_amplitude = rmsamp;

         trhead = sp->getTraceHeader(i);
         trhead->ave_amplitude = aveamp;
         trhead->abs_amplitude = absamp;
         trhead->rms_amplitude = rmsamp;

         if(aveamp > m_maxaveamp ) m_maxaveamp = aveamp;
         if(aveamp < m_minaveamp ) m_minaveamp = aveamp;

         if(absamp > m_maxabsamp ) m_maxabsamp = absamp;
         if(absamp < m_minabsamp ) m_minabsamp = absamp;

         if(rmsamp > m_maxrmsamp ) m_maxrmsamp = rmsamp;
         if(rmsamp < m_minrmsamp ) m_minrmsamp = rmsamp;
    }    
    delete [] trace;    
    m_ampcalculated = true;

    sp->setRangeOfAmplitude(QPointF(m_minaveamp, m_maxaveamp),
                            QPointF(m_minabsamp, m_maxabsamp),
                            QPointF(m_minrmsamp, m_maxrmsamp));
    sp->setAmplitudeCalculated(m_ampcalculated);

    //cout << "maxAve=" << m_maxaveamp << " minAve=" << m_minaveamp << endl;
    //cout << "maxAbs=" << m_maxabsamp << " minAve=" << m_minabsamp << endl;
    //cout << "maxRMS=" << m_maxrmsamp << " minAve=" << m_minrmsamp << endl;
    m_sbar->showMessage(QString("Amplitude was extracted to trace header for file: ")+ QString(m_filename.data()));
}

void Sdp2dSegy::getRangeOfAmplitude(QPointF& aveAmp, QPointF& absAmp, QPointF& rmsAmp)
{
    aveAmp.setX(m_minaveamp);
    aveAmp.setY(m_maxaveamp);
    absAmp.setX(m_minabsamp);
    absAmp.setY(m_maxabsamp);
    rmsAmp.setX(m_minrmsamp);
    rmsAmp.setY(m_maxrmsamp);
}

void Sdp2dSegy::setRangeOfAmplitude(QPointF aveAmp, QPointF absAmp, QPointF rmsAmp)
{
    m_minaveamp = aveAmp.x();
    m_maxaveamp = aveAmp.y();
    m_minabsamp = absAmp.x();
    m_maxabsamp = absAmp.y();
    m_minrmsamp = rmsAmp.x();
    m_maxrmsamp = rmsAmp.y();
}

void Sdp2dSegy::generateIntermediateFile(Sdp2dSegy* segy)
{
    string& name = segy->getSEGYFileName();
    const QString fullName = QString::fromUtf8(name.data(), name.size() );
    QFileInfo fi(fullName);
    QString suname = QDir::homePath() + QString("/.sdp2d/scratch_") + fi.baseName() + QString(".su");

    float* data = new float [m_ns];
    size_t dlen = m_ns * sizeof(float);

    ofstream outfile(suname.toStdString(), ofstream::binary);
    for(int idx=0; idx < m_ntr; idx++ ){
        TraceHeaders* thdr = getTraceHeader(idx);
        getTraceData(idx, data);
        outfile.write((char*)thdr, TRACE_HEADER_SIZE);
        outfile.write((char*)data, dlen);
    }
    outfile.close();
    delete [] data;
    segy->setIntermediateFileReadyForUse();
}

void Sdp2dSegy::removeIntermediateFile()
{
    m_interFile->remove();
}

void Sdp2dSegy::setIntermediateFileReadyForUse()
{
    const QString fullName = QString::fromUtf8(m_filename.data(), m_filename.size() );
    QFileInfo fi(fullName);
    QString suname = QDir::homePath() + QString("/.sdp2d/scratch_") + fi.baseName() + QString(".su");

    m_interFile = new QFile(suname);
    m_interFile->open(QIODevice::ReadWrite);
    m_interFileReady = true;
}
