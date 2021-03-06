#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h> 
#include <stdarg.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#define VERSION "0.55"

#ifdef _MSC_VER
#include "stdintmsc.h"
// Don't bug me with strcpy() deprecation warnings
#pragma warning(disable : 4996)
#else
#include <stdint.h>
#endif

typedef int64_t LLONG;

#include "608.h"
#include "708.h"
#include "bitstream.h"

#ifdef _WIN32
#define FOPEN64 fopen
#define OPEN _open
// 64 bit file functions
extern "C" int __cdecl _fseeki64(FILE *, __int64, int);
extern "C" __int64 __cdecl _ftelli64(FILE *);

#define FSEEK _fseeki64
#define FTELL _ftelli64
#define TELL _telli64
#define LSEEK _lseeki64
typedef struct _stati64 FSTATSTRUCT;
#else
#ifdef __CYGWIN__
// Cygwin internally maps these functions to 64bit usage, but there are
// no explicit xxxx64 functions.
#define FOPEN64 fopen
#define OPEN open
#define LSEEK lseek
#else
#define FOPEN64 fopen64
#define OPEN open64
#define LSEEK lseek64
#endif
#define FSEEK fseek
#define FTELL ftell
#define FSTAT fstat
#define TELL tell
#include <stdint.h>
#endif 

#ifndef int64_t_C
#define int64_t_C(c)     (c ## LL)
#define uint64_t_C(c)    (c ## ULL)
#endif

//typedef signed long long LLONG;


#define ONEPASS 120 /* Bytes we can always look ahead without going out of limits */
#define BUFSIZE (2048*1024+ONEPASS) /* 2 Mb plus the safety pass */
#define MAX_CLOSED_CAPTION_DATA_PER_PICTURE 32
#define EIA_708_BUFFER_LENGTH   2048 // TODO: Find out what the real limit is
#define TS_PACKET_PAYLOAD_LENGTH     184     // From specs
#define SUBLINESIZE 2048 // Max. length of a .srt line - TODO: Get rid of this
#define STARTBYTESLENGTH	(1024*1024)

#define XMLRPC_CHUNK_SIZE (64*1024) // 64 Kb per chunk, to avoid too many realloc()

struct boundary_time
{
    int hh,mm,ss;
    LLONG time_in_ms;
    int set;
};

struct s_write
{
    FILE *fh;
    char *filename;
    struct eia608 *data608;
};

enum output_format
{
    OF_RAW	= 0,
    OF_SRT	= 1,
    OF_SAMI = 2,
    OF_TRANSCRIPT = 3,
    OF_RCWT = 4
};

enum stream_mode_enum
{
    SM_ELEMENTARY_OR_NOT_FOUND=0,
    SM_TRANSPORT=1,
    SM_PROGRAM=2,
    SM_ASF=3,
    SM_MCPOODLESRAW = 4,
    SM_RCWT = 5, // Raw Captions With Time, not used yet.
    SM_MYTH = 6, // Use the myth loop
    SM_AUTODETECT = 16
};

enum encoding_type
{
    ENC_UNICODE = 0,
    ENC_LATIN_1 = 1,
    ENC_UTF_8 = 2
};

enum bufferdata_type
{
    UNKNOWN = 0,
    PES = 1,
    RAW = 2,
    H264 = 3
};

enum frame_type
{
    RESET_OR_UNKNOWN = 0,
    I_FRAME = 1,
    P_FRAME = 2,
    B_FRAME = 3,
    D_FRAME = 4
};

struct gop_time_code
{
  int drop_frame_flag;
  int time_code_hours;
  int time_code_minutes;
  int marker_bit;
  int time_code_seconds;
  int time_code_pictures;
  int inited;
  LLONG ms;
};

