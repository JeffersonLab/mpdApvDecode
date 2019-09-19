
#ifndef __GI_CONFIG__
#define __GI_CONFIG__

/*
 * GI_Config.h
 *
 * Author: E. Cisbani
 * First version: March 2012
 *
 * Read config file, online runtime parameters; used as slow
 * control data DB
 *
 * From the original idea of Config.* by P. Musico
 * Note: template in c++ template definition and implemetantion
 * must be in the same file
 *
 * Require libconfg++ version 1.4.8 (http://www.hyperrealm.com/libconfig/)
 *
 * (Test: g++ -Wall -L/usr/local/lib -lconfig++ TConfig.cpp )
 *
 */

#include <iomanip>
#include <cstdlib>
#include <string.h>
#include <sstream>
#include <typeinfo>
#include <iostream>

#include <libconfig.h++>

#undef GICONF_DEBUG
//#define GICONF_DEBUG 1

using namespace std;
using namespace libconfig;

class GI_Config
{

private:
  Config cfg;
  std::string fName;

  // cli argument array
  std::string * fargp, *fargv;
  int fargc;



  //:::::::::::::::::::::::::::::::::::::::::::::

  void manageXError(int nlev, int *idx, std::string * rdx, int ret)
  {
    cout << __FUNCTION__ << ": ";
    for (int i = 0; i < nlev; i++)
      {
	cout << rdx[i].data() << "[" << idx[i] << "].";
      }
    cout << " does not exist ... exit(0x" << (hex) << ret << " " << (dec) <<
      ret << ")" << endl;
    exit(ret);
  }

  /*................
   * Search setting, if nothing found (in specific and in default) return NULL
   */
  Setting *searchX(int &berr, int nlev, int *idx, std::string * rdx,
		   int &length)
  {

    int ret;

    int ll;

    Setting *bs[2], *ps;

    Setting *ss = &(cfg.getRoot());
    bs[0] = ss;
    bs[1] = &((*ss)["default"]);
#ifdef GICONF_DEBUG
    cout << __FUNCTION__ << " " << rdx[nlev - 1] << " " << nlev << endl;
#endif
    ps = NULL;

    for (int k = 0; k < 2; k++)
      {				// loop on specific / default setting

	ret = 0;
	ps = bs[k];

	for (int i = 0; i < nlev; i++)
	  {
#ifdef GICONF_DEBUG
	    cout << __FUNCTION__ << " " << k << "/" << i << " " << rdx[i] <<
	      " check " << ret << endl;
#endif
	    if (!(ps->exists(rdx[i].data())))
	      {			// check if param exist
		ret = 1;
		berr |= (2 * k + 1) << (i * 2);
		break;
	      }

	    if (ret == 0)
	      {
		if ((*ps)[rdx[i].data()].isArray()
		    || (*ps)[rdx[i].data()].isList())
		  {		// list should be enough
		    ll = (*ps)[rdx[i].data()].getLength();
		    if (idx[i] >= ll)
		      {		// check boundary
			if ((ll <= 1) && (k == 1))
			  {	// assume default scalar or one element list
			    ps = &((*ps)[rdx[i].data()][0]);
			  }
			else
			  {
			    ret = 1;
			    berr |= (((2 * k + 1) << (i * 2)) | 0x20000000);
			    break;
			  }
		      }
		    else
		      {
			ps = &((*ps)[rdx[i].data()][idx[i]]);
		      }
		  }
		else
		  {		// is scalar or group
		    ps = &((*ps)[rdx[i].data()]);
		  }
	      }

#ifdef GICONF_DEBUG
	    cout << __FUNCTION__ << " " << k << "/" << i <<
	      " loop passed (def/spe) " << (*ps).
	      getPath() << " berr= 0x" << (hex) << berr << (dec) << endl;
#endif
	  }			// loop on i

	if ((k == 0) && (ret == 0))
	  {
	    break;
	  }			// specific setting exist, no need to loop on default

	if ((k == 1) && (ret == 1))
	  {
	    berr |= 0x80000000;
	    ps = NULL;
	    break;
	  }

      }				// loop of k

    length = ll;
    return ps;

  };

