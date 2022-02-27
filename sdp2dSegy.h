/*******************************************************************************
 * Segy file related functions.
 * Authort: Xin Kefeng
 *
 ******************************************************************************/

#ifndef _SDP2DSEGY_H_
#define _SDP2DSEGY_H_

#include <iostream>
#include <fstream>
#include <string>
#include <QObject>
#include <QPointF>
#include <QList>
#include <QFile>

/**
* the size of each part in a segy file
*/
const int TEXT_HEADER_SIZE = 3200;
const int BINARY_HEADER_SIZE = 400;
const int TRACE_HEADER_SIZE = 240;
const int SEGY_HEADER_SIZE = 3600;

/**
 * ebcdic header
 */
struct EbcdicHeader
{
    unsigned char text_header[TEXT_HEADER_SIZE];
};

/**
 * \brief binary header 400
 */
struct BinaryHeader
{
    int   job_id_number;   					    	// 3201-3204
    int   line_number;		    					// 3205-3208
    int   reel_number;			    				// 3209-3212
    short traces_per_record;						// 3213-3214
    short aux_traces_per_record;					// 3215-3216
    unsigned short sample_data_interval_ms;			// 3217-3218
    unsigned short original_data_interval_ms;		// 3219-3220
    unsigned short samples_per_trace;				// 3221-3222
    unsigned short original_samples_per_trace;		// 3223-3224
    short data_sample_format_code;		    		// 3225-3226
    short CDP_fold;							    	// 3227-3228
    short trace_sorting_code;						// 3229-3230
    short vertical_sum_code;						// 3231-3232
    short sweep_frequency_start_hz;				    // 3233-3234
    short sweep_frequency_end_hz;					// 3235-3236
    short sweep_length_ms;					    	// 3237-3238
    short sweep_type_code;						    // 3239-3240
    short trace_number_of_sweep_channel;			// 3241-3242
    short sweep_trace_taper_length_start_ms;		// 3243-3244
    short sweep_trace_taper_length_end_ms;	    	// 3245-3246
    short taper_type_code;					    	// 3247-3248
    short correlated_data_traces_flag;		    	// 3249-3250
    short binary_gain_recovered_flag;				// 3251-3252
    short amplitude_recovery_method_code;			// 3253-3254
    short measurement_system;						// 3255-3256
    short impulse_signal_polarity;			    	// 3257-3258
    short vibratory_polarity_code;			    	// 3259-3260
    short undefined[170]; 					        // 3261-3600
};

struct TraceHeaders
{
    int   trace_sequence_number_within_line;							//1 1-4
    int   trace_sequence_number_within_reel;							//2 5-8
    int   original_field_record_number;									//3 9-12
    int   trace_sequence_number_within_original_field_record;			//4 13-16
    int   energy_source_point_number;									//5 17-20
    int   cdp_ensemble_number;						    				//6 21-24
    int   trace_sequence_number_within_cdp_ensemble;					//7 25-28

    short trace_identification_code;									//8 29-30
    short number_of_vertically_summed_traces_yielding_this_trace;		//9 31-32
    short number_of_horizontally_stacked_traced_yielding_this_trace;	//10 33-34
    short data_use;														//11 35-36

    int   distance_from_source_point_to_receiver_group;					//12 37-40
    int   receiver_group_elevation;										//13 41-44
    int   surface_elevation_at_source;									//14 45-48
    int   source_depth_below_surface;									//15 49-52
    int   datum_elevation_at_receiver_group;							//16 53-56
    int   datum_elevation_at_source;									//17 57-60
    int   water_depth_at_source;										//18 61-64
    int   water_depth_at_receiver_group;								//19 65-68

    short scalar_for_elevations_and_depths;								//20 69-70
    short scalar_for_coordinates;										//21 71-72

    int   x_source_coordinate;											//22 73-76
    int   y_source_coordinate;											//23 77-80
    int   x_receiver_group_coordinate;									//24 81-84
    int   y_receiver_group_coordinate;									//25 85-88

    short coordinate_units;												//26 89-90
    short weathering_velocity;							    			//27 91-92
    short subweathering_velocity;										//28 93-94
    short uphole_time_at_source;										//29 95-96
    short uphole_time_at_group;											//30 97-98
    short source_static_correction;										//31 99-100
    short group_static_correction;							    		//32 101-102
    short total_static_applied;											//33 103-104
    short lag_time_a;													//34 105-106
    short lag_time_b;													//35 107-108
    short delay_according_time;											//36 109-110
    short mute_time_start;										    	//37 111-112
    short mute_time_end;												//38 113-114
    unsigned short samples_in_this_trace;								//39 115-116
    unsigned short sample_intervall;									//40 117-118
    short gain_type_instruments;										//41 119-120
    short igc;														   	//42 121-122
    short igi;													    	//43 123-124
    short corr;															//44 125-126
    short sfs;														   	//45 127-128
    short sfe;														   	//46 129-130
    short slen;															//47 131-132
    short styp;															//48 133-134
    short stas;															//49 135-136
    short stae;															//50 137-138
    short tatyp;														//51 139-140
    short afilf;														//52 141-142
    short afils;														//53 143-144
    short nofilf;														//54 145-146
    short nofils;														//55 147-148
    short lcf;						    								//56 149-150
    short hcf;							    							//57 151-152
    short lcs;								    						//58 153-154
    short hcs;									    					//59 155-156
    short year;															//60 157-158
    short day;										    				//61 159-160
    short hour;															//62 161-162
    short minute;														//63 163-164
    short sec;												    		//64 165-166
    short timbas;														//65 167-168
    short trwf;															//66 169-170
    short grnors;														//67 171-172
    short grnofr;														//68 173-174
    short grnlof;														//69 175-176
    short gaps;															//70 177-178
    short otrav;														//71 179-180