extern struct gop_time_code gop_time, first_gop_time, printed_gop;
extern int gop_rollover;
extern LLONG min_pts, sync_pts, current_pts;
extern LLONG fts_now; // Time stamp of current file (w/ fts_offset, w/o fts_global)
extern LLONG fts_offset; // Time before first sync_pts
extern LLONG fts_fc_offset; // Time before first GOP
extern LLONG fts_max; // Remember the maximum fts that we saw in current file
extern LLONG fts_global; // Duration of previous files (-ve mode)
// Count 608 (per field) and 708 blocks since last set_fts() call
extern int cb_field1, cb_field2, cb_708;
extern int saw_caption_block;

extern const char *framerates_types[16];
extern const double framerates_values[16];
extern unsigned char *buffer;
extern LLONG past;
extern LLONG total_inputsize, total_past; // Only in binary concat mode

extern char **inputfile;
extern int current_file;
extern LLONG result; // Number of bytes read/skipped in last read operation

extern int strangeheader;

extern unsigned char startbytes[STARTBYTESLENGTH]; 
extern unsigned int startbytes_pos;
extern int startbytes_avail; // Needs to be able to hold -1 result.

extern unsigned char *pesheaderbuf;
extern int pts_set; //0 = No, 1 = received, 2 = min_pts set

extern int MPEG_CLOCK_FREQ; // This is part of the standard

extern unsigned pts_big_change;
extern unsigned total_frames_count;
extern unsigned total_pulldownfields;
extern unsigned total_pulldownframes;

extern int CaptionGap;

extern LLONG buffered_read_opt (unsigned char *buffer, unsigned int bytes);

extern unsigned char *filebuffer;
extern LLONG filebuffer_start; // Position of buffer start relative to file
extern int filebuffer_pos; // Position of pointer relative to buffer start
extern int bytesinbuffer; // Number of bytes we actually have on buffer
extern char *xmlrpc_content_body;
extern long xmlrpc_content_length;
extern long xmlrpc_content_capacity;
extern const char *xmlrpc_url;
extern int live_stream;

#define XMLRPC_APPEND(data,length) if (xmlrpc_url!=NULL) xmlrpc_append((char *) data,length);

#define buffered_skip(bytes) if (bytes<=bytesinbuffer-filebuffer_pos) { \
    filebuffer_pos+=bytes; \
    result=bytes; \
} else result=buffered_read_opt (NULL,bytes);

#define buffered_read(buffer,bytes) if (bytes<=bytesinbuffer-filebuffer_pos) { \
    if (buffer!=NULL) memcpy (buffer,filebuffer+filebuffer_pos,bytes); \
    filebuffer_pos+=bytes; \
    result=bytes; \
} else result=buffered_read_opt (buffer,bytes);

#define buffered_read_4(buffer) if (4<=bytesinbuffer-filebuffer_pos) { \
    if (buffer) { buffer[0]=filebuffer[filebuffer_pos]; \
    buffer[1]=filebuffer[filebuffer_pos+1]; \
    buffer[2]=filebuffer[filebuffer_pos+2]; \
    buffer[3]=filebuffer[filebuffer_pos+3]; \
    filebuffer_pos+=4; \
    result=4; } \
} else result=buffered_read_opt (buffer,4);

#define buffered_read_byte(buffer) if (bytesinbuffer-filebuffer_pos) { \
    if (buffer) { *buffer=filebuffer[filebuffer_pos]; \
    filebuffer_pos++; \
    result=1; } \
} else result=buffered_read_opt (buffer,1);

// ASF GUIDs
// 10.1
#define ASF_HEADER "\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C"
#define ASF_DATA "\x36\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C"