  /**
   * Generic method to get a parameter; pretty general
   * value : is the returned value of the parameter
   * name  : the name of the parameter
   * elx   : the element (if parameter is an array)
   * nlev  : the number of sub-groups
   * idx   : the element of each subgrops (not significant if the sub group is of scalar nature)
   * rdx   : the name of the subgroup
   *
   * the method return the error code (0 on success)
   *
   * note: the order of idx and rdx is the same of the config file (is opposite to the order in the public get methods)
   *
   * example:
   *   to get: bus[0].mpd[3].apv[4].mode
   *
   * getX(value, "mode", 0, 3, {0,3,4}, {"bus","mpd","apv"});
   *
   *
   * try specific setting; if does not exist or is out of range try default.
   * if default does not exist or (out of range and default is vector), get error
   *
   * returned warning/error:
   * 2 bits for each level (1 = error in specific setting, 2 = error in default setting, 3 = error on both)
   * | 0x80000000 - error (<0)
   * | 0x20000000 - out of range
   * | 0x40000000 - type not recognized
   *
   bits: 01   23   45   67   89
        a[1].b[2].c[3].d[4].n[m]
        v v 0 -> exist and in range
        v x 1 -> exist but out of range, use default -> a.b.c ...
        x x 3 -> try default, then error
        x v 3 -> impossible / error

(*)
 1 if default is a vector and m is in range, use default
 3 if default is a vector and m is out of range, raise error
 2 if default is a scalar, use default

  */

  int getX(void *value, int nlev, int *idx, std::string * rdx)
  {

    int *ival;
    float *fval;
    std::string * sval;
    bool *bval;

    int berr = 0;		// errors

    Setting *bs[2], *ps;

    Setting *ss = &(cfg.getRoot());
    bs[0] = ss;
    bs[1] = &((*ss)["default"]);

#ifdef GICONF_DEBUG
    cout << __FUNCTION__ << " " << rdx[nlev - 1] << " " << nlev << endl;
#endif

    int size;
    try
    {

      ps = searchX(berr, nlev, idx, rdx, size);

      if (ps == NULL)
	{
	  throw berr;
	}
      else
	{

	  Setting::Type tt = ps->getType();

	  tt = ps->getType();
	  switch (tt)
	    {
	    case Setting::TypeInt:
	    case Setting::TypeInt64:
	      ival = (int *) value;
	      *ival = 0;
	      cfg.lookupValue(ps->getPath(), *ival);
	      break;
	    case Setting::TypeFloat:
	      fval = (float *) value;
	      *fval = 0.;
	      cfg.lookupValue(ps->getPath(), *fval);
	      break;
	    case Setting::TypeString:
	      sval = (std::string *) value;
	      *sval = "";
	      cfg.lookupValue(ps->getPath(), *sval);
	      break;
	    case Setting::TypeBoolean:
	      bval = (bool *) value;
	      *bval = 0;
	      cfg.lookupValue(ps->getPath(), *bval);
	      break;
	    default:
	      berr |= 0xC0000000;
	      throw berr;
	      break;
	    }
	}

    }
    catch(int er)
    {
      manageXError(nlev, idx, rdx, er);
    }

    return berr;

  }

  //

  int getX(void *value, int nlev, int *idx, const char **rdx)
  {

    std::string * ss;
    ss = new std::string[nlev];

    for (int i = 0; i < nlev; i++)
      {
	ss[i] = rdx[i];
      }
    return getX(value, nlev, idx, ss);
  }

  //
  //

  Setting::Type getType(int &berr, int nlev, int *idx, std::string * rdx)
  {

    Setting::Type sety;

    Setting *ps;

#ifdef GICONF_DEBUG
    cout << __FUNCTION__ << " " << rdx[nlev - 1] << " " << nlev << endl;
#endif

    int size;
    ps = searchX(berr, nlev, idx, rdx, size);

    if (ps != NULL)
      {
	// cout << __FUNCTION__ << " " << ps->getPath() << " " << ps->getType() << " " << berr << endl;
	sety = ps->getType();
      }
    else
      {				// setting does not exist, cannot update
	sety = Setting::TypeNone;
#ifdef GICONF_DEBUG
	cout << __FUNCTION__ << " setting does not exist" << endl;
#endif
      }

    return sety;

  }