    int   x_cdp_coordinate;                                             //72 181-184
    int   y_cdp_coordinate;                                             //73 185-188
    unsigned int num_in_line;                                           //74 189-192
    unsigned int num_cross_line;                                        //75 193-196
    unsigned int num_shot_point;										//76 197-200

    float ave_amplitude;                                                //77 201-204
    float abs_amplitude;                                                //78 205-208
    float rms_amplitude;                                                //79 209-212
    unsigned short identifier_trace;                                    //81 213-214
    unsigned short scalar_trace;                                        //82 215-216
    unsigned short source_orientation;                                  //83 217-218
    unsigned short source_energy_direction[3];                          //84 219-224
    unsigned short source_measurement[3];                               //85 225-230
    unsigned short unit_source_measurement;                             //86 231-232

    unsigned char  un_define[8];                                        //87 233-240

};

QT_BEGIN_NAMESPACE
class QStatusBar;
QT_END_NAMESPACE

class SeismicData2D;

/**
 * \brief all function in this Sdp2dSegy class
 */

class Sdp2dSegy : public QObject
{
    Q_OBJECT

public:
    Sdp2dSegy();
    Sdp2dSegy(const std::string& segy_file_name, QStatusBar *sbar);
    Sdp2dSegy(Sdp2dSegy* sp, QStatusBar *sbar=nullptr);
    ~Sdp2dSegy();

    std::string& getSEGYFileName(void)  { return m_filename; }
    int getNumberOfTraces(void) const  { return m_ntr; }
    int getDataType(void) const { return m_dtype; }
    int getSamplesPerTrace(void) const  { return m_ns; }
    int getTimeSampleRateInUs(void) const { return m_dt; }

    bool getAmplitudeCalculated(void) const { return m_ampcalculated; }
    void setAmplitudeCalculated(bool flag) { m_ampcalculated = flag; }

    void setTraceDataSwapbytes(int swapbytes);
    int getTraceDataSwapbytes();

    float getSpecificData(const int trace_idx,  const int sample_idx);//one sample
    bool getTraceData(const int trace_idx, float *trace);

    EbcdicHeader* get3200TextHeader(void) { return m_eHeader; }
    BinaryHeader* get400BinaryHeader(void) { return m_bHeader; }
    TraceHeaders *getTraceHeader(const int trace_idx);
    TraceHeaders** getAllTracesHeader(void);

    void getAllCDPXY(QList<QPointF>&);

    void keepSEGYTraceHeader(int itr, TraceHeaders*);
    void readTraceHeader(const int trace_idx);

    void calculateAmplitudePutToHeader(Sdp2dSegy* sp);
    bool getAmpCalculateStates() const { return m_ampcalculated; }

    void getRangeOfAmplitude(QPointF& aveAmp, QPointF& absAmp, QPointF& rmsAmp);
    void setRangeOfAmplitude(QPointF aveAmp, QPointF absAmp, QPointF rmsAmp);

    void generateIntermediateFile(Sdp2dSegy* sp);
    void removeIntermediateFile();
    bool getTraceDataFromIntermFile(const int trace_idx, float *trace);
    bool writeTraceDataToIntermFile(const int trace_idx, float *trace);

private:
    std::string m_filename;
    std::ifstream m_segyin;
    QFile* m_interFile;
    bool m_interFileReady;

    QStatusBar *m_sbar;
    EbcdicHeader  *m_eHeader;
    BinaryHeader  *m_bHeader;
    TraceHeaders **m_tHeader;

    int m_ns;
    int m_dt;
    int m_ntr;
    int m_dtype;

    bool m_headswapbytes;
    bool m_dataswapbytes;

    bool m_ampcalculated;

    float m_minaveamp;
    float m_maxaveamp;
    float m_minabsamp;
    float m_maxabsamp;
    float m_minrmsamp;
    float m_maxrmsamp;

    enum DataType { IBM = 1, IEEE = 5};

    bool checkEndian(BinaryHeader *bhdr);

    void calNumberOfTraces(void);
    void readTextHeader(void);
    void readBinaryHeader(void);
    void allocateTraceHeaders(void);

    void readAllTracesHeader(void);

    void swapBinaryHeader(BinaryHeader *bhead);
    void swapTraceHeader(TraceHeaders *thead);

    void setIntermediateFileReadyForUse();

signals:
    //void resultReady(void);
};
#endif //!_SDP2DSEGY_H_
