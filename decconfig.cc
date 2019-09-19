//  class decconfig
//    Class to handle configuring the output of mpddata
//
//   Usage:
//
//  Author: Bryan Moffit
//          Jefferson Lab Data Acquisition Group
//          September 2019
//


#include <decconfig.hh>
#include <string>

decconfig::decconfig(std::string filename = "decode.cfg")
{
  current_rocnum = -1;
  cfg = new GI_Config();
  cfg->parseFile(filename);
}

decconfig::~decconfig()
{
  delete cfg;
}


uint32_t
decconfig::mpdmask()
{
  int ret = 0;
  uint32_t val = cfg->getSet<uint32_t>(ret, "mpdmask", current_rocnum);
  return val;
}

uint16_t
decconfig::apvmask()
{
  int ret = 0;
  uint16_t val = (cfg->getSet<uint32_t>(ret, "apvmask", current_rocnum)) & 0xFFFF;
  return val;
}

uint16_t
decconfig::minimum_baseline()
{
  int ret = 0;
  uint16_t val = (cfg->getSet<uint32_t>(ret, "minimum_baseline", current_rocnum) & 0xFFFF);
  return val;
}

uint16_t
decconfig::maximum_baseline()
{
  int ret = 0;
  uint16_t val = (cfg->getSet<uint32_t>(ret, "maximum_baseline", current_rocnum) & 0xFFFF);
  return val;
}

bool
decconfig::show_block_header(int mpd)
{
  int ret = 0;
  bool val = cfg->getShow<bool>(ret, "block_header", current_rocnum, mpd);

  return val;
}

bool
decconfig::show_block_trailer(int mpd)
{
  int ret = 0;
  bool val = cfg->getShow<bool>(ret, "block_trailer", current_rocnum, mpd);

  return val;
}

bool
decconfig::show_event_header(int mpd)
{
  int ret = 0;
  bool val = cfg->getShow<bool>(ret, "event_header", current_rocnum, mpd);

  return val;
}

bool
decconfig::show_trigger_time(int mpd)
{
  int ret = 0;
  bool val = cfg->getShow<bool>(ret, "trigger_time", current_rocnum, mpd);

  return val;
}

bool
decconfig::show_apv_header(int mpd, int apv)
{
  int ret = 0;
  bool val = cfg->getShow<bool>(ret, "apv_header", current_rocnum, mpd);

  return val;
}

bool
decconfig::show_apv_data(int mpd, int apv)
{
  int ret = 0;
  bool val = cfg->getShow<bool>(ret, "apv_data", current_rocnum, mpd);

  return val;
}

bool
decconfig::show_apv_trailer(int mpd, int apv)
{
  int ret = 0;
  bool val = cfg->getShow<bool>(ret, "apv_trailer", current_rocnum, mpd);

  return val;
}

bool
decconfig::show_event_trailer(int mpd)
{
  int ret = 0;
  bool val = cfg->getShow<bool>(ret, "event_trailer", current_rocnum, mpd);

  return val;
}

bool
decconfig::show_filler_word(int mpd)
{
  int ret = 0;
  bool val = cfg->getShow<bool>(ret, "filler_word", current_rocnum, mpd);

  return val;
}


bool
decconfig::check_apvmask(int mpd)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "apvmask", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_napv(int mpd)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "napv", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_event_count(int mpd)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "event_count", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_ndata(int mpd, int apv)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "ndata", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_sample_count(int mpd, int apv)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "sample_count", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_frame_count(int mpd, int apv)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "frame_count", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_baseline_value(int mpd, int apv)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "baseline_value", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_word_count(int mpd, int apv)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "word_count", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_n_words_in_event(int mpd)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "n_words_in_event", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_n_words_in_block(int mpd)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "n_words_in_block", current_rocnum, mpd);

  return val;
}

bool
decconfig::check_data_count(int mpd, int apv)
{
  int ret = 0;
  bool val = cfg->getCheck<bool>(ret, "data_count", current_rocnum, mpd);

  return val;
}