  /*
   * Return number of elements of a given setting (-1 if not found)
   *
   */
  int getLength(int nlev, int *idx, std::string * rdx)
  {

    int size;
    int berr;

    Setting *ps;

#ifdef GICONF_DEBUG
    cout << __FUNCTION__ << " " << rdx[nlev - 1] << " " << nlev << endl;
#endif

    ps = searchX(berr, nlev, idx, rdx, size);

    if (ps == NULL)
      {
	// cout << __FUNCTION__ << " " << ps->getPath() << " " << ps->getType() << " " << berr << endl;
	size = -1;
#ifdef GICONF_DEBUG
	cout << __FUNCTION__ << " setting does not exist" << endl;
#endif
      }

    return size;

  }

  int getLength(int nlev, int *idx, const char **rdx)
  {

    std::string * ss;
    ss = new std::string[nlev];

    for (int i = 0; i < nlev; i++)
      {
	ss[i] = rdx[i];
      }
    return getLength(nlev, idx, ss);
  }


  // from string to T ( see http://forums.codeguru.com/showthread.php?t=231054)
  template < class T > bool from_string(T & t, const std::string & s,
					std::ios_base & (*f) (std::
							      ios_base &))
  {
    std::istringstream iss(s);
    return !(iss >> f >> t).fail();
  }


  //##############################################################à

public:

  GI_Config()
  {

    fName = "";
    fargc = 0;
  };

  ~GI_Config()
  {
  };

  /* Read and parse input config file
   *
   * return -1 on error (TBD)
   */

  int parseFile(std::string filename)
  {

    if (fName != "")
      {
	cout << __FUNCTION__ << ": WARNING file " << filename <<
	  " will be parsed, overlapping previous " << fName << " config file"
	  << endl;
      }

    try
    {
      cfg.readFile(filename.data());
    }
    catch(const FileIOException & fioex)
    {
      std::cerr << "I/O error while reading file." << std::endl;
      exit(EXIT_FAILURE);
    }
    catch(const ParseException & pex)
    {
      std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
	<< " - " << pex.getError() << std::endl;
      exit(EXIT_FAILURE);
    }
    fName = filename;

    //    const Setting& ss = cfg.getRoot();

    //    navigate();

    return 0;

  };

  /*
   * write configuration on output file name
   */
  void save(const char *outfile)
  {
    if (fName.size() > 0)
      {
	cfg.writeFile(outfile);
      }
    cout << __FUNCTION__ << ": configuration written on file " << outfile <<
      endl;
  }


  /*
   * Navigate into the parsed db and print results
   * (mainly used for debug)
   */

  int nav(const Setting & ss)
  {

    //  cout << " Setting Name  : " << ss.getName() << endl;

    int ll = ss.getLength();

    cout << __FUNCTION__ << " " << ss.getPath();

    Setting::Type tt = ss.getType();
    cout << " type: " << tt;

    int i32;
    double dou;
    std::string str;
    bool boo;

    switch (tt)
      {
      case Setting::TypeInt:
      case Setting::TypeInt64:
	i32 = ss;
	cout << " = " << i32;
	break;
      case Setting::TypeFloat:
	dou = ss;
	cout << " = " << dou;
	break;
      case Setting::TypeString:
	str = ss.c_str();
	cout << " = " << str;
	break;
      case Setting::TypeBoolean:
	boo = ss;
	cout << " = " << boo;
	break;
      default:
	cout << " [" << ll << "]";
	// loop could be here
	break;
      }
    cout << endl;

    for (int i = 0; i < ll; i++)
      {
	nav(ss[i]);
      }

    return ll;

  };

  /*
   * ---> obsolete
   */

  int getInt(const char *path, int &ret)
  {

    int value;

    if (cfg.lookupValue(path, value))
      {
	ret = 0;
	return value;
      }
    else
      {
	ret = -1;
	cout << " ERROR " << endl;

	exit(-1);
      }

  }