// 10.2
#define ASF_FILE_PROPERTIES "\xA1\xDC\xAB\x8C\x47\xA9\xCF\x11\x8E\xE4\x00\xC0\x0C\x20\x53\x65"
#define ASF_STREAM_PROPERTIES "\x91\x07\xDC\xB7\xB7\xA9\xCF\x11\x8E\xE6\x00\xC0\x0C\x20\x53\x65"
#define ASF_HEADER_EXTENSION "\xB5\x03\xBF\x5F\x2E\xA9\xCF\x11\x8E\xE3\x00\xC0\x0C\x20\x53\x65"
#define ASF_CONTENT_DESCRIPTION "\x33\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C"
#define ASF_EXTENDED_CONTENT_DESCRIPTION "\x40\xA4\xD0\xD2\x07\xE3\xD2\x11\x97\xF0\x00\xA0\xC9\x5E\xA8\x50"
#define ASF_STREAM_BITRATE_PROPERTIES "\xCE\x75\xF8\x7B\x8D\x46\xD1\x11\x8D\x82\x00\x60\x97\xC9\xA2\xB2"
// 10.3
#define ASF_EXTENDED_STREAM_PROPERTIES "\xCB\xA5\xE6\x14\x72\xC6\x32\x43\x83\x99\xA9\x69\x52\x06\x5B\x5A"
#define ASF_METADATA "\xEA\xCB\xF8\xC5\xAF\x5B\x77\x48\x84\x67\xAA\x8C\x44\xFA\x4C\xCA"
#define ASF_METADATA_LIBRARY "\x94\x1C\x23\x44\x98\x94\xD1\x49\xA1\x41\x1D\x13\x4E\x45\x70\x54"
#define ASF_COMPATIBILITY2 "\x5D\x8B\xF1\x26\x84\x45\xEC\x47\x9F\x5F\x0E\x65\x1F\x04\x52\xC9"
// Actually 10.2
#define ASF_PADDING "\x74\xD4\x06\x18\xDF\xCA\x09\x45\xA4\xBA\x9A\xAB\xCB\x96\xAA\xE8"
// 10.4
#define ASF_AUDIO_MEDIA "\x40\x9E\x69\xF8\x4D\x5B\xCF\x11\xA8\xFD\x00\x80\x5F\x5C\x44\x2B"
#define ASF_VIDEO_MEDIA "\xC0\xEF\x19\xBC\x4D\x5B\xCF\x11\xA8\xFD\x00\x80\x5F\x5C\x44\x2B"
#define ASF_BINARY_MEDIA "\xE2\x65\xFB\x3A\xEF\x47\xF2\x40\xAC\x2C\x70\xA9\x0D\x71\xD3\x43"

// ASF_BINARY_MEDIA : Major media types
#define DVRMS_AUDIO "\x9D\x8C\x17\x31\xE1\x03\x28\x45\xB5\x82\x3D\xF9\xDB\x22\xF5\x03"
#define DVRMS_NTSC "\x80\xEA\x0A\x67\x82\x3A\xD0\x11\xB7\x9B\x00\xAA\x00\x37\x67\xA7"
#define DVRMS_ATSC "\x89\x8A\x8B\xB8\x49\xB0\x80\x4C\xAD\xCF\x58\x98\x98\x5E\x22\xC1"

// 10.13 - Undocumented DVR-MS properties
#define DVRMS_PTS "\x2A\xC0\x3C\xFD\xDB\x06\xFA\x4C\x80\x1C\x72\x12\xD3\x87\x45\xE4"


// MPEG-2 TS stream types
enum stream_type
{
    UNKNOWNSTREAM = 0,

    VIDEO_MPEG1 = 0x01,
    VIDEO_MPEG2 = 0x02,
    AUDIO_MPEG1 = 0x03,
    AUDIO_MPEG2 = 0x04,
    AUDIO_AAC   = 0x0f,
    VIDEO_MPEG4 = 0x10,
    VIDEO_H264  = 0x1b,

    AUDIO_AC3   = 0x81,
    AUDIO_HDMV_DTS = 0x82,
    AUDIO_DTS   = 0x8a,
};
extern const char *desc[256];

extern FILE *clean;
extern int infd;
extern const char *aspect_ratio_types[16];
extern const char *pict_types[8];
extern const char *cc_types[4];
extern int false_pict_header;

