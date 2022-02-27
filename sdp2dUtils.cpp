#include "sdp2dUtils.h"

#include <iostream>
#include <sys/stat.h>

#include <stdlib.h>
#include <math.h>

using namespace std;

bool Sdp2dUtils::dirExist (const string & fullname)
{
    struct stat buf;

    if ( stat(fullname.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode) )
    {
        return true;
    }

    return false;
}

void Sdp2dUtils::rmDir(string dir)
{
    char command[4096];
    if(dirExist(dir))
    {
        sprintf(command,"rm -rf %s",dir.c_str());
        system(command);
    }
}

void Sdp2dUtils::rmDir(const char* dir)
{
    char command[4096];
    if(dirExist(dir))
    {
        sprintf(command,"rm -rf %s",dir);
        system(command);
    }
}

bool Sdp2dUtils::fileExist (const string& filename)
{
    struct stat buf;

    if ( !stat(filename.c_str(), &buf) )  {
        if (!(S_ISREG(buf.st_mode)))  {
            cout<<"File "<<filename<<" exists, but is not a regular file."<<endl;
            return false;
        }
        return true;
    } else {
        return false;
    }

}


// if t_owneronly > 0, then only allow user to do certain oprerations.
void Sdp2dUtils::checkOutputFileStatus(const char* filename, int t_owneronly)
{
    struct stat buf;
    int error = stat(filename, &buf);

    if(error == -1)              // file doesn't exist or can not be searched
    {
        char tmpfile[4096];
        strcpy(tmpfile, filename);

        char* loc = strrchr(tmpfile, '/');
        if(loc == 0)             // no directory
        {
            error = stat(".", & buf);
            if( error==0 )       // no error
            {
                if( S_ISDIR(buf.st_mode) && (buf.st_mode & S_IWUSR) )
                {
                    //cerr<<filename<< " is not a directory"<<endl;
                    return;
                }
            }

            cerr<<" ERROR ================================ ERROR "<<endl;
            cerr<<" The current directory doesn't have write permission. file name: "<<filename<<endl;
            cerr<<" ERROR ================================ ERROR "<<endl;

            exit(1);
        } else { // has subdirectory

            *loc = '\0';
            error = stat(tmpfile, &buf);

            if(error==0)         // it has this directory
            {
                if( S_ISDIR(buf.st_mode) && (buf.st_mode & S_IWUSR) )
                {
                    cerr<<filename <<" has write permission."<<endl;
                    return;
                }
            }
            cerr<<" The directory doesn't have the write permission. file name: "<<filename<<endl;

            exit(1);
        }
    } else { // file exist
        uid_t myuid = getuid();

        bool t_reporterr = false;
        // not a regular file
        if(!(S_ISREG(buf.st_mode)) ||
                // only user allowed, but I am the owner
                ((t_owneronly > 0) && (buf.st_uid  != myuid)) )
        {
            t_reporterr = true;
        }

        if(t_reporterr)          // file already exist but have no write permission
        {
            cout<<" === can not write file name: "<<filename<<endl;
            exit(1);
        }
    }
}


/* allocate a 2-d array of ints */
int** Sdp2dUtils::alloc2int(size_t n1, size_t n2)
{
    return (int**)alloc2(n1,n2,sizeof(int));
}


/* free a 2-d array of ints */
void Sdp2dUtils::free2int(int **p)
{
    free2((void**)p);
}


/* allocate a 2-d array of floats */
float** Sdp2dUtils::alloc2float(size_t n1, size_t n2)
{
    return (float**)alloc2(n1,n2,sizeof(float));
}


/* free a 2-d array of floats */
void Sdp2dUtils::free2float(float **p)
{
    free2((void**)p);
}


/* allocate a 2-d array of doubles */
double** Sdp2dUtils::alloc2double(size_t n1, size_t n2)
{
    return (double**)alloc2(n1,n2,sizeof(double));
}


/* free a 2-d array of doubles */
void Sdp2dUtils::free2double(double **p)
{
    free2((void**)p);
}


/* allocate a 2-d array */
void** Sdp2dUtils::alloc2 (size_t n1, size_t n2, size_t size)
{
    size_t i2;
    void **p;

    if ((p=(void**)malloc(n2*sizeof(void*)))==NULL)
        return NULL;
    if ((p[0]=(void*)malloc(n2*n1*size))==NULL)
    {
        free(p);
        return NULL;
    }
    for (i2=0; i2<n2; i2++)
        p[i2] = (char*)p[0]+size*n1*i2;
    return p;
}


/* free a 2-d array */
void Sdp2dUtils::free2 (void **p)
{
    free(p[0]);
    free(p);
}


/* allocate a 3-d array of floats */
float*** Sdp2dUtils::alloc3float(size_t n1, size_t n2, size_t n3)
{
    return (float***)alloc3(n1,n2,n3,sizeof(float));
}


/* free a 3-d array of floats */
void Sdp2dUtils::free3float(float ***p)
{
    free3((void***)p);
}


/* allocate a 3-d array of doubles */
double*** Sdp2dUtils::alloc3double(size_t n1, size_t n2, size_t n3)
{
    return (double***)alloc3(n1,n2,n3,sizeof(double));
}


/* free a 3-d array of floats */
void Sdp2dUtils::free3double(double ***p)
{
    free3((void***)p);
}