  /**
   * specialized get bus.mpd.apv parameter (can be an array element)
   * ret: is the return error code
   * name: is the parameter name
   * elx: is the array element of name (0 if scalar)
   * apv, mpd, bus: the index of the apv, mpd and bus lists
   *
   */

  template < typename Tipo > Tipo getAPV(int &ret, int elx, const char *name,
					 int apv, int mpd, int bus = 0)
  {
    Tipo val;
    int lev[4] = { bus, mpd, apv, elx };
    const char *slev[4] = { "bus", "mpd", "apv", name };
    ret = getX(&val, 4, lev, slev);
    return (Tipo) val;
  }

  /**
   * as before, but for scalar
   */
  template < typename Tipo > Tipo getAPV(int &ret, const char *name, int apv,
					 int mpd, int bus = 0)
  {
    return (Tipo) getAPV < Tipo > (ret, 0, name, apv, mpd, bus);
  }

  /* Return number of APV elements in given mpd and bus
   */
  int getAPVLength(int mpd, int bus = 0)
  {
    int lev[3] = { bus, mpd, 0 };
    const char *slev[3] = { "bus", "mpd", "apv" };
    return getLength(3, lev, slev);
  }

  /**
   * specialized get bus.mpd parameter
   */

  template < typename Tipo > Tipo getMPD(int &ret, int elx, const char *name,
					 int mpd, int bus = 0)
  {
    Tipo val;
    int lev[3] = { bus, mpd, elx };
    const char *slev[3] = { "bus", "mpd", name };
    ret = getX(&val, 3, lev, slev);
    return (Tipo) val;
  }

  /**
   * as before, but for scalar
   */
  template < typename Tipo > Tipo getMPD(int &ret, const char *name, int mpd,
					 int bus = 0)
  {
    return (Tipo) getMPD < Tipo > (ret, 0, name, mpd, bus);
  }

  /* Return the number of MPD elements in given bus
   */
  int getMPDLength(int bus = 0)
  {
    int lev[2] = { bus, 0 };
    const char *slev[2] = { "bus", "mpd" };
indent: Standard input:636: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:637: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:638: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:639: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:640: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:641: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:642: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:643: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

    return getLength(2, lev, slev);
  }

  /**
   * specialized get bus.mpd.adc parameter
   */

  template < typename Tipo > Tipo getADC(int &ret, int elx, const char *name,
					 int adc, int mpd, int bus)
  {
    Tipo val;
    int lev[4] = { bus, mpd, adc, elx };
    const char *slev[4] = { "bus", "mpd", "adc", name };
    ret = getX(&val, 4, lev, slev);
    return (Tipo) val;
  }

  /**
   * as before, but for scalar
   */
  template < typename Tipo > Tipo getADC(int &ret, const char *name, int adc,
					 int mpd, int bus = 0)
  {
    return (Tipo) getADC < Tipo > (ret, 0, name, adc, mpd, bus);
  }

  /**
   * specialized get bus.mpd.i2c parameter
   */

  template < typename Tipo > Tipo getI2C(int &ret, int elx, const char *name,
					 int mpd, int bus)
  {
    Tipo val;
    int lev[4] = { bus, mpd, 0, elx };
    const char *slev[4] = { "bus", "mpd", "i2c", name };
    ret = getX(&val, 4, lev, slev);
    return (Tipo) val;
  }

  /**
   * as before, but for scalar
   */
  template < typename Tipo > Tipo getI2C(int &ret, const char *name, int mpd,
					 int bus = 0)
  {
    return (Tipo) getI2C < Tipo > (ret, 0, name, mpd, bus);
  }

  /**
   * specialized get bus parameter
   */

  template < typename Tipo > Tipo getBUS(int &ret, int elx, const char *name,
					 int bus = 0)
  {
    Tipo val;
    int lev[2] = { bus, elx };
    const char *slev[2] = { "bus", name };
    ret = getX(&val, 2, lev, slev);
    return (Tipo) val;
  }

  /**
   * as before, but for scalar
   */
  template < typename Tipo > Tipo getBUS(int &ret, const char *name, int bus =
					 0)
  {
    return (Tipo) getBUS < Tipo > (ret, 0, name, bus);
  }

