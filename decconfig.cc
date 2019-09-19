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

bool
decconfig::show_block_header(int mpd)
{
  return true;
}

bool
decconfig::show_block_trailer(int mpd)
{
  return true;
}

bool
decconfig::show_event_header(int mpd)
{
  return true;
}

bool
decconfig::show_trigger_time(int mpd)
{
  return true;
}

bool
decconfig::show_apv_header(int mpd, int apv)
{
  return true;
}

bool
decconfig::show_apv_data(int mpd, int apv)
{
  return true;
}

bool
decconfig::show_apv_trailer(int mpd, int apv)
{
  return true;
}

bool
decconfig::show_event_trailer(int mpd)
{
  return true;
}

bool
decconfig::show_filler_word(int mpd)
{
  return true;
}


bool
decconfig::check_apvmask(int mpd)
{
  return true;
}

bool
decconfig::check_napv(int mpd)
{
  return true;
}

bool
decconfig::check_event_count(int mpd)
{
  return true;
}

bool
decconfig::check_ndata(int mpd, int apv)
{
  return true;
}

bool
decconfig::check_sample_count(int mpd, int apv)
{
  return true;
}

bool
decconfig::check_frame_count(int mpd, int apv)
{
  return true;
}

bool
decconfig::check_baseline_value(int mpd, int apv)
{
  return true;
}

bool
decconfig::check_word_count(int mpd, int apv)
{
  return true;
}

bool
decconfig::check_n_words_in_event(int mpd)
{
  return true;
}

bool
decconfig::check_n_words_in_block(int mpd)
{
  return true;
}

bool
decconfig::check_data_count(int mpd, int apv)
{
  return true;
}


int
decconfig::minimum_baseline()
{
  return 1;
}

int
decconfig::maximum_baseline()
{
  return 1;
}
