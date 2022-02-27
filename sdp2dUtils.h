#ifndef SDP2DUtils_h
#define SDP2DUtils_h

#include <unistd.h>
#include <string>
#include <sstream>
#include "sdp2dSegy.h"

using namespace std;

#define NINT(x) ((int)((x)>0.0?(x)+0.5:(x)-0.5))

template< class type>
inline string to_string( const type & value)
{
     ostringstream streamOut;
     streamOut << value;
     return streamOut.str();
}

/** This class is used for some useful functions.
 */
class Sdp2dUtils {

public:
     static bool dirExist (const string& fullname);
     static void rmDir(const char* dir);
     static void rmDir(string dir);

     static bool fileExist (const string& filename);

     /*  allocate and free 3-d array of space */
     static  float *** alloc3float(size_t n1, size_t n2, size_t n3);
     static  void free3float(float ***p);

     static  double *** alloc3double(size_t n1, size_t n2, size_t n3);
     static  void free3double(double ***p);

    /*   // allocate and free 2-d array of space */
     static  float **alloc2float(size_t n1, size_t n2);
     static  void free2float(float **p);

     static  double **alloc2double(size_t n1, size_t n2);
     static  void free2double(double **p);

     static  int **alloc2int(size_t n1, size_t n2);
     static  void free2int(int **p);

     /* compare two string with case insensitive */
     /* from The C++ programming language, 3rd edition, p. 591 */
     static int cmp_nocase(const string& s, const string& s2);

     /* check if the user has the permission to write to the disk */
     static void checkOutputFileStatus(const char* filename,  int t_owneronly);

     static void intlin (int nin, float* xin, float* yin, float yinl, float yinr, int nout, float* xout, float* yout);
     static void xindex (int nx, float* ax, float x, int *index);

     static bool integerSwapByte(void *data, int size);
     static void swapByte2(unsigned short *data);
     static void swapByte4(unsigned int *data);
     static void ibm2Ieee(float* input, int swap);
     static void ebcdic2Ascii(EbcdicHeader *ehead);
     static void ascii2Ebcdic(EbcdicHeader *ehead);

     static unsigned char *make_rgbbuf(float * display_data, int width, int height, int color, float perc);
     static void qkpart (float a[], int p, int q, int *j, int *k);
     static void qkinss (float a[], int p, int q);
     static void qksort (int n, float a[]);
     static void qkfind (int m, int n, float a[]);

     static void ints8r (int nxin, float dxin, float fxin, float yin[], float yinl, float yinr, int nxout, float xout[], float yout[]);
     static void intt8r (int ntable, float table[][8], int nxin, float dxin, float fxin, float yin[], float yinl, float yinr, int nxout, float xout[], float yout[]);
     static void mksinc (float d, int lsinc, float sinc[]);
     static void stoepd (int n, double r[], double g[], double f[], double a[]);
     static void yxtoxy (int nx, float dx, float fx, float y[], int ny, float dy, float fy, float xylo, float xyhi, float x[]);

private:
     static void *** alloc3 (size_t n1, size_t n2, size_t n3, size_t size);
     /* free a 3-d array */
     static void free3 (void ***p);

     static void **alloc2 (size_t n1, size_t n2, size_t size);
     static void free2 (void **p);

     static double dsinc (double x);

};
#endif
