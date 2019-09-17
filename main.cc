// parses event using stream parser

// builds a tree from selected nodes
// builds an index to banks in the event, searches index for particular banks

// in principle more than one bank may have the same tagNum,
//   in this case must use the getRange() method to get them all, see Doxygen doc


// ejw, 1-may-2012


#include <iostream>
#include <stdio.h>
#include "evioUtil.hxx"
#include "evioFileChannel.hxx"
#include "evioBankIndex.hxx"


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
  cout << "<program name> -f <evio filename> -C <config filename> -o <output filename>"
       << endl;
}

int
main(int argc, char **argv)
{
  InputParser input(argc, argv);
  int32_t mpdtag = 10, mpdnum = 0;
  uint64_t max_eventnumber = -1;
  string evio_filename,
    config_filename = "mpdApvDecode.cfg",
    out_filename = "out.dec";

  if(input.cmdOptionExists("-f"))
    {
      evio_filename = input.getCmdOption("-f").c_str();
    }

  if(input.cmdOptionExists("-C"))
    {
      config_filename = input.getCmdOption("-C").c_str();
    }

  if(input.cmdOptionExists("-o"))
    {
      out_filename = input.getCmdOption("-o").c_str();
    }

  if(input.cmdOptionExists("-n"))
    {
      max_eventnumber = strtoll(input.getCmdOption("-n").c_str(), NULL, 10);
    }

  if(evio_filename.empty())
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
    int stat = chan->ioctl("V",&version);
    if(stat == 0)
      cout << "# EVIO Version  " << version << endl;

    uint32_t *buffer, blen;
    evioDictEntry tn;

    while (chan->readAlloc((uint32_t **)&buffer, &blen))
      {
	uint64_t eventNumber=0;
	uint64_t eventTimestamp=0;
	uint16_t evType=0;
	const uint64_t *d64;
	const uint32_t *d32;
	const uint16_t *d16;
	evioBankIndex bi(buffer);

	int len;

	if(version < 4)
	  {
	    tn = evioDictEntry(0xC000, 0);
	    d32 = bi.getData < uint32_t >(tn, &len);
	    if(d32 != NULL)
	      {
		eventNumber = (uint32_t)d32[0];
		evType   = (uint32_t)d32[1];
	      }
	  }
	else
	  {
	    tn = evioDictEntry(0x2, // Eventnumber/timestamp bank
			       0, // num
			       0, // tagend
			       true, // hasparent
			       0xFF21, // parentTag
			       1, // parentNum
			       0, // parentTagEnd
			       EVIO_ULONG64,
			       false,
			       "",
			       "");
	    d64 = bi.getData < uint64_t >(tn, &len);
	    if(d64 != NULL)
	      {
		eventNumber    = d64[0];
		eventTimestamp = d64[1];
	      }

	    tn = evioDictEntry(0x2, // Event Type bank
			       0, // num
			       0, // tagend
			       true, // hasparent
			       0xFF21, // parentTag
			       1, // parentNum
			       0, // parentTagEnd
			       EVIO_USHORT16,
			       false,
			       "",
			       "");

	    // Bug in evioBankIndex::getData only checks on entry in the map..
	    // I guess we gotta do this.
	    bankIndexMap::const_iterator iter = bi.tagNumMap.find(tn);
	    while(iter != bi.tagNumMap.end())
	      {
		if( ((*iter).second).contentType == EVIO_USHORT16)
		  {
		    d16 = (const uint16_t *)((*iter).second).data;
		    len = ((*iter).second).dataLength;
		  }
		iter++;
	      }

	    if(d16 != NULL)
	      {
		evType    = d16[0];
	      }
	  }

	// Check to see if this event falls into the user specified range
	if((eventNumber > 0) && (eventNumber < max_eventnumber))
	  {
	    printf("# %6d: evType = %d   eventTimestamp = 0x%lx",
	    	   (uint32_t)eventNumber, evType, eventTimestamp);

	    if(evType == 0)
	      printf("    **************\n");
	    else
	      printf("\n");
	  }

	// Get the ROC / Payload data
	tn = evioDictEntry(mpdtag,
			   mpdnum,
			   0,
			   true,
			   0x4, // ROC number
			   1,
			   0,
			   EVIO_UINT32,
			   false,
			   "",
			   "");

	d32 = bi.getData < uint32_t >(tn, &len);

	if (d32 != NULL)
	  {
	    // deploy the data to the decoder
	    int myIndex = 2; // Unless there's a filler word

	    if((d32[0] & 0xF8000000) == 0xF8000000)
	      myIndex = 3;

	    uint32_t tiEvType = (d32[myIndex] & 0xFF000000) >> 24;

	    printf(" 0x%08x  0x%08x  0x%08x  0x%08x  0x%08x",
		   d32[myIndex-2], d32[myIndex],
		   d32[myIndex+1], d32[myIndex+2],
		   (d32[myIndex+3] & 0x3ff00)>>8);

	    if(tiEvType == 0)
	      printf("    **************\n");
	    else
	      printf("\n");
	  }

	free(buffer);
      }


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