//
void*** Sdp2dUtils::alloc3 (size_t n1, size_t n2, size_t n3, size_t size)
{
    size_t i3,i2;
    void ***p;

    if ((p=(void***)malloc(n3*sizeof(void**)))==NULL)
        return NULL;
    if ((p[0]=(void**)malloc(n3*n2*sizeof(void*)))==NULL)
    {
        cout<<" 2 out of memory in alloc3."<<endl;
        free(p);
        return NULL;
    }
    if ((p[0][0]=(void*)malloc(n3*n2*n1*size))==NULL)
    {
        cout<<" 3 out of memory in alloc3."<<endl;
        free(p[0]);
        free(p);
        return NULL;
    }
    for (i3=0; i3<n3; i3++)
    {
        p[i3] = p[0]+n2*i3;
        for (i2=0; i2<n2; i2++)
            p[i3][i2] = (char*)p[0][0]+size*n1*(i2+n2*i3);
    }
    return p;
}


/* free a 3-d array */
void Sdp2dUtils::free3 (void ***p)
{
    free(p[0][0]);
    free(p[0]);
    free(p);
}


// compare two string with case insensitive
// from The C++ programming language, 3rd edition, p. 591
int Sdp2dUtils::cmp_nocase(const string& s, const string& s2)
{
    string::const_iterator p = s.begin();
    string::const_iterator p2 = s2.begin();
    //
    while(p!=s.end() && p2!=s2.end())
    {
        if(toupper(*p) != toupper(*p2)) return (toupper(*p)<toupper(*p2))?-1:1;
        ++p;
        ++p2;
    }

    return s2.size()-s.size();
}


void Sdp2dUtils::intlin (int nin, float* xin, float* yin, float yinl, float yinr, int nout, float* xout, float* yout)
/*****************************************************************************
evaluate y(x) via linear interpolation of y(x[0]), y(x[1]), ...
******************************************************************************
Input:
nin		length of xin and yin arrays
xin		array[nin] of monotonically increasing or decreasing x values
yin		array[nin] of input y(x) values
yinl		value used to extraplate y(x) to left of input yin values
yinr		value used to extraplate y(x) to right of input yin values
nout		length of xout and yout arrays
xout		array[nout] of x values at which to evaluate y(x)

Output:
yout		array[nout] of linearly interpolated y(x) values
******************************************************************************
Notes:
xin values must be monotonically increasing or decreasing.

Extrapolation of the function y(x) for xout values outside the range
spanned by the xin values in performed as follows:

    For monotonically increasing xin values,
        yout=yinl if xout<xin[0], and yout=yinr if xout>xin[nin-1].

    For monotonically decreasing xin values,
        yout=yinl if xout>xin[0], and yout=yinr if xout<xin[nin-1].

If nin==1, then the monotonically increasing case is used.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 06/02/89
*****************************************************************************/
{
    int idx=0;
    int jout;
    float x;

    /* if input x values are monotonically increasing, then */
    if (xin[0]<=xin[nin-1]) {
        for (jout=0; jout<nout; jout++) {
            x = xout[jout];
            if (x<xin[0])
                yout[jout] = yinl;
            else if (x>xin[nin-1])
                yout[jout] = yinr;
            else if (x==xin[nin-1] || nin==1)
                yout[jout] = yin[nin-1];
            else {
                xindex(nin,xin,x,&idx);
                yout[jout] = yin[idx]+(x-xin[idx])
                        *(yin[idx+1]-yin[idx])
                        /(xin[idx+1]-xin[idx]);
            }
        }

        /* else, if input x values are monotonically decreasing, then */
    } else {
        for (jout=0; jout<nout; jout++) {
            x = xout[jout];
            if (x>xin[0])
                yout[jout] = yinl;
            else if (x<xin[nin-1])
                yout[jout] = yinr;
            else if (x==xin[nin-1] || nin==1)
                yout[jout] = yin[nin-1];
            else {
                xindex(nin,xin,x,&idx);
                yout[jout] = yin[idx]+(x-xin[idx])
                        *(yin[idx+1]-yin[idx])
                        /(xin[idx+1]-xin[idx]);
            }
        }
    }

}

void Sdp2dUtils::xindex (int nx, float* ax, float x, int *index)
/*****************************************************************************
determine index of x with respect to an array of x values
******************************************************************************
Input:
nx		number of x values in array ax
ax		array[nx] of monotonically increasing or decreasing x values
x		the value for which index is to be determined
index		index determined previously (used to begin search)

Output:
index		for monotonically increasing ax values, the largest index
        for which ax[index]<=x, except index=0 if ax[0]>x;
        for monotonically decreasing ax values, the largest index
        for which ax[index]>=x, except index=0 if ax[0]<x
******************************************************************************
Notes:
This function is designed to be particularly efficient when called
repeatedly for slightly changing x values; in such cases, the index
returned from one call should be used in the next.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 12/25/89
*****************************************************************************/
{
    int lower,upper,middle,step;

    /* initialize lower and upper indices and step */
    lower = *index;
    if (lower<0) lower = 0;
    if (lower>=nx) lower = nx-1;
    upper = lower+1;
    step = 1;

    /* if x values increasing */
    if (ax[nx-1]>ax[0]) {

        /* find indices such that ax[lower] <= x < ax[upper] */
        while (lower>0 && ax[lower]>x) {
            upper = lower;
            lower -= step;
            step += step;
        }
        if (lower<0) lower = 0;
        while (upper<nx && ax[upper]<=x) {
            lower = upper;
            upper += step;
            step += step;
        }
        if (upper>nx) upper = nx;

        /* find index via bisection */
        while ((middle=(lower+upper)>>1)!=lower) {
            if (x>=ax[middle])
                lower = middle;
            else
                upper = middle;
        }

        /* else, if not increasing */
    } else {

        /* find indices such that ax[lower] >= x > ax[upper] */
        while (lower>0 && ax[lower]<x) {
            upper = lower;
            lower -= step;
            step += step;
        }
        if (lower<0) lower = 0;
        while (upper<nx && ax[upper]>=x) {
            lower = upper;
            upper += step;
            step += step;
        }
        if (upper>nx) upper = nx;

        /* find index via bisection */
        while ((middle=(lower+upper)>>1)!=lower) {
            if (x<=ax[middle])
                lower = middle;
            else
                upper = middle;
        }
    }

    /* return lower index */
    *index = lower;

}



