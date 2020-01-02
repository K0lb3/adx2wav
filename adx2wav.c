/*
	adv2wavmod3

	(c)2001 BERO
    (c)2008 hcs

	http://www.geocities.co.jp/Playtown/2004/
	bero@geocities.co.jp

	adx info from: http://ku-www.ss.titech.ac.jp/~yatsushi/adx.html

	modified by K0lb3 for buffer i/o
*/


#include <stdio.h>
#include <math.h>
#include <stdlib.h>

const double M_SQRT2 = 1.4142135623730951;
const double M_PI = 3.141592653589793;

long read_long(unsigned char *p)
{
	return (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3];
}

int read_word(unsigned char *p)
{
	return (p[0]<<8)|p[1];
}

typedef struct {
	int s1,s2;
} PREV;

void convert(short *out,unsigned char *in,PREV *prev,int coef1, int coef2)
{
	int scale = (((in[0]<<8)|(in[1]))) + 1;
	int i;
	int s0,s1,s2,d;
//	int over=0;

	in+=2;
	s1 = prev->s1;
	s2 = prev->s2;
	for(i=0;i<16;i++) {
		d = in[i]>>4;
		if (d&8) d-=16;
		s0 = d*scale + ((coef1*s1 + coef2*s2)>>12);
//		if (abs(s0)>32767) over=1;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;
		*out++=s0;
		s2 = s1;
		s1 = s0;

		d = in[i]&15;
		if (d&8) d-=16;
		s0 = d*scale + ((coef1*s1 + coef2*s2)>>12);
//		if (abs(s0)>32767) over=1;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;
		*out++=s0;
		s2 = s1;
		s1 = s0;
	}
	prev->s1 = s1;
	prev->s2 = s2;

//	if (over) putchar('.');
}

#define PY_SSIZE_T_CLEAN  /* Make "s#" use Py_ssize_t rather than int. */
#include <Python.h>

static PyObject * adx2wav(PyObject * self, PyObject * args) {
    unsigned char *src;
    size_t data_size;
    if (!PyArg_ParseTuple(args, "y#", &src, &data_size)) {
        return NULL;
    }

	unsigned char buf[18*2];
	short outbuf[32*2];
	int offset;
	int channel,freq,size,wsize;
    int coef1,coef2;
	PREV prev[2];

	static struct {
		char hdr1[4];
		long totalsize;

		char hdr2[8];
		long hdrsize;
 		short format;
		short channel;
		long freq;
		long byte_per_sec;
		short blocksize;
		short bits;

		char hdr3[4];
		long datasize;
	} wavhdr = {
		"RIFF",0,
		"WAVEfmt ",0x10,1/* PCM */,2,44100,44100*2*2,2*2,16,
		"data"
	};

	memcpy(buf, src, 16);

	channel = buf[7];
	freq = read_long(buf+8);
	size = read_long(buf+12);

	offset = read_word(buf+2)-2;

	//fseek(in,offset,SEEK_SET);
	src+=offset;
	//fread(buf+1,1,6,in);
	memcpy(buf+1, src, 6);
	src+=6;

	if (buf[0]!=0x80 || memcmp(buf+1,"(c)CRI",6)) {
		puts("not adx!");
		return -1;
	}

	wavhdr.channel = channel;
	wavhdr.freq = freq;
	wavhdr.blocksize = channel*sizeof(short);
	wavhdr.byte_per_sec = freq*wavhdr.blocksize;
	wavhdr.datasize = size*wavhdr.blocksize;
	wavhdr.totalsize = wavhdr.datasize + sizeof(wavhdr)-8;

	prev[0].s1 = 0;
	prev[0].s2 = 0;
	prev[1].s1 = 0;
	prev[1].s2 = 0;

    {
    double x,y,z,a,b,c;

    x = 500;
    y = freq;
    z = cos(2.0*M_PI*x/y);

    a = M_SQRT2-z;
    b = M_SQRT2-1.0;
    c = (a-sqrt((a+b)*(a-b)))/b;

    coef1 = floor(8192.0*c);
    coef2 = floor(-4096.0*c*c);
    }

	size_t dst_len = sizeof(unsigned char) * size * channel * 2 + 44;
	unsigned char* dst = malloc(dst_len);
	short* sdst = (short*) dst;
	int dst_offset = 0;
	memcpy(dst, &wavhdr,sizeof(wavhdr));
	dst_offset += sizeof(wavhdr)/2;

	if (channel==1)
		while(size) {
			//fread(buf,1,18,in);
			memcpy(buf, src, 18);
			src+=18;

			//convert(outbuf,buf,prev,coef1,coef2);
			convert(sdst+dst_offset,buf,prev,coef1,coef2);
			dst_offset+=32*2;

			if (size>32) wsize=32; else wsize = size;
			size-=wsize;
			//fwrite(outbuf,1,wsize*2,out);
		}
	else if (channel==2)
		while(size) {
			short tmpbuf[32*2];
			int i;

			//fread(buf,1,18*2,in);
			memcpy(buf, src, 36);
			src+=36;

			convert(tmpbuf,buf,prev,coef1,coef2);
			convert(tmpbuf+32,buf+18,prev+1,coef1,coef2);
			/*
			for(i=0;i<32;i++) {
				outbuf[i*2]   = tmpbuf[i];
				outbuf[i*2+1] = tmpbuf[i+32];
			}
			*/
			for(i=0;i<32;i++) {
				sdst[dst_offset+i*2]   = tmpbuf[i];
				sdst[dst_offset+i*2+1] = tmpbuf[i+32];
			}
			dst_offset+=32*2;

			if (size>32) wsize=32; else wsize = size;
			size-=wsize;
			//fwrite(outbuf,1,wsize*2*2,out);
		}

	return Py_BuildValue("y#", dst, dst_len);
}

// Exported methods are collected in a table
static struct PyMethodDef method_table[] = {
    {
        "adx2wav",
        (PyCFunction) adx2wav,
        METH_VARARGS,
        "Converts adx to wav.\
        :input: adx bytes\
        :returns: wav bytes"
    },
    {
        NULL,
        NULL,
        0,
        NULL
    } // Sentinel value ending the table
};

// A struct contains the definition of a module
static PyModuleDef _adx2wav_module = {
    PyModuleDef_HEAD_INIT,
    "adx2wav", // Module name
    "Converts adx to wav.",
    -1, // Optional size of the module state memory
    method_table,
    NULL, // Optional slot definitions
    NULL, // Optional traversal function
    NULL, // Optional clear function
    NULL // Optional module deallocation function
};

// The module init function
PyMODINIT_FUNC PyInit_adx2wav(void) {
    return PyModule_Create( &_adx2wav_module);
}
