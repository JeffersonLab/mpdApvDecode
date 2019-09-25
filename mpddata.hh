//  class mpddata
//    Class to handle decoding of MPD and APV data.
//

#ifndef __MPDDEC__
#define __MPDDEC__
#include <stdint.h>
#include <ostream>
#include "decconfig.hh"

#define DEC_ERR(format, ...) {						\
    char buf[128];							\
    sprintf((char *)buf,format, ## __VA_ARGS__);			\
    *fp << "# ERROR: " << buf;						\
  }

#define DEC_SHOW(format, ...) {						\
    char buf[128];							\
    sprintf((char *)buf,format, ## __VA_ARGS__);			\
    *fp << buf;								\
  }



class mpddata
{
public:
  typedef struct
  {
    uint32_t ndata; // number of data words in current frame
    uint8_t sample_count; // current frame  APV Trailer
    uint8_t frame_count; // current frame  APV Trailer
    uint16_t baseline_value; // current frame APV Trailer
    uint8_t word_count; // current frame  APV Trailer
  } apvstats;

  typedef struct
  {
    uint32_t n_words_in_block; // block trailer
    uint16_t apvmask;
    uint8_t  napv;
    uint32_t event_count;  // event header
    uint64_t trigger_time; // trigger time
    apvstats apv[16]; // apv_id is index
    uint16_t n_words_in_event;
    int have_stats;
  } mpdstats;

  mpdstats mpd[21];
  uint32_t mpdmask;
  uint16_t nmpd;

  decconfig *cfg;
  ostream *fp;

  mpddata(decconfig &acfg)
  {
    cfg = &acfg;
    fp = &cout;
  };

  int SetOutput(ostream &os)
  {
    fp = &os;
    return 0;
  };
  int ShowData(int enable);
  int ClearStats();
  int DecodeWord(uint32_t data);
  int DecodeBuffer(const uint32_t *buffer, int len);

  ////////////////////////////////////////////////////////
  // Decoding structures and unions for the MPD data
  typedef struct
  {
    uint32_t data:21;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } generic_data_word;

  typedef union
  {
    uint32_t raw;
    generic_data_word bf;
  } generic_data_word_t;

  /* 0: BLOCK HEADER */
  typedef struct
  {
    uint32_t block_count:8;
    uint32_t event_per_block:8;
    uint32_t module_id:5;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_block_header;

  typedef union
  {
    uint32_t raw;
    mpd_block_header bf;
  } mpd_block_header_t;

  /* 1: BLOCK TRAILER */
  typedef struct
  {
    uint32_t n_words_in_block:20;
    uint32_t blank:1;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_block_trailer;

  typedef union
  {
    uint32_t raw;
    mpd_block_trailer bf;
  } mpd_block_trailer_t;

  /* 2: EVENT HEADER */
  typedef struct
  {
    uint32_t event_count:20;
    uint32_t blank:1;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_event_header;

  typedef union
  {
    uint32_t raw;
    mpd_event_header bf;
  } mpd_event_header_t;

  /* 3: TRIGGER TIME */
  typedef struct
  {
    uint32_t coarse_trigger_time:20;
    uint32_t cont:1;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_trigger_time;

  typedef union
  {
    uint32_t raw;
    mpd_trigger_time bf;
  } mpd_trigger_time_t;

  /* 4: APV CHANNEL DATA */
  typedef struct
  {
    uint32_t processed_data:19;
    uint32_t proc_data_type:2;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_apv_channel_data;

  typedef union
  {
    uint32_t raw;
    mpd_apv_channel_data bf;
  } mpd_apv_channel_data_t;

  /* 4.0: HEADER */
  typedef struct
  {
    uint32_t apv_id:4;
    uint32_t apv_header:13;
    uint32_t what:1;
    uint32_t blank:1;
    uint32_t proc_data_type:2;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_data_header;

  typedef union
  {
    uint32_t raw;
    mpd_data_header bf;
  } mpd_data_header_t;

  /* 4.1: Reduced Data */
  typedef struct
  {
    uint32_t data:12;
    uint32_t channel_number:7;
    uint32_t proc_data_type:2;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_reduced_data;

  typedef union
  {
    uint32_t raw;
    mpd_reduced_data bf;
  } mpd_reduced_data_t;

  /* 4.2: ApvTrailer */
  typedef struct
  {
    uint32_t frame_counter:8;
    uint32_t sample_count:4;
    uint32_t mod_id:6;
    uint32_t blank:1;
    uint32_t proc_data_type:2;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_apv_trailer;

  typedef union
  {
    uint32_t raw;
    mpd_apv_trailer bf;
  } mpd_apv_trailer_t;

  /* 4.3: Trailer */
  typedef struct
  {
    uint32_t word_count:8;
    uint32_t baseline_value:11;
    uint32_t proc_data_type:2;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_data_trailer;

  typedef union
  {
    uint32_t raw;
    mpd_data_trailer bf;
  } mpd_data_trailer_t;


  /* 5: EVENT TRAILER */
  typedef struct
  {
    uint32_t fine_trigger_time:8;
    uint32_t n_words_in_event:12;
    uint32_t blank:1;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_event_trailer;

  typedef union
  {
    uint32_t raw;
    mpd_event_trailer bf;
  } mpd_event_trailer_t;


  /* 7: FILLER */
  typedef struct
  {
    uint32_t zero:21;
    uint32_t data_type_tag:3;
    uint32_t undef:8;
  } mpd_filler;

  typedef union
  {
    uint32_t raw;
    mpd_filler bf;
  } mpd_filler_t;

};

#endif /* __MPDDEC__ */