/**
 * convert 2 or 4 byte for the header or data
 */
bool Sdp2dUtils::integerSwapByte(void* data, int size)
{
    switch ((size))
    {
        case 2:
            swapByte2(static_cast<unsigned short *>(data));
            return true;
        case 4:
            swapByte4(static_cast<unsigned int *>(data));
            return true;
        default:
            return false;
    }
}

/**
 * change two bytes(not the bit) 1 2 to 2 1
 */
void Sdp2dUtils::swapByte2(unsigned short *tni2)
{
    *tni2=(((*tni2>>8)&0xff) | ((*tni2&0xff)<<8));
}

/**
 * change four bytes(not the bit) 1 2 3 4 to 4 3 2 1
 */
void Sdp2dUtils::swapByte4(unsigned int *tni4)
{
    *tni4=(((*tni4>>24)&0xff) | ((*tni4&0xff)<<24) |
            ((*tni4>>8)&0xff00) | ((*tni4&0xff00)<<8));
}


/**
 * convert ONE IBM float to IEEE float
 * input: one float value
 * swap: change byte order or not
 */
void Sdp2dUtils::ibm2Ieee(float* input, int swap)
{
    unsigned char  *cbuf, expp, tem, sign;
    unsigned int  *umantis, expll;
    long *mantis;
    int  shift;

    cbuf = (unsigned char*)&input[0];        /* assign address of input to char array */
    umantis = (unsigned int*)&input[0];     /* two differnt points to the same spot  */
    mantis = (long*)&input[0];     /* signned & unsigned                    */

    if (swap)
    {
        /* now byte reverce for PC use if swap true */
        tem = cbuf[0]; cbuf[0] = cbuf[3]; cbuf[3] = tem;
        tem = cbuf[2]; cbuf[2] = cbuf[1]; cbuf[1] = tem;
    }

    /* start extraction information from number */

    expp = *mantis >> 24;     /* get expo fro upper byte      */
    *mantis = (*mantis) << 8; /* shift off upper byte         */
    shift = 1;              /* set a counter to 1           */
    while (*mantis>0 && shift<23) /* start of shifting data*/
    {
        *mantis = *mantis << 1;
        shift++;
    } /* shift until a 1 in msb */

    *mantis = *mantis << 1; /* need one more shift to get implied one bit */
    sign = expp & 0x80;   /* set sign to msb of exponent            */
    expp = expp & 0x7F;   /* kill sign bit                          */

    if (expp != 0)        /* don't do anymore if zero exponent       */
    {
        expp = expp - 64;   /* compute what shift was (old exponent)*/
        *umantis = *umantis >> 9; /* MOST IMPORTANT an UNSIGNED shift back down */
        expll = 0x7F + (expp * 4 - shift); /* add in excess 172 */

        /* now mantissa is correctly aligned, now create the other two pairs */
        /* needed the extended sign word and the exponent word               */

        expll = expll << 23;        /* shift exponent up */

        /* combine them into a floating point IEEE format !     */

        if (sign) *umantis = expll | *mantis | 0x80000000;
        else     *umantis = expll | *mantis; /* set or don't set sign bit */
    }
}


void Sdp2dUtils::ascii2Ebcdic(EbcdicHeader* ehdr)
{
    static unsigned char a2e[256] = {
    0, 1, 2, 3, 55, 45, 46, 47, 22, 5, 37, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 60, 61, 50, 38, 24, 25, 63, 39, 28, 29, 30, 31,
    64, 79,127,123, 91,108, 80,125, 77, 93, 92, 78,107, 96, 75, 97,
    240,241,242,243,244,245,246,247,248,249,122, 94, 76,126,110,111,
    124,193,194,195,196,197,198,199,200,201,209,210,211,212,213,214,
    215,216,217,226,227,228,229,230,231,232,233, 74,224, 90, 95,109,
    121,129,130,131,132,133,134,135,136,137,145,146,147,148,149,150,
    151,152,153,162,163,164,165,166,167,168,169,192,106,208,161, 7,
    32, 33, 34, 35, 36, 21, 6, 23, 40, 41, 42, 43, 44, 9, 10, 27,
    48, 49, 26, 51, 52, 53, 54, 8, 56, 57, 58, 59, 4, 20, 62,225,
    65, 66, 67, 68, 69, 70, 71, 72, 73, 81, 82, 83, 84, 85, 86, 87,
    88, 89, 98, 99,100,101,102,103,104,105,112,113,114,115,116,117,
    118,119,120,128,138,139,140,141,142,143,144,154,155,156,157,158,
    159,160,170,171,172,173,174,175,176,177,178,179,180,181,182,183,
    184,185,186,187,188,189,190,191,202,203,204,205,206,207,218,219,
    220,221,222,223,234,235,236,237,238,239,250,251,252,253,254,255
    };

    int i = 0;
    for (unsigned char& c : (*ehdr).text_header)
    {
        c = a2e[c];
        //cout<< c;
        i++;
        //if ( i%80 == 0) cout << endl;
    }
    //cout<<endl;
}

