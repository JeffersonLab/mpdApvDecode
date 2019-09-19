/* configure.cpp

   g++ -Wall -I ../include -L/usr/local/lib -lconfig++ TConfig.cpp

*/

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string.h>

#include "GI_Config.h"

using namespace std;
using namespace libconfig;

/*
 *
 */

int main(int argc, char *argv[]) {


  GI_Config *cfg = new GI_Config();

 
  int ret;

  int val;
  std::string sval;


  cfg->parseInline(argc, argv);


  cout << " parse arguments " << endl;

  cfg->parseFile("../cfg/config_new.txt");

  cout << " Try replace" << endl;

  cfg->replace("12", "default.bus.[0].mpd.[0].adc.gain");

  cfg->insertInline();


  cout << " Insert" << endl;

  cfg->insert<int>(13, "prova.level1.niveau2.int");

  cfg->insert<float>(13.2, "ppinco.pallo.float");

  cfg->insert<std::string>("stringa", "ppinco.pallo.string");

  cfg->navigate();

  cout << " Test some methods: " << endl;

  val = cfg->getMPDLength(0);
  cout << " ... mpd elements = " << val << endl;

  val = cfg->getAPV<int>(ret, "Ipsf",18,1,0);

  cout << " ...apv.Ipsf = " << val << " " << ret << endl;

  val = cfg->getAPV<int>(ret, "i2c", 4, 0);
  cout << " ...apv.i2c = " << val << " " << ret << endl;

  sval = cfg->getBUS<std::string>(ret, "controller");
  cout << " bus.controller = " << sval.data() << " " << ret << endl;

  sval = cfg->getTop<std::string>(ret, "version");
  cout << " version = " << sval.data() << " "  << ret << endl;

  sval = cfg->getRUN<std::string>(ret, "mode");
  cout << " run.mode= " << sval << " " << ret << endl;

  val = cfg->getADC<int>(ret, "gain", 1, 2);
  cout << " mpd.adc.gain= " << val << " " << ret << endl;

  val = cfg->get<int>(ret, "bus", 0, "mpd", 0, "adc", 0, "gain", 0);
  cout << " bus.mpd.adc.gain= " << val << " " << ret << endl;

  val = cfg->get<int>(ret, "bus", "mpd", "adc", "gain");
  cout << " bus.mpd.adc.gain (scalar)= " << val << " " << ret << endl;

  sval = cfg->get<std::string>(ret, "time");
  cout << " time= " << sval << " " << ret << endl;

  val = cfg->get<int>(ret, "bus", 0, "mpd", 1, "adc", 0, "gain", 0);
  cout << " bus.mpd[1].adc.gain= " << val << " " << ret << endl;

  val = cfg->getAPV<int>(ret, "iii", 0, 0, 0);
  cout << " apv.iiix = " << val << " " << ret << endl;

  return 0;
  
}
