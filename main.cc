//  mpdApvDecode
//    Program to decode MPD and APV data taken with CODA.
//
//  Author: Bryan Moffit
//          Jefferson Lab Data Acquisition Group
//          September 2019
//
// original source taken from
// parses event using stream parser
//   evio-4.3/src/examples/etst12.cc
//   ejw, 1-may-2012


#include <iostream>
#include <stdio.h>
#include "evioUtil.hxx"
#include "evioFileChannel.hxx"
#include "evioBankIndex.hxx"
#include "mpddata.hh"
#include "decconfig.hh"

using namespace evio;
using namespace std;

class InputParser
{
public:
  InputParser(int &argc, char **argv)
  {
    for (int i = 1; i < argc; ++i)
      this->tokens.push_back(string(argv[i]));
  }
  /// @author iain
  const string getCmdOption(const string & option) const
  {
    vector < string >::const_iterator itr;
    itr = find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end())
      {
	return *itr;
      }
    return "";
  }
  /// @author iain
  bool cmdOptionExists(const string & option) const
  {
    return find(this->tokens.begin(), this->tokens.end(), option)
      != this->tokens.end();
  }
private:
    vector < string > tokens;
};

//--------------------------------------------------------------
//--------------------------------------------------------------
void
Usage()
{
  cout <<
    "<program name> -f <evio filename> -C <config filename> -o <output filename>"
    << endl;
}

