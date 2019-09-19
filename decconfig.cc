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

decconfig::decconfig(std::string filename)
{
  rocnum = -1;
  cfg = new GI_Config();
  cfg->parseFile("decode.cfg");
}

decconfig::~decconfig()
{
  delete cfg;
}

decconfig::SetRocNumber(int rocnum)
{
  current_rocnum = roc_num;
}
