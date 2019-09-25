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
  rocIndex = -1;
  cfg = new GI_Config();
  cfg->parseFile(filename);
  showOverride = true;
}

decconfig::~decconfig()
{
  delete cfg;
}

uint8_t
decconfig::mpd_bank_tag()
{
  int ret = 0;
  uint8_t val = cfg->getRoc<uint32_t>(ret, "mpd_bank_tag", rocIndex);
  return val;
}

uint8_t
decconfig::mpd_bank_num()
{
  int ret = 0;
  uint8_t val = cfg->getRoc<uint32_t>(ret, "mpd_bank_num", rocIndex);
  return val;
}

uint32_t
decconfig::mpdmask()
{
  int ret = 0;
  uint32_t val = cfg->getSet<uint32_t>(ret, "mpdmask", rocIndex);
  return val;
}

uint16_t
decconfig::apvmask()
{
  int ret = 0;
  uint16_t val = (cfg->getSet<uint32_t>(ret, "apvmask", rocIndex)) & 0xFFFF;
  return val;
}

uint16_t
decconfig::minimum_baseline()
{
  int ret = 0;
  uint16_t val = (cfg->getSet<uint32_t>(ret, "minimum_baseline", rocIndex) & 0xFFFF);
  return val;
}

uint16_t
decconfig::maximum_baseline()
{
  int ret = 0;
  uint16_t val = (cfg->getSet<uint32_t>(ret, "maximum_baseline", rocIndex) & 0xFFFF);
  return val;
}

void
decconfig::show_no_data(int enable)
{
  if(enable)
    showOverride = false;
}

bool
decconfig::show_block_header(int mpd)
{

  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getShow<bool>(ret, "block_header", rocIndex);

  return (val & showOverride);
}

bool
decconfig::show_block_trailer(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getShow<bool>(ret, "block_trailer", rocIndex);

  return (val & showOverride);
}

bool
decconfig::show_event_header(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getShow<bool>(ret, "event_header", rocIndex);

  return (val & showOverride);
}

bool
decconfig::show_trigger_time(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getShow<bool>(ret, "trigger_time", rocIndex);

  return (val & showOverride);
}

bool
decconfig::show_apv_header(int mpd, int apv)
{
  int ret = 0;
  bool val = false;

  if( ((1 << mpd) & mpdmask() ) && ((1 << apv) & apvmask() ))
    val = cfg->getShow<bool>(ret, "apv_header", rocIndex);

  return (val & showOverride);
}

bool
decconfig::show_apv_data(int mpd, int apv)
{
  int ret = 0;
  bool val = false;

  if( ((1 << mpd) & mpdmask() ) && ((1 << apv) & apvmask() ))
    val = cfg->getShow<bool>(ret, "apv_data", rocIndex);

  return (val & showOverride);
}

bool
decconfig::show_apv_trailer(int mpd, int apv)
{
  int ret = 0;
  bool val = false;

  if( ((1 << mpd) & mpdmask() ) && ((1 << apv) & apvmask() ))
    val = cfg->getShow<bool>(ret, "apv_trailer", rocIndex);

  return (val & showOverride);
}

bool
decconfig::show_event_trailer(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getShow<bool>(ret, "event_trailer", rocIndex);

  return (val & showOverride);
}

bool
decconfig::show_filler_word(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getShow<bool>(ret, "filler_word", rocIndex);

  return (val & showOverride);
}


bool
decconfig::check_apvmask(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getCheck<bool>(ret, "apvmask", rocIndex);

  return val;
}

bool
decconfig::check_napv(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getCheck<bool>(ret, "napv", rocIndex);

  return val;
}

bool
decconfig::check_event_count(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getCheck<bool>(ret, "event_count", rocIndex);

  return val;
}

bool
decconfig::check_ndata(int mpd, int apv)
{
  int ret = 0;
  bool val = false;

  if( ((1 << mpd) & mpdmask() ) && ((1 << apv) & apvmask() ))
    val = cfg->getCheck<bool>(ret, "ndata", rocIndex);

  return val;
}

bool
decconfig::check_sample_count(int mpd, int apv)
{
  int ret = 0;
  bool val = false;

  if( ((1 << mpd) & mpdmask() ) && ((1 << apv) & apvmask() ))
    val = cfg->getCheck<bool>(ret, "sample_count", rocIndex);

  return val;
}

bool
decconfig::check_frame_count(int mpd, int apv)
{
  int ret = 0;
  bool val = false;

  if( ((1 << mpd) & mpdmask() ) && ((1 << apv) & apvmask() ))
    val = cfg->getCheck<bool>(ret, "frame_count", rocIndex);

  return val;
}

bool
decconfig::check_baseline_value(int mpd, int apv)
{
  int ret = 0;
  bool val = false;

  if( ((1 << mpd) & mpdmask() ) && ((1 << apv) & apvmask() ))
    val = cfg->getCheck<bool>(ret, "baseline_value", rocIndex);

  return val;
}

bool
decconfig::check_word_count(int mpd, int apv)
{
  int ret = 0;
  bool val = false;

  if( ((1 << mpd) & mpdmask() ) && ((1 << apv) & apvmask() ))
    val = cfg->getCheck<bool>(ret, "word_count", rocIndex);

  return val;
}

bool
decconfig::check_n_words_in_event(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getCheck<bool>(ret, "n_words_in_event", rocIndex);

  return val;
}

bool
decconfig::check_n_words_in_block(int mpd)
{
  int ret = 0;
  bool val = false;

  if( (1 << mpd) & mpdmask() )
    val = cfg->getCheck<bool>(ret, "n_words_in_block", rocIndex);

  return val;
}

bool
decconfig::check_data_count(int mpd, int apv)
{
  int ret = 0;
  bool val = false;

  if( ((1 << mpd) & mpdmask() ) && ((1 << apv) & apvmask() ))
    val = cfg->getCheck<bool>(ret, "data_count", rocIndex);

  return val;
}