int
main(int argc, char **argv)
{
  InputParser input(argc, argv);
  uint64_t max_eventnumber = -1, skip_eventnumber = 0;
  string evio_filename,
    config_filename = "decode.cfg", out_filename = "out.dec";
  bool showNoData = false;

  if (input.cmdOptionExists("-f"))
    {
      evio_filename = input.getCmdOption("-f").c_str();
    }

  if (input.cmdOptionExists("-C"))
    {
      config_filename = input.getCmdOption("-C").c_str();
    }

  if (input.cmdOptionExists("-o"))
    {
      out_filename = input.getCmdOption("-o").c_str();
    }

  if (input.cmdOptionExists("-s"))
    {
      skip_eventnumber = strtoll(input.getCmdOption("-s").c_str(), NULL, 10);
    }

  if (input.cmdOptionExists("-m"))
    {
      max_eventnumber = strtoll(input.getCmdOption("-m").c_str(), NULL, 10);
    }

  if (input.cmdOptionExists("-e"))
    {
      showNoData = true;
    }

  if (evio_filename.empty())
    {
      cerr << "ERROR: filename required" << endl;
      Usage();
      exit(-1);
    }

  try
  {

    // create and open file channel
    evioFileChannel *chan = new evioFileChannel(evio_filename, "r");
    chan->open();

    int32_t version = 0;
    int stat = chan->ioctl("V", &version);
    if (stat == 0)
      cout << "# EVIO Version  " << version << endl;

    uint32_t *buffer, blen;
    evioDictEntry tn;

    decconfig *cfg = new decconfig(config_filename);
    cfg->show_no_data(showNoData);

    mpddata *mdat[16];
    for(int iroc = 0; iroc < 16; iroc++)
      {
	mdat[iroc] = new mpddata(*cfg);
    	mdat[iroc]->ClearStats();
      }


    while (chan->readAlloc((uint32_t **) &buffer, &blen))
      {
	uint64_t eventNumber = 0;
	uint64_t eventTimestamp = 0;
	uint16_t evType = 0;
	const uint64_t *d64;
	const uint32_t *d32;
	const uint16_t *d16;

	evioBankIndex ebi(buffer,1);

	bankIndexMap::const_iterator b_iter;

	// Get the tag numbers that are not FF50 or FF21
	// These should be the ROC banks.
	bankIndex bi_roc;
	map <int, bankIndex> rocBiMap;
	for(b_iter = ebi.tagNumMap.begin(); b_iter != ebi.tagNumMap.end();
	    b_iter++)
	  {
	    if((b_iter->first.getTag() != 0xFF50) &&
	       (b_iter->first.getTag() != 0xFF21))
	      {
#define DEBUG_BANKS
#ifdef DEBUG_BANKS
		cout << "In bi: tag = 0x" << hex << b_iter->first.getTag()
		     << " type = 0x" << hex << b_iter->second.contentType << endl;
#endif
		rocBiMap.insert( pair <int, bankIndex>(b_iter->first.getTag(), b_iter->second) );
		bi_roc = b_iter->second;
	      }
	  }

	int len;
	if (version < 4)
	  {
	    tn = evioDictEntry(0xC000, 0);
	    d32 = ebi.getData < uint32_t > (tn, &len);
	    if (d32 != NULL)
	      {
		eventNumber = (uint32_t) d32[0];
		evType = (uint32_t) d32[1];
	      }
	  }
	else
	  {
	    tn = evioDictEntry(0xFF21,   // Trigger Bank Tag
			       1 ); // bank num

	    if(!ebi.tagNumExists(tn))
	      {
		continue;
	      }

	    bankIndex bi_trigbank = ebi.getBankIndex(tn);
	    evioBankIndex bi_triggerbank((uint32_t *)bi_trigbank.bankPointer, 4);

	    evioBankIndex bi_evTypeBank;
	    for(b_iter = bi_triggerbank.tagNumMap.begin(); b_iter != bi_triggerbank.tagNumMap.end();
		b_iter++)
	      {
		if((b_iter->first.getTag() == 0x2) &&
		   (b_iter->second.contentType == EVIO_USHORT16))
		  {
		    d16 = (uint16_t *)b_iter->second.data;
		    len = b_iter->second.dataLength;
		    if (d16 != NULL)
		      {
			evType = d16[0];
		      }
		  }
	      }

	    tn = evioDictEntry(0x2,  // Eventnumber/timestamp Bank Tag=0x2
			       0, // num
			       0, // tag en
			       EVIO_ULONG64,
			       true,
			       "",
			       "");


	    if(!bi_triggerbank.tagNumExists(tn))
	      {
		continue;
	      }

	    d64 = bi_triggerbank.getData < uint64_t > (tn, &len);
	    if (d64 != NULL)
	      {
		eventNumber = d64[0];
		eventTimestamp = d64[1];
	      }

	  }

	// Check to see if this event falls into the user specified range
	if ((eventNumber > skip_eventnumber)
	    && (eventNumber <= max_eventnumber))
	  {
	    printf("# %6d: evType = %d   eventTimestamp = 0x%lx \n",
		   (uint32_t) eventNumber, evType, eventTimestamp);

	    int rocnum = 4;
	    if(rocBiMap.find(rocnum) == rocBiMap.end())
	      {
		cout << "#  NO ROC " << rocnum << endl;
		continue;
	      }

	    evioBankIndex ebi_roc((uint32_t *)rocBiMap.find(4)->second.bankPointer,0);
	    // Get the ROC / Payload data
	    int32_t mpdtag = 10, mpdnum = 0;
	    tn = evioDictEntry(mpdtag, mpdnum);

	    d32 = ebi_roc.getData < uint32_t > (tn, &len);

	    if (d32 != NULL)
	      {
		printf("#      ROC %2d MPD+APV Data\n",
		       rocnum);
		mdat[rocnum]->DecodeBuffer(d32, len);
	      }
	    else
	      {
		printf("#      NO ROC %2d d32 !!! \n",
		       rocnum);
	      }

	  }
	free(buffer);
      }


    for(int iroc = 0; iroc < 16; iroc++)
      delete mdat[iroc];
  }
  catch(evioException e)
  {
    cerr << e.toString() << endl;
    exit(EXIT_FAILURE);
  }

  exit(0);
  printf("%s", DataTypeNames[0]);
}


//--------------------------------------------------------------
//--------------------------------------------------------------