void Sdp2dUtils::ebcdic2Ascii(EbcdicHeader* ehdr)
{
    /*
    static char ebcdic_to_ascii_table[256] =
    {
        0x00,0x01,0x02,0x03,0x00,0x09,0x00,0x7f,
        0x00,0x00,0x00,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x00,0x00,0x08,0x00,
        0x18,0x19,0x00,0x00,0x1c,0x1d,0x1e,0x1f,
        0x00,0x00,0x00,0x00,0x00,0x0a,0x17,0x1b,
        0x00,0x00,0x00,0x00,0x00,0x05,0x06,0x07,
        0x00,0x00,0x16,0x00,0x00,0x00,0x00,0x04,
        0x00,0x00,0x00,0x00,0x14,0x15,0x00,0x1a,
        0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x2e,0x3c,0x28,0x2b,0x7c,
        0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x21,0x24,0x2a,0x29,0x3b,0x5e,
        0x2d,0x2f,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x2c,0x25,0x5f,0x3e,0x3f,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x60,0x3a,0x23,0x40,0x27,0x3d,0x22,
        0x00,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
        0x68,0x69,0x00,0x7b,0x00,0x00,0x00,0x00,
        0x00,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,
        0x71,0x72,0x00,0x7d,0x00,0x00,0x00,0x00,
        0x00,0x7e,0x73,0x74,0x75,0x76,0x77,0x78,
        0x79,0x7a,0x00,0x00,0x00,0x5b,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x5d,0x00,0x00,
        0x7b,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
        0x48,0x49,0x00,0x00,0x00,0x00,0x00,0x00,
        0x7d,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,0x50,
        0x51,0x52,0x00,0x00,0x00,0x00,0x00,0x00,
        0x5c,0x00,0x53,0x54,0x55,0x56,0x57,0x58,
        0x59,0x5a,0x00,0x00,0x00,0x00,0x00,0x00,
        0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
        0x38,0x39,0x00,0x00,0x00,0x00,0x00,0x00,
    };
*/
    static unsigned char e2a[256] = {
    0, 1, 2, 3,156, 9,134,127,151,141,142, 11, 12, 13, 14, 15,
    16, 17, 18, 19,157,133, 8,135, 24, 25,146,143, 28, 29, 30, 31,
    128,129,130,131,132, 10, 23, 27,136,137,138,139,140, 5, 6, 7,
    144,145, 22,147,148,149,150, 4,152,153,154,155, 20, 21,158, 26,
    32,160,161,162,163,164,165,166,167,168, 91, 46, 60, 40, 43, 33,
    38,169,170,171,172,173,174,175,176,177, 93, 36, 42, 41, 59, 94,
    45, 47,178,179,180,181,182,183,184,185,124, 44, 37, 95, 62, 63,
    186,187,188,189,190,191,192,193,194, 96, 58, 35, 64, 39, 61, 34,
    195, 97, 98, 99,100,101,102,103,104,105,196,197,198,199,200,201,
    202,106,107,108,109,110,111,112,113,114,203,204,205,206,207,208,
    209,126,115,116,117,118,119,120,121,122,210,211,212,213,214,215,
    216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,
    123, 65, 66, 67, 68, 69, 70, 71, 72, 73,232,233,234,235,236,237,
    125, 74, 75, 76, 77, 78, 79, 80, 81, 82,238,239,240,241,242,243,
    92,159, 83, 84, 85, 86, 87, 88, 89, 90,244,245,246,247,248,249,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57,250,251,252,253,254,255
    };

    int i = 0;
    for (unsigned char& c : (*ehdr).text_header)
    {
        c = e2a[c];
        //cout<< c;
        i++;
        //if ( i%80 == 0) cout << endl;
    }
    //cout<<endl;
}