  /**
   * specialized get run parameter
   */

  template < typename Tipo > Tipo getRUN(int &ret, int elx, const char *name)
  {
    Tipo val;
    int lev[2] = { 0, elx };
    const char *slev[2] = { "run", name };
    ret = getX(&val, 2, lev, slev);
    return (Tipo) val;
  }

  /**
   * as before, but for scalar
   */
  template < typename Tipo > Tipo getRUN(int &ret, const char *name)
  {
    return (Tipo) getRUN < Tipo > (ret, 0, name);
  }

  /**
   * specialized get parameter (on top level)
   */

  template < typename Tipo > Tipo getTop(int &ret, int elx, const char *name)
  {
    Tipo val;
    int lev[1] = { elx };
    const char *slev[1] = { name };
    ret = getX(&val, 1, lev, slev);
    return (Tipo) val;
  }

  /**
   * as before, but for scalar
   */
  template < typename Tipo > Tipo getTop(int &ret, const char *name)
  {
    return (Tipo) getTop < Tipo > (ret, 0, name);
  }

  /**
   * generic get function (up to 8 sub-groups)
   * example:
   *   to get the int value of
   *   bus[0].mpd[3].adc.gain the call is:
   * get<int>(ret, "bus", 0, "mpd", 3, "adc", 0, "gain");
   *
   * ret: return error code (0 if success)
   */

  template < typename Tipo > Tipo get(int &ret, std::string srx0, int idx0,
				      std::string srx1 = "", int idx1 = -1,
				      std::string srx2 = "", int idx2 = -1,
				      std::string srx3 = "", int idx3 = -1,
				      std::string srx4 = "", int idx4 = -1,
				      std::string srx5 = "", int idx5 = -1,
				      std::string srx6 = "", int idx6 = -1,
				      std::string srx7 = "", int idx7 = -1,
				      std::string srx8 = "", int idx8 = -1)
  {

    Tipo val;
    int dum[9];
    std::string sdum[9];
    int nlev = 0;
    int i;

    dum[0] = idx0;
    dum[1] = idx1;
    dum[2] = idx2;
    dum[3] = idx3;
    dum[4] = idx4;
    dum[5] = idx5;
    dum[6] = idx6;
    dum[7] = idx7;
    dum[8] = idx8;
    sdum[0] = srx0;
    sdum[1] = srx1;
    sdum[2] = srx2;
    sdum[3] = srx3;
    sdum[4] = srx4;
    sdum[5] = srx5;
    sdum[6] = srx6;
    sdum[7] = srx7;
    sdum[8] = srx8;

    nlev = 0;
    for (i = 0; i < 9; i++)
      {
	if (sdum[i].length() <= 0)
	  {
	    break;
	  }
	nlev++;
      }

    ret = getX(&val, nlev, dum, sdum);
    return (Tipo) val;

  }

  /**
   * as before, full scalar setting
   */

  template < typename Tipo > Tipo get(int &ret, std::string srx0,
				      std::string srx1 = "",
				      std::string srx2 = "",
				      std::strindent: Standard input:716: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:717: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:718: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:719: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:720: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:721: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:722: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

indent: Standard input:723: Warning:old style assignment ambiguity in "=-".  Assuming "= -"

ing srx3 = "",
				      std::string srx4 = "",
				      std::string srx5 = "",
				      std::string srx6 = "",
				      std::string srx7 = "",
				      std::string srx8 = "")
  {

    return (Tipo) get < Tipo > (ret, srx0, 0,
				srx1, 0, srx2, 0, srx3, 0, srx4, 0,
				srx5, 0, srx6, 0, srx7, 0, srx8, 0);
  }

  /**
   * generic get function (up to 8 sub-groups) / inverse mapping (obsolete)
   * example:
   *   to get the int value of
   *   bus[0].mpd[3].adc.gain the call is:
   * get<int>(ret, 0, "gain", 0,"adc",3,"mpd",0,"bus");
   *
   * note the reverse order of the subgroups
   *
   * ret: return error code (0 if success)
   *
   */

