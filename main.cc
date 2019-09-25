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
      DEC_SHOW("# EVIO Version %d\n", version);



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


    bool stop = false;

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

	int len;
	bankIndex bi_trigger, bi_roc;
	map <int, bankIndex> rocBiMap;

	memset(&bi_trigger, 0, sizeof(bi_trigger));
	memset(&bi_roc, 0, sizeof(bi_roc));

	if (version < 4)
	  {
	    for(b_iter = ebi.tagNumMap.begin(); b_iter != ebi.tagNumMap.end();
		b_iter++)
	      {
#ifdef DEBUG_BANKS
		DEC_SHOW("#  In bi: tag = 0x%x   type = 0x%x\n",
			 b_iter->first.getTag(),
			 b_iter->second.contentType);
#endif
		switch (b_iter->first.getTag())
		  {
		  case 0xC000: // Event Bank
		    d32 = ebi.getData < uint32_t > (tn, &len);
		    if (d32 != NULL)
		      {
			eventNumber = (uint32_t) d32[0];
			evType = (uint32_t) d32[1];
		      }
		    break;

		  default: // ROC Banks
		    rocBiMap.insert( pair <int, bankIndex>(b_iter->first.getTag(), b_iter->second) );
		    bi_roc = b_iter->second;

		  }
	      }
	  }
	else
	  {
	    for(b_iter = ebi.tagNumMap.begin(); b_iter != ebi.tagNumMap.end();
		b_iter++)
	      {
#ifdef DEBUG_BANKS
		DEC_SHOW("#  In bi: tag = 0x%x  type = 0x%x\n",
			 b_iter->first.getTag(),
			 b_iter->second.contentType);
#endif
		switch (b_iter->first.getTag())
		  {
		  case 0xFF50: // Event Bank
		    break;

		  case 0xFF21: // Trigger Bank
		    bi_trigger = b_iter->second;
		    break;

		  default: // ROC Banks
		    rocBiMap.insert( pair <int, bankIndex>(b_iter->first.getTag(), b_iter->second) );
		    bi_roc = b_iter->second;

		  }
	      }

	    if(bi_trigger.bankLength > 0)
	      {
		evioBankIndex ebi_trigger((uint32_t *)bi_trigger.bankPointer, 4);

		for(b_iter = ebi_trigger.tagNumMap.begin();
		    b_iter != ebi_trigger.tagNumMap.end();
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
#ifdef DEBUG_BANKS
			    DEC_SHOW("#  evType = %d\n");
#endif // DEBUG_BANKS

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

		if(!ebi_trigger.tagNumExists(tn))
		  {
		    continue;
		  }

		d64 = ebi_trigger.getData < uint64_t > (tn, &len);
		if (d64 != NULL)
		  {
		    eventNumber = d64[0];
		    eventTimestamp = d64[1];
#ifdef DEBUG_BANKS
		    DEC_SHOW("#  eventNumber = %d  eventTimestamp = 0x%x\n",
			     eventNumber,
			     eventTimestamp);
#endif // DEBUG_BANKS
		  }

	      }
	  }

	// Check to see if this event is skipped
	if (eventNumber > skip_eventnumber)
	  {
	    DEC_SHOW("# eventNumber = %lu   evType = %d   eventTimestamp = 0x%lx\n",
		     eventNumber, evType, eventTimestamp);

	    // Get the ROC / Payload data
	    map <int, bankIndex>::iterator rocIter;

	    for(rocIter = rocBiMap.begin(); rocIter != rocBiMap.end(); ++rocIter)
	      {
		int rocnum = rocIter->first;
		cfg->set_roc_number(rocnum);

		uint32_t mpdtag = cfg->mpd_bank_tag();
		uint16_t mpdnum = cfg->mpd_bank_num();

#ifdef DEBUG_BANKS
		DEC_SHOW("# roc = %d  mpdmask = 0x%x  apvmask = 0x%x\n",
			 rocnum, cfg->mpdmask(), cfg->apvmask());
#endif // DEBUG_BANKS

		evioBankIndex ebi_roc((uint32_t *)rocIter->second.bankPointer,0);
		tn = evioDictEntry(mpdtag, mpdnum);

		d32 = ebi_roc.getData < uint32_t > (tn, &len);

		if (d32 != NULL)
		  {
		    DEC_SHOW("#  ROC %d  MPD+APV Data\n",rocnum);
		    mdat[rocnum]->DecodeBuffer(d32, len);
		  }
		else
		  {
		    DEC_SHOW("#      NO ROC %2d data found for mpdtag = %d  mpdnum = %d \n",
			     rocnum,
			     mpdtag, mpdnum);
		  }
	      }
	  }

	if (eventNumber == max_eventnumber)
	  {
	    stop = true;
	  }

	free(buffer);

	if(stop)
	  {
	    break;
	  }
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