unsigned char *Sdp2dUtils::make_rgbbuf (float * display_data, int width, int height, int color, float perc)
{
	int nz;
	int i, i1, i2;
    //float vmin, vmax, zi;
    float zi;
	float zoffset, zscale;
	unsigned char *rgbbuf;
    //float r, g, b, h, s, v;
	float *temp;
	float bperc, wperc;
	float bclip, wclip;


# define RGB_BLACK      {0x00, 0x00, 0x00}
# define RGB_WHITE      {0xff, 0xff, 0xff}
# define RGB_GRAY       {0x80, 0x80, 0x80}

# define RGB_ORANGE     {0xff, 0x80, 0x00}

# define RGB_RED        {0xe0, 0x00, 0x50}
# define RGB_BLUE       {0x00, 0x40, 0xc0}
# define RGB_GREEN      {0x06, 0x5b, 0x3f}
# define RGB_BROWN      {0x72, 0x5b, 0x3f}
# define RGB_REDBROWN   {0xa0, 0x40, 0x00}

# define RGB_GRAY2      {0xb0, 0xb0, 0xb0}

# define RGB_LGRAY      {0xf0, 0xf0, 0xf0}
# define RGB_LBLUE      {0x55, 0x9c, 0xe0}
# define RGB_YELLOW     {0xd0, 0xb0, 0x20}


	float c_rgb[][3][3] = {
		{RGB_BLACK, RGB_GRAY, RGB_WHITE},
		{RGB_BLUE, RGB_WHITE, RGB_RED},
		{RGB_RED, RGB_YELLOW, RGB_WHITE},
		{RGB_RED, RGB_WHITE, RGB_LBLUE},
		{RGB_BLUE, RGB_YELLOW, RGB_RED},
		{RGB_REDBROWN, RGB_WHITE, RGB_GREEN},
		{RGB_BROWN, RGB_YELLOW, RGB_RED},
		{RGB_REDBROWN, RGB_WHITE, RGB_RED},
		{RGB_RED, RGB_LGRAY, RGB_BLUE},
		{RGB_RED, RGB_WHITE, RGB_GREEN},
		{RGB_RED, RGB_YELLOW, RGB_GREEN},
		{RGB_RED, RGB_YELLOW, RGB_BLUE},
		{RGB_BROWN, RGB_YELLOW, RGB_BLUE},
		{RGB_BROWN, RGB_YELLOW, RGB_GREEN},
		{RGB_BROWN, RGB_WHITE, RGB_GREEN},
		{RGB_REDBROWN, RGB_YELLOW, RGB_BLUE},
		{RGB_REDBROWN, RGB_LGRAY, RGB_BLUE},
		{RGB_REDBROWN, RGB_LGRAY, RGB_GREEN},
		{RGB_ORANGE, RGB_LGRAY, RGB_BLUE},
		{RGB_ORANGE, RGB_WHITE, RGB_GREEN},
		{RGB_ORANGE, RGB_LGRAY, RGB_GRAY},
		{RGB_BLUE, RGB_WHITE, RGB_RED}
	};

	nz = width * height;

    temp = (float *)malloc(sizeof (float) * nz);

	for (i = 0; i < nz; i++)
	{
		temp[i] = display_data[i];

	}

	bperc = perc;
	i = (nz * bperc / 100.0);
    if (i < 0) 	    i = 0;
    if (i > nz - 1) i = nz - 1;
	qkfind (i, nz, temp);
	bclip = temp[i];

	wperc = 100.0 - perc;
	i = (nz * wperc / 100.0);
    if (i < 0) 	    i = 0;
    if (i > nz - 1) i = nz - 1;
	qkfind (i, nz, temp);
	wclip = temp[i];
    free (temp);
    cout << "wclip="<<wclip<< " bclip=" << bclip<< endl;
	color = color - 1;
    rgbbuf = (unsigned char *)calloc (sizeof (unsigned char),  nz*3);

	zscale = (wclip != bclip) ? 255.0 / (wclip - bclip) : 1.0e10;

	zoffset = -bclip * zscale;

    //cout << "zscale=" << zscale << " zoffset=" << zoffset << endl;

	for (i1 = 0; i1 < height; i1++)
	{
		for (i2 = 0; i2 < width; i2++)
		{
            zi = zoffset + display_data[i1 + i2 * height] * zscale;
            //cout << " i1="<< i1<< " i2="<< i2 << " zi="<< zi << endl;
            if (zi < 0.0)   zi = 0.0;
            if (zi > 255.0) zi = 255.0;

			if (zi < 128)
			{
				rgbbuf[(i1 * width + i2) * 3] =
					c_rgb[color][0][0] +
					(c_rgb[color][1][0] -
					 c_rgb[color][0][0]) * zi / 128.0;
				rgbbuf[(i1 * width + i2) * 3 + 1] =
					c_rgb[color][0][1] +
					(c_rgb[color][1][1] -
					 c_rgb[color][0][1]) * zi / 128.0;
				rgbbuf[(i1 * width + i2) * 3 + 2] =
					c_rgb[color][0][2] +
					(c_rgb[color][1][2] -
					 c_rgb[color][0][2]) * zi / 128.0;
			}
			else
			{
				rgbbuf[(i1 * width + i2) * 3] =
					c_rgb[color][1][0] +
					(c_rgb[color][2][0] -
					 c_rgb[color][1][0]) *
					(zi - 128.0) / 128.0;
				rgbbuf[(i1 * width + i2) * 3 + 1] =
					c_rgb[color][1][1] +
					(c_rgb[color][2][1] -
					 c_rgb[color][1][1]) *
					(zi - 128.0) / 128.0;
				rgbbuf[(i1 * width + i2) * 3 + 2] =
					c_rgb[color][1][2] +
					(c_rgb[color][2][2] -
					 c_rgb[color][1][2]) *
					(zi - 128.0) / 128.0;
			}
		}
	}
	return (rgbbuf);
}

#define NSTACK 50	/* maximum sort length is 2^NSTACK */
#define NSMALL 7	/* size of array for which insertion sort is fast */
#define FM 7875		/* constants used to generate random pivots */
#define FA 211
#define FC 1663