extern int stat_numuserheaders;
extern int stat_dvdccheaders;
extern int stat_scte20ccheaders;
extern int stat_replay5000headers;
extern int stat_replay4000headers;
extern int stat_dishheaders;
extern int stat_hdtv;
extern int stat_divicom;
extern int ff_cleanup; 
extern stream_mode_enum stream_mode;
extern int use_gop_as_pts;
extern int fix_padding; 
extern int rawmode; 
extern int extract; 
extern int cc_stats[4];
extern LLONG inputsize;
extern int cc_channel;
extern encoding_type encoding ;
extern int direct_rollup;
extern LLONG subs_delay; 
extern struct boundary_time extraction_start, extraction_end; 
extern struct boundary_time startcreditsnotbefore, startcreditsnotafter;
extern struct boundary_time startcreditsforatleast, startcreditsforatmost;
extern struct boundary_time endcreditsforatleast, endcreditsforatmost;
extern int startcredits_displayed, end_credits_displayed;
extern LLONG last_displayed_subs_ms; 
extern LLONG screens_to_process;
extern int processed_enough;
extern int nofontcolor;
extern unsigned char usercolor_rgb[8];
extern color_code default_color;
extern int sentence_cap;
extern int binary_concat;
extern int trim_subs;
extern int norollup;
extern int gui_mode_reports;
extern int no_progress_bar;
extern const char *extension;

//params.cpp
void parse_parameters (int argc, char *argv[]);
void usage (void);

// general_loop.cpp
void position_sanity_check ();
int init_file_buffer( void );
LLONG ps_getmoredata( void );
LLONG general_getmoredata( void );
void raw_loop (void);
LLONG process_raw (void);
void general_loop(void);
void rcwt_loop( void );

/* General (ES stream) video information */
extern unsigned current_hor_size;
extern unsigned current_vert_size;
extern unsigned current_aspect_ratio;
extern unsigned current_frame_rate;
extern double current_fps;


// activity.cpp
void activity_header (void);
void activity_progress (int percentaje, int cur_min, int cur_sec);
void activity_report_version (void);
void activity_input_file_closed (void);
void activity_input_file_open (const char *filename);
void activity_message (const char *fmt, ...);
void  activity_video_info (int hor_size,int vert_size, 
    const char *aspect_ratio, const char *framerate);

extern LLONG result;
extern int end_of_file;
extern LLONG inbuf;
extern int bufferdatatype; // Can be RAW or PES

// asf_functions.cpp
LLONG asf_getmoredata( void );

// avc_functions.cpp
LLONG process_avc (unsigned char *avcbuf, LLONG avcbuflen);

// es_functions.cpp
LLONG process_m2v (unsigned char *data, LLONG length);

extern unsigned top_field_first;

// es_userdata.cpp
int user_data(struct bitstream *ustream, int udtype);

// bitstream.cpp - see bitstream.h


// 608.cpp
int write_cc_buffer (struct s_write *wb);
unsigned char *debug_608toASC (unsigned char *ccdata, int channel);

extern int firstcall;

// file_functions.cpp
LLONG getfilesize (int in);
LLONG gettotalfilessize (void);
void prepare_for_new_file (void);
void close_input_file (void);
int switch_to_next_file (LLONG bytesinbuffer);

// timing.cpp
void set_fts(void);
LLONG get_fts(void);
LLONG get_fts_max(void);
char *print_mstime( LLONG mstime );
void print_debug_timing( void );
int gop_accepted(struct gop_time_code* g );
void calculate_ms_gop_time (struct gop_time_code *g);

// sequencing.cpp
void init_hdcc (void);
void store_hdcc(unsigned char *cc_data, int cc_count, int sequence_number, LLONG current_fts);
void anchor_hdcc(int seq);
void process_hdcc (void);
int do_cb (unsigned char *cc_block);

#define MAXBFRAMES 10
#define SORTBUF (2*MAXBFRAMES+1)
extern int cc_data_count[SORTBUF];
extern unsigned char cc_data_pkts[SORTBUF][31*3+1];
extern int has_ccdata_buffered;
extern int current_field;

// params_dump.cpp
void params_dump(void);

// output.cpp
void init_write (struct s_write *wb);
void writeraw (const unsigned char *data, int length, struct s_write *wb);
void writedata (const unsigned char *data, int length, struct s_write *wb);
void flushbuffer (struct s_write *wb, int closefile);
void printdata (const unsigned char *data1, int length1,const unsigned char *data2, int length2);
void writercwtdata (const unsigned char *data);