  template < typename Tipo > Tipo get(int &ret, int elx, std::string name,
				      int idx0 = -1, std::string srx0 = "",
				      int idx1 = -1, std::string srx1 = "",
				      int idx2 = -1, std::string srx2 = "",
				      int idx3 = -1, std::string srx3 = "",
				      int idx4 = -1, std::string srx4 = "",
				      int idx5 = -1, std::string srx5 = "",
				      int idx6 = -1, std::string srx6 = "",
				      int idx7 = -1, std::string srx7 = "")
  {

    Tipo val;
    int lev[9], dum[9];
    std::string rdx[9], sdum[9];
    int nlev = 0;
    int i;

    dum[0] = elx;
    dum[1] = idx0;
    dum[2] = idx1;
    dum[3] = idx2;
    dum[4] = idx3;
    dum[5] = idx4;
    dum[6] = idx5;
    dum[7] = idx6;
    dum[8] = idx7;
    sdum[0] = name;
    sdum[1] = srx0;
    sdum[2] = srx1;
    sdum[3] = srx2;
    sdum[4] = srx3;
    sdum[5] = srx4;
    sdum[6] = srx5;
    sdum[7] = srx6;
    sdum[8] = srx7;

    nlev = 0;
    for (i = 0; i < 9; i++)
      {
	if ((dum[i] < 0) || (sdum[i].length() <= 0))
	  {
	    break;
	  }
	nlev++;
      }

    for (i = 0; i < nlev; i++)
      {
	lev[i] = dum[nlev - 1 - i];
	rdx[i] = sdum[nlev - 1 - i];
      }

    ret = getX(&val, nlev, lev, rdx);
    return (Tipo) val;

  }


  /**
   * Replace a value to a setting, if the setting "path" exists
   * value must match the setting type
   *
   * return -1 on error
   */

  int replace(const std::string value, std::string path)
  {

    long int i64;
    int i32;
    float flo;
    std::string str;
    bool boo;

    if (cfg.exists(path))
      {

	cout << __FUNCTION__ << ": " << path << " will be modified" << endl;

	Setting & s1 = cfg.lookup(path);
	Setting::Type tp = s1.getType();

	switch (tp)
	  {
	  case Setting::TypeInt64:
	    if (s1.getFormat() == Setting::FormatHex)
	      {
		from_string < long int >(i64, value, std::hex);
	      }
	    else
	      {
		from_string < long int >(i64, value, std::dec);
	      }
	    s1 = i64;
	    break;
	  case Setting::TypeInt:
	    if (s1.getFormat() == Setting::FormatHex)
	      {
		from_string < int >(i32, value, std::hex);
	      }
	    else
	      {
		from_string < int >(i32, value, std::dec);
	      }
	    s1 = i32;
	    break;
	  case Setting::TypeFloat:
	    from_string < float >(flo, value, std::dec);
	    s1 = flo;
	    break;
	  case Setting::TypeString:
	    s1 = value;
	    break;
	  case Setting::TypeBoolean:
	    from_string < bool > (boo, value, std::dec);
	    s1 = boo;
	    break;
	  default:
	    cout << __FUNCTION__ << ": " << path <<
	      " type not recognized, nothing replaced" << endl;
	    return -1;
	    break;
	  }
      }
    else
      {
	cout << __FUNCTION__ << ": " << path <<
	  " setting does not exist, nothing replaced" << endl;
	return -1;
      }

    return 0;

  }

  //
  // Replace setting that are given in the CLI
  //

  int insertInline()
  {

    int i;

    for (i = 0; i < fargc; i++)
      {

	replace(fargv[i], fargp[i]);

      }

    return 0;

  }


  //
  // ---------------------------------------------------------------
  // parse the inline input arguments provided with the Daq command (Command Line Interface)
  //
  // return <0 if some error (TBD)
  //