void Sdp2dUtils::qkpart (float a[], int p, int q, int *j, int *k)
/*****************************************************************************
quicksort partition (FOR INTERNAL USE ONLY):
Take the value x of a random element from the subarray a[p:q] of
a[0:n-1] and rearrange the elements in this subarray in such a way
that there exist integers j and k with the following properties:
  p <= j < k <= q, provided that p < q
  a[l] <= x,  for p <= l <= j
  a[l] == x,  for j < l < k
  a[l] >= x,  for k <= l <= q
Note that this effectively partitions the subarray with bounds
[p:q] into lower and upper subarrays with bounds [p:j] and [k:q].
******************************************************************************
Input:
a		array[p:q] to be rearranged
p		lower bound of subarray; must be less than q
q		upper bound of subarray; must be greater then p

Output:
a		array[p:q] rearranged
j		upper bound of lower output subarray
k		lower bound of upper output subarray
******************************************************************************
Notes:
This function is adapted from procedure partition by
Hoare, C.A.R., 1961, Communications of the ACM, v. 4, p. 321.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 01/13/89
*****************************************************************************/
{
    int pivot,left,right;
    float apivot,temp;
    static long int seed=0L;

    /* choose random pivot element between p and q, inclusive */
    seed = (seed*FA+FC)%FM;
    pivot = p+(q-p)*(float)seed/(float)FM;
    if (pivot<p) pivot = p;
    if (pivot>q) pivot = q;
    apivot = a[pivot];

    /* initialize left and right pointers and loop until break */
    for (left=p,right=q;;) {
        /*
         * increment left pointer until either
         * (1) an element greater than the pivot element is found, or
         * (2) the upper bound of the input subarray is reached
         */
        while (a[left]<=apivot && left<q) left++;

        /*
         * decrement right pointer until either
         * (1) an element less than the pivot element is found, or
         * (2) the lower bound of the input subarray is reached
         */
        while (a[right]>=apivot && right>p) right--;

        /* if left pointer is still to the left of right pointer */
        if (left<right) {
            /* exchange left and right elements */
            temp = a[left];
            a[left++] = a[right];
            a[right--] = temp;
        }
        /* else, if pointers are equal or have crossed, break */
        else break;
    }
    /* if left pointer has not crossed pivot */
    if (left<pivot) {

        /* exchange elements at left and pivot */
        temp = a[left];
        a[left++] = a[pivot];
        a[pivot] = temp;
    }
    /* else, if right pointer has not crossed pivot */
    else if (pivot<right) {

        /* exchange elements at pivot and right */
        temp = a[right];
        a[right--] = a[pivot];
        a[pivot] = temp;
    }
    /* left and right pointers have now crossed; set output bounds */
    *j = right;
    *k = left;
}

void Sdp2dUtils::qkinss (float a[], int p, int q)
/*****************************************************************************
quicksort insertion sort (FOR INTERNAL USE ONLY):
Sort a subarray bounded by p and q so that
a[p] <= a[p+1] <= ... <= a[q]
******************************************************************************
Input:
a		subarray[p:q] containing elements to be sorted
p		lower bound of subarray; must be less than q
q		upper bound of subarray; must be greater then p

Output:
a		subarray[p:q] sorted
******************************************************************************
Notes:
Adapted from Sedgewick, R., 1983, Algorithms, Addison Wesley, p. 96.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 01/13/89
*****************************************************************************/
{
    int i,j;
    float ai;

    for (i=p+1; i<=q; i++) {
        for (ai=a[i],j=i; j>p && a[j-1]>ai; j--)
            a[j] = a[j-1];
        a[j] = ai;
    }
}

void Sdp2dUtils::qksort (int n, float a[])
/*****************************************************************************
Sort an array such that a[0] <= a[1] <= ... <= a[n-1]
******************************************************************************
Input:
n		number of elements in array a
a		array[n] containing elements to be sorted

Output:
a		array[n] containing sorted elements
******************************************************************************
Notes:
n must be less than 2^NSTACK, where NSTACK is defined above.

This function is adapted from procedure quicksort by
Hoare, C.A.R., 1961, Communications of the ACM, v. 4, p. 321;
the main difference is that recursion is accomplished
explicitly via a stack array for efficiency; also, a simple
insertion sort is used to sort subarrays too small to be
partitioned efficiently.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 01/13/89
*****************************************************************************/
{
    int pstack[NSTACK],qstack[NSTACK],j,k,p,q,top=0;

    /* initialize subarray lower and upper bounds to entire array */
    pstack[top] = 0;
    qstack[top++] = n-1;

    /* while subarrays remain to be sorted */
    while(top!=0) {

        /* get a subarray off the stack */
        p = pstack[--top];
        q = qstack[top];

        /* while subarray can be partitioned efficiently */
        while(q-p>NSMALL) {

            /* partition subarray into two subarrays */
            qkpart(a,p,q,&j,&k);

            /* save larger of the two subarrays on stack */
            if (j-p<q-k) {
                pstack[top] = k;
                qstack[top++] = q;
                q = j;
            } else {
                pstack[top] = p;
                qstack[top++] = j;
                p = k;
            }
        }
        /* use insertion sort to finish sorting small subarray */
        qkinss(a,p,q);
    }
}

