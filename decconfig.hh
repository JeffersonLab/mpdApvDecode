//  class decconfig
//    Class to handle configuring the output of mpddata
//
#ifndef __DECCONFIG__
#define __DECCONFIG__
#include <GI_Config.h>
#include <stdint.h>

class decconfig
{
public:
  decconfig(std::string filename);
  ~decconfig();

  void set_roc_number(int rocnum)
  {
    current_rocnum = rocnum;
  }

  uint32_t mpdmask();
  uint16_t apvmask();
  uint16_t minimum_baseline();
  uint16_t maximum_baseline();

  void show_no_data(int enable);

  bool show_block_header(int mpd);
  bool show_block_trailer(int mpd);
  bool show_event_header(int mpd);
  bool show_trigger_time(int mpd);
  bool show_apv_header(int mpd, int apv);
  bool show_apv_data(int mpd, int apv);
  bool show_apv_trailer(int mpd, int apv);
  bool show_event_trailer(int mpd);
  bool show_filler_word(int mpd);

  bool check_apvmask(int mpd);
  bool check_napv(int mpd);
  bool check_event_count(int mpd);
  bool check_ndata(int mpd, int apv);
  bool check_sample_count(int mpd, int apv);
  bool check_frame_count(int mpd, int apv);
  bool check_baseline_value(int mpd, int apv);
  bool check_word_count(int mpd, int apv);
  bool check_n_words_in_event(int mpd);
  bool check_n_words_in_block(int mpd);
  bool check_data_count(int mpd, int apv);

private:
  GI_Config *cfg;
  uint8_t current_rocnum;
  bool showOverride;

};

#endif // __DECCONFIG__
