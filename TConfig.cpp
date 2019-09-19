/* TConfig.cpp
   Test reading of decode.cfg with decconfig class

   g++ -Wall -I -lconfig++ TConfig.cpp decconfig.o

*/

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string.h>

#include "decconfig.hh"

using namespace std;
using namespace libconfig;

/*
 *
 */

int main(int argc, char *argv[]) {


  decconfig *cfg = new decconfig("decode.cfg");


  int ret;
  cout << " Test some methods: " << endl;

  bool isIt = cfg->show_block_header(1);
  cout << " ...show_block_header = " << (isIt ? "true" : "false") << endl;

  uint32_t val;

  val = cfg->mpdmask();
  cout << "... mpdmask = " << hex << val << endl;

  val = cfg->apvmask();
  cout << "... apvmask = " << hex << val << endl;

  val = cfg->minimum_baseline();
  cout << "... minimum_baseline = " << dec << val << endl;

  val = cfg->maximum_baseline();
  cout << "... maximum_baseline = " << dec << val << endl;

  return 0;

  if(ret) {}

  if(val) {}
}