void Sdp2dUtils::qkfind (int m, int n, float a[])
/*****************************************************************************
Partially sort an array so that the element a[m] has the value it
would have if the entire array were sorted such that
a[0] <= a[1] <= ... <= a[n-1]
******************************************************************************
Input:
m		index of element to be found
n		number of elements in array a
a		array[n] to be partially sorted

Output:
a		array[n] partially sorted
******************************************************************************
Notes:
This function is adapted from procedure find by
Hoare, C.A.R., 1961, Communications of the ACM, v. 4, p. 321.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 01/13/89
*****************************************************************************/
{
    int j,k,p,q;

    /* initialize subarray lower and upper bounds to entire array */
    p = 0;  q = n-1;

    /* while subarray can be partitioned efficiently */
    while(q-p>NSMALL) {

        /* partition subarray into two subarrays */
        qkpart(a,p,q,&j,&k);

        /* if desired value is in lower subarray, then */
        if (m<=j)
            q = j;

        /* else, if desired value is in upper subarray, then */
        else if (m>=k)
            p = k;

        /* else, desired value is between j and k */
        else
            return;
    }

    /* completely sort the small subarray with insertion sort */
    qkinss(a,p,q);
}

void Sdp2dUtils::mksinc (float d, int lsinc, float sinc[])
/*****************************************************************************
Compute least-squares optimal sinc interpolation coefficients.
******************************************************************************
Input:
d		fractional distance to interpolation point; 0.0<=d<=1.0
lsinc		length of sinc approximation; lsinc%2==0 and lsinc<=20

Output:
sinc		array[lsinc] containing interpolation coefficients
******************************************************************************
Notes:
The coefficients are a least-squares-best approximation to the ideal
sinc function for frequencies from zero up to a computed maximum
frequency.  For a given interpolator length, lsinc, mksinc computes
the maximum frequency, fmax (expressed as a fraction of the nyquist
frequency), using the following empirically derived relation (from
a Western Geophysical Technical Memorandum by Ken Larner):

	fmax = min(0.066+0.265*log(lsinc),1.0)

Note that fmax increases as lsinc increases, up to a maximum of 1.0.
Use the coefficients to interpolate a uniformly-sampled function y(i) 
as follows:

            lsinc-1
    y(i+d) =  sum  sinc[j]*y(i+j+1-lsinc/2)
              j=0

Interpolation error is greatest for d=0.5, but for frequencies less
than fmax, the error should be less than 1.0 percent.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 06/02/89
*****************************************************************************/
{
	int j;
	double s[20],a[20],c[20],work[20],fmax;

	/* compute auto-correlation and cross-correlation arrays */
	fmax = 0.066+0.265*log((double)lsinc);
	fmax = (fmax<1.0)?fmax:1.0;
	for (j=0; j<lsinc; j++) {
		a[j] = dsinc(fmax*j);
		c[j] = dsinc(fmax*(lsinc/2-j-1+d));
	}

	/* solve symmetric Toeplitz system for the sinc approximation */
	stoepd(lsinc,a,c,s,work);
	for (j=0; j<lsinc; j++)
		sinc[j] = s[j];
}

void Sdp2dUtils::ints8r (int nxin, float dxin, float fxin, float yin[],
        float yinl, float yinr, int nxout, float xout[], float yout[])
/*****************************************************************************
Interpolation of a uniformly-sampled real function y(x) via a
table of 8-coefficient sinc approximations; maximum error for frequiencies
less than 0.6 nyquist is less than one percent.
******************************************************************************
Input:
nxin		number of x values at which y(x) is input
dxin		x sampling interval for input y(x)
fxin		x value of first sample input
yin		array[nxin] of input y(x) values:  yin[0] = y(fxin), etc.
yinl		value used to extrapolate yin values to left of yin[0]
yinr		value used to extrapolate yin values to right of yin[nxin-1]
nxout		number of x values a which y(x) is output
xout		array[nxout] of x values at which y(x) is output

Output:
yout		array[nxout] of output y(x):  yout[0] = y(xout[0]), etc.
******************************************************************************
Notes:
Because extrapolation of the input function y(x) is defined by the
left and right values yinl and yinr, the xout values are not restricted
to lie within the range of sample locations defined by nxin, dxin, and
fxin.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 06/02/89
*****************************************************************************/

{
    int NTABLE = 513;
    int LTABLE = 8;

    static float table[513][8];
	static int tabled=0;
	int jtable;
	float frac;


	/* tabulate sinc interpolation coefficients if not already tabulated */
	if (!tabled) {
		for (jtable=1; jtable<NTABLE-1; jtable++) {
			frac = (float)jtable/(float)(NTABLE-1);
			mksinc(frac,LTABLE,&table[jtable][0]);
		}
		for (jtable=0; jtable<LTABLE; jtable++) {
			table[0][jtable] = 0.0;
			table[NTABLE-1][jtable] = 0.0;
		}
		table[0][LTABLE/2-1] = 1.0;
		table[NTABLE-1][LTABLE/2] = 1.0;
		tabled = 1;
	}

	/* interpolate using tabulated coefficients */
	intt8r(NTABLE,table,nxin,dxin,fxin,yin,yinl,yinr,nxout,xout,yout);
}