  int parseInline(int argc, char *argv[])
  {

    int i;
    char cdummy[1000];
    std::string sarg;

    // init

    fargp = new std::string[argc];	// max settings expected
    fargv = new std::string[argc];
    fargc = 0;

    // hardcoded dafault value

    for (i = 1; i < argc; i++)
      {

	if (strcmp("-cnf", argv[i]) == 0)
	  {			// config file name, already used
	    i++;
	    sprintf(cdummy, "%s", argv[i]);
	    continue;
	  }

	if (strcmp("-fun", argv[i]) == 0)
	  {
	    printf("\n ------------------- WARNING ---------------------\n");
	    printf(" This software requires a valid license\n");
	    printf(" Any abuse will be persecuted by international laws\n");
	    printf(" Licenses are released by the Author only\n");
	    printf
	      (" A single node license costs $ 1,000,000/y + VAT when applicable\n");
	    printf(" ---------------------------------------------------\n");
	    exit(0);
	  }

	if (strcmp("-h", argv[i]) == 0)
	  {
	    i++;
	    printf
	      (" Command line: %s [-cnf prefix] [-h] [setting=value] ...\n",
	       argv[0]);
	    printf("  -cnf prefix    : config parameter file name\n");
	    printf("  -h             : this help\n");
	    printf
	      ("  setting=value  : setting is the path of a libconfig setting, value is the set value; can be repeated\n");
	    printf(" Example:\n");
	    printf
	      ("  %s bus.[0].mpd.[1].adc.[0].clock_phase=3 run.mode=\"event\" run.info=\"This is an example\"\n",
	       argv[0]);

	    exit(0);
	  }

	sarg = argv[i];

	cout << __FUNCTION__ << " process inline argument " << sarg << endl;

	size_t found;

	found = sarg.find_first_of("=");

	if (found < std::string::npos)
	  {
	    fargp[fargc] = sarg.substr(0, found);
	    fargv[fargc] = sarg.substr(found + 1, sarg.length());
	    cout << __FUNCTION__ << " " << i << " / " << fargc << " : " <<
	      fargp[fargc] << " = " << fargv[fargc] << endl;
	    fargc++;
	  }
	else
	  {
	    cout << __FUNCTION__ << " warning: >" << sarg <<
	      "< attribute looks not properly formed, skipped" << endl;
	  }

      }

    return 0;

  }

  /**
   *
   *
   */
  template < typename Tipo > int insert(Tipo val, std::string path)
  {

    long int i64;
    int i32;
    float flo;
    bool boo;
    std::string str;

    Setting::Type sety;

    size_t pos1, pos0;

    std::string sa, sb;

    pos0 = 0;

    Setting *se1;
    Setting *se0 = &(cfg.getRoot());

    se1 = NULL;

    sb = path;

    if (typeid(val) == typeid(i64))
      {
	sety = Setting::TypeInt64;
      }

    if (typeid(val) == typeid(i32))
      {
	sety = Setting::TypeInt;
      }

    if (typeid(val) == typeid(flo))
      {
	sety = Setting::TypeFloat;
      }

    if (typeid(val) == typeid(boo))
      {
	sety = Setting::TypeBoolean;
      }

    if (typeid(val) == typeid(std::string))
      {
	sety = Setting::TypeString;
      }

    cout << path << " " << typeid(val).name() << endl;

    do
      {
	pos1 = sb.find_first_of(".");
	if (pos1 >= std::string::npos)
	  {
	    sa = sb.substr(0, sb.length());
	    sb = "";
	  }
	else
	  {
	    sa = sb.substr(0, pos1);
	    sb = sb.substr(pos1 + 1, sb.length());
	  }

	cout << pos1 << " " << sa << " / " << sb << endl;

	if (!se0->exists(sa))
	  {
	    if (sb.length() > 0)
	      {
		se1 = &(se0->add(sa, Setting::TypeGroup));
	      }
	    else
	      {
		se1 = &(se0->add(sa, sety));
	      }
	    se0 = se1;
	  }
      }
    while (pos1 < std::string::npos);

    if (se1)
      {
	Setting & se = *se1;
	se = val;
      }

    cout << se1->getPath() << endl;

    return 0;
  }


  /**
   *
   */

  int navigate()
  {

    const Setting & ss = cfg.getRoot();

    nav(ss);

    return 0;

  }

};



#endif