// stream_functions.cpp
void detect_stream_type (void);
int detect_myth( void );
int read_video_pes_header (unsigned char *header, int *headerlength, int sbuflen);

// ts_functions.cpp
void init_ts_constants( void );
int ts_readpacket(void);
long ts_readstream(void);
LLONG ts_getmoredata( void );

// myth.cpp
void myth_loop(void);

// ccextractor.cpp
LLONG calculate_gop_mstime (struct gop_time_code *g);
void set_fts(void);
LLONG get_fts ( void );
LLONG get_fts_max ( void );
char *print_mstime( LLONG mstime );
void print_debug_timing( void );
int switch_to_next_file (LLONG bytesinbuffer);
void fatal(int exit_code, const char *fmt, ...);

void sleep_secs (int secs);
void dump (unsigned char *start, int l);
void init_eia608 (struct eia608 *data);
unsigned encode_line (unsigned char *buffer, unsigned char *text);
void buffered_seek (int offset);
void write_subtitle_file_header (struct s_write *wb);
void write_subtitle_file_footer (struct s_write *wb);
extern void build_parity_table(void);
void xmlrpc_append (char *data, long length);

extern const unsigned char BROADCAST_HEADER[4];
extern const unsigned char DVD_HEADER[8];
extern const unsigned char lc1[1];
extern const unsigned char lc2[1];
extern const unsigned char lc3[2];
extern const unsigned char lc4[2];
extern const unsigned char lc5[1];
extern const unsigned char lc6[1];

extern int last_reported_progress;
extern int buffer_input;
extern int debug_verbose;
extern int debug_608;
extern int debug_708;
extern int debug_time;
extern int debug_vides;
extern int debug_parse;
extern int debug_cbraw;
extern int nosync;
extern int fullbin;
extern unsigned char *subline;
extern int saw_gop_header;
extern int max_gop_length;
extern int last_gop_length;
extern int frames_since_last_gop;
extern LLONG fts_at_gop_start;
extern int frames_since_ref_time;
extern stream_mode_enum auto_stream;
extern int num_input_files;
extern char *sentence_cap_file;
extern int auto_myth;
extern const unsigned char rcwt_header[11];
extern struct s_write wbout1, wbout2;

extern char **spell_lower;
extern char **spell_correct;
extern int spell_words;
extern int spell_capacity;

extern char *output_filename;
extern char *clean_filename;

extern unsigned char encoded_crlf[16]; // We keep it encoded here so we don't have to do it many times
extern unsigned int encoded_crlf_length;
extern unsigned char encoded_br[16];
extern unsigned int encoded_br_length;
extern output_format write_format;

extern int current_picture_coding_type; 
extern int current_tref; // Store temporal reference of current frame

extern int cc608_parity_table[256]; // From myth

// Credits stuff
extern char *start_credits_text;
extern char *end_credits_text;

/* Exit codes. Take this seriously as the GUI depends on them. 
   0 means OK as usual,
   <100 means display whatever was output to stderr as a warning
   >=100 means display whatever was output to stdout as an error
*/

#define EXIT_OK                                 0
#define EXIT_NO_INPUT_FILES                     2
#define EXIT_TOO_MANY_INPUT_FILES               3
#define EXIT_INCOMPATIBLE_PARAMETERS            4
#define EXIT_FILE_CREATION_FAILED               5
#define EXIT_UNABLE_TO_DETERMINE_FILE_SIZE      6
#define EXIT_MALFORMED_PARAMETER                7
#define EXIT_READ_ERROR                         8
#define EXIT_NOT_CLASSIFIED                     300
#define EXIT_NOT_ENOUGH_MEMORY                  500
#define EXIT_ERROR_IN_CAPITALIZATION_FILE       501
#define EXIT_BUFFER_FULL                        502
#define EXIT_BUG_BUG                            1000
#define EXIT_MISSING_ASF_HEADER                 1001
#define EXIT_MISSING_RCWT_HEADER                1002