void Sdp2dUtils::intt8r (int ntable, float table[][8],
	int nxin, float dxin, float fxin, float yin[], float yinl, float yinr,
	int nxout, float xout[], float yout[])
{
	int ioutb,nxinm8,ixout,ixoutn,kyin,ktable,itable;
	float xoutb,xoutf,xouts,xoutn,frac,fntablem1,yini,sum,
		*yin0,*table00,*pyin,*ptable;

	/* compute constants */
	ioutb = -3-8;
	xoutf = fxin;
	xouts = 1.0/dxin;
	xoutb = 8.0-xoutf*xouts;
	fntablem1 = (float)(ntable-1);
	nxinm8 = nxin-8;
	yin0 = &yin[0];
	table00 = &table[0][0];

	/* loop over output samples */
	for (ixout=0; ixout<nxout; ixout++) {

		/* determine pointers into table and yin */
		xoutn = xoutb+xout[ixout]*xouts;
		ixoutn = (int)xoutn;
		kyin = ioutb+ixoutn;
		pyin = yin0+kyin;
		frac = xoutn-(float)ixoutn;
		ktable = frac>=0.0?frac*fntablem1+0.5:(frac+1.0)*fntablem1-0.5;
		ptable = table00+ktable*8;
		
		/* if totally within input array, use fast method */
		if (kyin>=0 && kyin<=nxinm8) {
			yout[ixout] = 
				pyin[0]*ptable[0]+
				pyin[1]*ptable[1]+
				pyin[2]*ptable[2]+
				pyin[3]*ptable[3]+
				pyin[4]*ptable[4]+
				pyin[5]*ptable[5]+
				pyin[6]*ptable[6]+
				pyin[7]*ptable[7];
		
		/* else handle end effects with care */
		} else {
	
			/* sum over 8 tabulated coefficients */
			for (itable=0,sum=0.0; itable<8; itable++,kyin++) {
				if (kyin<0)
					yini = yinl;
				else if (kyin>=nxin)
					yini = yinr;
				else
					yini = yin[kyin];
				sum += yini*(*ptable++);
			}
			yout[ixout] = sum;
		}
	}
}

double Sdp2dUtils::dsinc (double x)
/*****************************************************************************
Return sinc(x) = sin(PI*x)/(PI*x) (double version)
******************************************************************************
Input:
x       value at which to evaluate sinc(x)

Returned:   sinc(x)
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 06/02/89
*****************************************************************************/
{
    double pix;

    if (x==0.0) {
        return 1.0;
    } else {
        pix = 3.141592653589793*x;
        return sin(pix)/pix;
    }
}

void Sdp2dUtils::stoepd (int n, double r[], double g[], double f[], double a[])
/*****************************************************************************
Solve a symmetric Toeplitz linear system of equations Rf=g for f
(double version)
******************************************************************************
Input:
n       dimension of system
r       array[n] of top row of Toeplitz matrix
g       array[n] of right-hand-side column vector

Output:
f       array[n] of solution (left-hand-side) column vector
a       array[n] of solution to Ra=v (Claerbout, FGDP, p. 57)
******************************************************************************
Notes:
This routine does NOT solve the case when the main diagonal is zero, it
just silently returns.

The left column of the Toeplitz matrix is assumed to be equal to the top
row (as specified in r); i.e., the Toeplitz matrix is assumed symmetric.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 06/02/89
*****************************************************************************/
{
    int i,j;
    double v,e,c,w,bot;

    if (r[0] == 0.0) return;

    a[0] = 1.0;
    v = r[0];
    f[0] = g[0]/r[0];

    for (j=1; j<n; j++) {

        /* solve Ra=v as in Claerbout, FGDP, p. 57 */
        a[j] = 0.0;
        f[j] = 0.0;
        for (i=0,e=0.0; i<j; i++)
            e += a[i]*r[j-i];
        c = e/v;
        v -= c*e;
        for (i=0; i<=j/2; i++) {
            bot = a[j-i]-c*a[i];
            a[i] -= c*a[j-i];
            a[j-i] = bot;
        }

        /* use a and v above to get f[i], i = 0,1,2,...,j */
        for (i=0,w=0.0; i<j; i++)
            w += f[i]*r[j-i];
        c = (w-g[j])/v;
        for (i=0; i<=j; i++)
            f[i] -= c*a[j-i];
    }
}

void Sdp2dUtils::yxtoxy (int nx, float dx, float fx, float y[],
                         int ny, float dy, float fy, float xylo, float xyhi, float x[])
{
    int nxi,nyo,jxi1,jxi2,jyo;
    float dxi,fxi,dyo,fyo,fyi,yo,xi1,yi1,yi2;

    nxi = nx; dxi = dx; fxi = fx;
    nyo = ny; dyo = dy; fyo = fy;
    fyi = y[0];

    /* loop over output y less than smallest input y */
    for (jyo=0,yo=fyo; jyo<nyo; jyo++,yo+=dyo) {
        if (yo>=fyi) break;
        x[jyo] = xylo;
    }

    /* loop over output y between smallest and largest input y */
    if (jyo==nyo-1 && yo==fyi) {
        x[jyo++] = fxi;
        yo += dyo;
    }
    jxi1 = 0;
    jxi2 = 1;
    xi1 = fxi;
    while (jxi2<nxi && jyo<nyo) {
        yi1 = y[jxi1];
        yi2 = y[jxi2];
        if (yi1<=yo && yo<=yi2) {
            x[jyo++] = xi1+dxi*(yo-yi1)/(yi2-yi1);
            yo += dyo;
        } else {
            jxi1++;
            jxi2++;
            xi1 += dxi;
        }
    }

    /* loop over output y greater than largest input y */
    while (jyo<nyo)
        x[jyo++] = xyhi;
}





