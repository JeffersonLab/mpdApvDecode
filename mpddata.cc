//  class mpddata
//    Class to handle decoding of MPD and APV data.
//
//   Usage:
//     mpddata mdat = new mdata();
//     mdat->DecodeWord(uint32_t *buffer, int len);
//
//  Author: Bryan Moffit
//          Jefferson Lab Data Acquisition Group
//          September 2019
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mpddata.hh"

int
mpddata::ClearStats()
{
  memset(&mpd, 0, sizeof(mpd));

  return 1;
}

int
mpddata::DecodeWord(uint32_t data)
{
  int rval = 0;
  static uint32_t data_count = 0;
  int type_current = 0;
  generic_data_word_t gword;
  static uint16_t modID_current = 0;
  static uint16_t apvID_current = 0;
  static uint16_t apvmask = 0;
  static uint8_t napv = 0;

  gword.raw = data;

  type_current = gword.bf.data_type_tag;

  rval = type_current;
  switch( type_current )
    {
    case 0:		/* BLOCK HEADER */
      {
	mpd_block_header_t d; d.raw = data;

	if(cfg->show_block_header(d.bf.module_id))
	  {
	    printf("%08X - BLOCK HEADER - modID = %d   event_per_block = %d   block_count = %d\n",
		   d.raw,
		   d.bf.module_id,
		   d.bf.event_per_block,
		   d.bf.block_count);
	  }

	modID_current = d.bf.module_id;

	/* Reset APV stats for this module */
	apvmask = 0;
	napv = 0;

	break;
      }

    case 1:		/* BLOCK TRAILER */
      {
	mpd_block_trailer_t d; d.raw = data;

	if(cfg->show_block_trailer(modID_current))
	  {
	    printf("%08X - BLOCK TRAILER - n_words_in_block = %d\n",
		   d.raw,
		   d.bf.n_words_in_block);
	  }

	// Checks
	if(mpd[modID_current].have_stats & cfg->check_n_words_in_block(modID_current))
	  {
	    if(mpd[modID_current].n_words_in_block != d.bf.n_words_in_block)
	      {
		DEC_ERR("%2d: Current n_words_in_block != previous (%d != %d)\n",
			modID_current,
			d.bf.n_words_in_block,
			mpd[modID_current].n_words_in_block);
	      }
	  }

	if(mpd[modID_current].have_stats & cfg->check_apvmask(modID_current))
	  {
	    if(apvmask != mpd[modID_current].apvmask)
	      {
		DEC_ERR("%2d: Current apvmask != previous (0x%04x != 0x%04x)\n",
			modID_current,
			apvmask,
			mpd[modID_current].apvmask);
	      }
	  }

	if(mpd[modID_current].have_stats & cfg->check_napv(modID_current))
	  {
	    if(napv != mpd[modID_current].napv)
	      {
		DEC_ERR("%2d: Current napv != previous (%2d != %2d)\n",
			modID_current,
			napv,
			mpd[modID_current].napv);
	      }
	  }

	mpd[modID_current].have_stats = 1;
	mpd[modID_current].n_words_in_block = d.bf.n_words_in_block;
	mpd[modID_current].apvmask = apvmask;
	mpd[modID_current].napv = napv;

	apvmask = 0;
	napv = 0;
	modID_current = -1; // reset for next module
	apvID_current = -1;

	break;
      }

    case 2:		/* EVENT HEADER */
      {
	mpd_event_header_t d; d.raw = data;

	if(cfg->show_event_header(modID_current))
	  {
	    printf("%08X - EVENT HEADER - event_count = %d\n",
		   d.raw,
		   d.bf.event_count);
	  }

	// Checks
	if(mpd[modID_current].have_stats & cfg->check_event_count(modID_current))
	  {
	    if(d.bf.event_count <= mpd[modID_current].event_count)
	      {
		DEC_ERR("%2d: Current event_count <= previous (%d <= %d)\n",
			modID_current,
			d.bf.event_count,
			mpd[modID_current].event_count);
	      }
	  }

	mpd[modID_current].event_count = d.bf.event_count;

	break;
      }

    case 3:		/* TRIGGER TIME */
      {
	mpd_trigger_time_t d; d.raw = data;

	if(cfg->show_trigger_time(modID_current))
	  {
	    printf("%08X - TRIGGER TIME %d - coarse_trigger_time = %08x\n",
		   d.raw,
		   (d.bf.cont) ? 2 : 1,
		   d.bf.coarse_trigger_time );
	  }

	/* Each trigger time word is 20 bits.  First is first 20 bits. Second is last 20 bits */
	if(d.bf.cont == 0)
	  mpd[modID_current].trigger_time = d.bf.coarse_trigger_time;
	else
	  mpd[modID_current].trigger_time |= (d.bf.coarse_trigger_time << 20);

	break;
      }

    case 4:		/* APV CHANNEL DATA */
      {
	mpd_apv_channel_data_t d; d.raw = data;
	mpd_data_header_t dh;
	mpd_reduced_data_t rd;
	mpd_apv_trailer_t at;
	mpd_data_trailer_t dt;

	switch ( d.bf.proc_data_type )
	  {
	  case 0: /* HEADER */
	    dh.raw = data;

	    if(cfg->show_apv_header(modID_current, dh.bf.apv_id))
	      {
		printf("%08X - APV ",
		       d.raw);
		printf("HEADER: what %x  apv_header %x  apv_id %x\n",
		       dh.bf.what,
		       dh.bf.apv_header,
		       dh.bf.apv_id);
	      }
	    data_count = 0;
	    apvID_current = dh.bf.apv_id;

	    /* Add it to the mask, if not already there */
	    if (( (1 << apvID_current) & (apvmask)) == 0)
	      {
		apvmask |= (1 << apvID_current);
		napv++;
	      }

	    break;

	  case 1: /* Reduced Data */
	    rd.raw = data;

	    if(cfg->show_apv_data(modID_current, apvID_current))
	      {
		printf("%08X - APV ",
		       d.raw);
		printf("DATA (%3d): channel_number = %3d  data = 0x%03x (%4d)\n",
		       data_count,
		       rd.bf.channel_number,
		       rd.bf.data, rd.bf.data);
	      }
	    data_count++;
	    break;

	  case 2: /* APV Trailer */
	    at.raw = data;

	    if(cfg->show_apv_trailer(modID_current, apvID_current))
	      {
		printf("%08X - APV ",
		       d.raw);
		printf("TRAILER 1: mod_id = %d  sample_count = %d  frame_counter = %d\n",
		       at.bf.mod_id,
		       at.bf.sample_count,
		       at.bf.frame_counter);
	      }

	    // Checks
	    if(mpd[modID_current].have_stats & cfg->check_ndata(modID_current, apvID_current))
	      {
		if(data_count != mpd[modID_current].apv[apvID_current].ndata)
		  {
		    DEC_ERR("%2d: APV%2d: ndata != previous (%d != %d)\n",
			    modID_current, apvID_current,
			    data_count,
			    mpd[modID_current].apv[apvID_current].ndata);
		  }

	      }

	    if(mpd[modID_current].have_stats & cfg->check_sample_count(modID_current, apvID_current))
	      {
		if((at.bf.sample_count != 0) &&
		   (at.bf.sample_count <= mpd[modID_current].apv[apvID_current].sample_count))
		  {
		    DEC_ERR("%2d: APV%2d: sample_count <= previous (%d <= %d)\n",
			    modID_current, apvID_current,
			    at.bf.sample_count,
			    mpd[modID_current].apv[apvID_current].sample_count);

		  }

	      }

	    if(mpd[modID_current].have_stats & cfg->check_frame_count(modID_current, apvID_current))
	      {
		if((at.bf.frame_counter != 0) &&
		   (at.bf.frame_counter <= mpd[modID_current].apv[apvID_current].frame_count))
		  {
		    DEC_ERR("%2d: APV%2d: frame_count <= previous (%d <= %d)\n",
			    modID_current, apvID_current,
			    at.bf.frame_counter,
			    mpd[modID_current].apv[apvID_current].frame_count);
		  }
	      }

	    mpd[modID_current].apv[apvID_current].ndata = data_count;
	    mpd[modID_current].apv[apvID_current].sample_count = at.bf.sample_count;
	    mpd[modID_current].apv[apvID_current].frame_count = at.bf.frame_counter;

	    break;

	  case 3: /* Trailer */
	  default:
	    dt.raw = data;
	    if(cfg->show_apv_trailer(modID_current, apvID_current))
	      {
		printf("%08X - APV ",
		       d.raw);
		printf("TRAILER 2: baseline_value = %d  word_count = %d\n",
		       dt.bf.baseline_value,
		       dt.bf.word_count);
	      }

	    // Checks
	    if(mpd[modID_current].have_stats & cfg->check_baseline_value(modID_current, apvID_current))
	      {
		if( (dt.bf.baseline_value < cfg->minimum_baseline()) ||
		    (dt.bf.baseline_value > cfg->maximum_baseline()) )
		  {
		    DEC_ERR("%2d: APV%2d: baseline out of range (%d)\n",
			    modID_current, apvID_current,
			    dt.bf.baseline_value);
		  }
	      }

	    if(mpd[modID_current].have_stats & cfg->check_word_count(modID_current, apvID_current))
	      {
		if(dt.bf.word_count != mpd[modID_current].apv[apvID_current].word_count)
		  {
		    DEC_ERR("%2d: APV%2d: word_count != previous (%d != %d)\n",
			    modID_current, apvID_current,
			    dt.bf.word_count,
			    mpd[modID_current].apv[apvID_current].word_count);
		  }
	      }

	    if(mpd[modID_current].have_stats & cfg->check_data_count(modID_current, apvID_current))
	      {
		if(dt.bf.word_count - data_count != 3)
		  {
		    DEC_ERR("%2d: APV%2d: word_count - data_count != 3 (%d - %d != 3)\n",
			    modID_current, apvID_current,
			    dt.bf.word_count,
			    data_count);
		  }
	      }

	    mpd[modID_current].apv[apvID_current].baseline_value = dt.bf.baseline_value;
	    mpd[modID_current].apv[apvID_current].word_count = dt.bf.word_count;

	    break;
	  }
	break;
      }

    case 5:		/* EVENT TRAILER */
      {
	mpd_event_trailer_t d; d.raw = data;

	if(cfg->show_event_trailer(modID_current))
	  {
	    printf("%08X - EVENT TRAILER - n_words_in_event = %d  fine_trigger_time = %d\n",
		   d.raw,
		   d.bf.n_words_in_event,
		   d.bf.fine_trigger_time);
	  }

	// Checks;
	if(mpd[modID_current].have_stats & cfg->check_n_words_in_event(modID_current))
	  {
	    if(d.bf.n_words_in_event != mpd[modID_current].n_words_in_event)
	      {
		DEC_ERR("%2d: Current n_words_in_event != previous (%d != %d)\n",
			modID_current,
			d.bf.n_words_in_event,
			mpd[modID_current].n_words_in_event);
	      }
	  }

	mpd[modID_current].n_words_in_event = d.bf.n_words_in_event;
	break;
      }

    case 7:		/* FILLER WORD */
      {
	mpd_filler_t d; d.raw = data;

	if(cfg->show_filler_word(modID_current))
	  {
	    printf("%08X - FILLER WORD\n",
		   d.raw);
	  }
	break;
      }

    default:
      {
	printf("%08X - UNDEFINED TYPE = %d\n",
	       gword.raw,
	       gword.bf.data_type_tag);
	rval = -1;
	break;
      }

    }

  return rval;
}

int
mpddata::DecodeBuffer(const uint32_t *buffer, int len)
{
  int rval = 0, iword = 0;

  while(iword < len)
    {
      rval = DecodeWord(buffer[iword++]);
    }

  return rval;
}
