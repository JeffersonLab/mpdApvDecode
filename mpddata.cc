#include <stdio.h>
#include <stdint.h>
#include "mpddata.hh"

int
mpddata::Clear_Stats()
{
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

	printf("%08X - BLOCK HEADER - modID = %d   event_per_block = %d   block_count = %d\n",
	       d.raw,
	       d.bf.module_id,
	       d.bf.event_per_block,
	       d.bf.block_count);

	modID_current = d.bf.module_id;

	/* Reset APV stats for this module */
	apvmask = 0;
	napv = 0;

	break;
      }

    case 1:		/* BLOCK TRAILER */
      {
	mpd_block_trailer_t d; d.raw = data;

	printf("%08X - BLOCK TRAILER - n_words_in_block = %d\n",
	       d.raw,
	       d.bf.n_words_in_block);

	// FIXME: Compare these values before overwritting
	// Check apvmask, napv vs previous

	modID_current = -1; // reset for next module
	apvID_current = -1;

	break;
      }

    case 2:		/* EVENT HEADER */
      {
	mpd_event_header_t d; d.raw = data;

	printf("%08X - EVENT HEADER - event_count = %d\n",
	       d.raw,
	       d.bf.event_count);

	// FIXME: Compare these values before overwritting (should inc by 1)
	mpd[modID_current].event_count = d.bf.event_count;
	break;
      }

    case 3:		/* TRIGGER TIME */
      {
	mpd_trigger_time_t d; d.raw = data;
	printf("%08X - TRIGGER TIME %d - coarse_trigger_time = %08x\n",
	       d.raw,
	       (d.bf.cont) ? 2 : 1,
	       d.bf.coarse_trigger_time );

	// FIXME: Check that the shift is correct for the continution
	if(d.bf.cont)
	  mpd[modID_current].trigger_time |= (d.bf.coarse_trigger_time << 20);
	else
	  mpd[modID_current].trigger_time = d.bf.coarse_trigger_time;
	break;
      }

    case 4:		/* APV CHANNEL DATA */
      {
	mpd_apv_channel_data_t d; d.raw = data;
	mpd_data_header_t dh;
	mpd_reduced_data_t rd;
	mpd_apv_trailer_t at;
	mpd_data_trailer_t dt;

	printf("%08X - APV ",//- proc_data_type = %d  processed_data = %x\n",
	       d.raw);/* , */
	/* d.bf.proc_data_type, */
	/* d.bf.processed_data); */

	switch ( d.bf.proc_data_type )
	  {
	  case 0: /* HEADER */
	    dh.raw = data;

	    printf("HEADER: what %x  apv_header %x  apv_id %x\n",
		   dh.bf.what,
		   dh.bf.apv_header,
		   dh.bf.apv_id);
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

	    printf("DATA (%3d): channel_number = %3d  data = 0x%03x (%4d)\n",
		   data_count++,
		   rd.bf.channel_number,
		   rd.bf.data, rd.bf.data);
	    break;

	  case 2: /* APV Trailer */
	    at.raw = data;
	    printf("TRAILER 1: mod_id = %d  sample_count = %d  frame_counter = %d\n",
		   at.bf.mod_id,
		   at.bf.sample_count,
		   at.bf.frame_counter);

	    // FIXME: Compare these values before overwritting
	    mpd[modID_current].apv[apvID_current].ndata = data_count;
	    mpd[modID_current].apv[apvID_current].sample_count = at.bf.sample_count;
	    mpd[modID_current].apv[apvID_current].frame_count = at.bf.frame_counter;

	    break;

	  case 3: /* Trailer */
	  default:
	    dt.raw = data;
	    printf("TRAILER 2: baseline_value = %d  word_count = %d\n",
		   dt.bf.baseline_value,
		   dt.bf.word_count);

	    // FIXME: Compare these values before overwritting
	    // Check that word_count - data_count = 4.
	    mpd[modID_current].apv[apvID_current].baseline_value = dt.bf.baseline_value;
	    mpd[modID_current].apv[apvID_current].frame_count = dt.bf.word_count;

	    break;
	  }
	break;
      }

    case 5:		/* EVENT TRAILER */
      {
	mpd_event_trailer_t d; d.raw = data;

	printf("%08X - EVENT TRAILER - n_words_in_event = %d  fine_trigger_time = %d\n",
	       d.raw,
	       d.bf.n_words_in_event,
	       d.bf.fine_trigger_time);
	break;
      }

    case 7:		/* FILLER WORD */
      {
	mpd_filler_t d; d.raw = data;

	printf("%08X - FILLER WORD\n",
	       d.raw);
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
