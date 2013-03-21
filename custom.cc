/* custom.cc -- Implementation of Localization, Customization, and Preferences

Copyright (C) 1997-2013, Brian Bray

*/

#include <cstring>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sstream>
#include <fstream>
#include <list>
#include <map>
#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/custom.h"
#include "bw/exception.h"
#include "bw/process.h"


namespace bw {


//
// Compilation parameters
//
// These are quite hardcoded right now.
//
// The site file is at /etc/bw/<appname>
//
// The user file is at ~/.<appname>
//
// The Application file is at /usr/lib/X11/app-defaults/<application>
//
//
const char* pszSiteDir = "/etc/bw/";
const char* pszUserPrefix = "/.";
const char* pszLocaleDir = "/usr/lib/X11/app-defaults/";

// Globals and statics
static class Custom* pTheSite = 0;
static class Custom* pTheUser = 0;
static class Custom* pTheApp = 0;


// Local routines
String makeKeyname(const String& key, int index)
{
	std::ostringstream ost;
	ost << key << index << std::ends;
	return ost.str().c_str();
}


/*: class Custom

   Abstract class for user customization parameters.  The Custom class holds
   parameters obtained from the windowing system configuration and from
   configuration files.  The nature of file storage depends on the OS/Window
   system in use.  Customizations stored in files have a standard format --
   see the description of <a href="CustomFile.html">CustomFile</a> for details.

   Three customization categories are predefined.  References to the relevent
   customization objects are available from global procedures:
   <ul>
   <li>TheSite() - administrator defined parameters.
   <li>TheUser() - Settings specific to this user for all applications.
   <li>TheApp() - Settings specific to this application for all sites.
   </ul>

   Each Custom object is logically composed of "categories" containing "keys".
   There is always a blank category ("").  Each key has a value that can be
   interpreted as a string or as a long integer.

   When retrieving values, a default is always provided.  If the key is not
   found, the default value is returned.  For numeric values, the default
   is also returned if the value cannot be converted to a long integer.

   It is common to store an array of related values using keys that end with
   an integer.  Helper routines are provided for retrieving and setting these
   indexed keys.

   The value of a key can be changed at any time.  A "writable" custom
   object is one where changes are written to permanent storage.  Whether
   an instance is writable or not depends on the OS/Windowing system in use
   and the access rights of the executing process.

   Some customization files contain keys indexed by locale.  When loaded, only
   the values without locales or for the requested locale are read.  The
   Locale name is interpreted with the pattern:

   &lt;Language> [ _&lt;region> [ .&lt;character-set> ] ]

   The most specific locale is matched.  Typically, the language is in lower
   case and the region is in upper case.  E.g. "fr" is french, "fr_FR" is
   french as written in France, and "fr_CA.iso-8859-1" is french as written
   in Canada in the iso-8859-1 (latin-1) character set.
*/

/*: Custom::isWritable

  Returns true if changes to this custom object will be written to
  permanent storage.

  Prototype: bool isWritable() const

*/

/*: Custom:: getString

  Returns the string value of the specified key in the specified category.
  If the key has no defined value in the customization object, then the
  default value is returned.

  The variant with an index is identical to searching for a key name consisting
  of the given key name followed by the index converted to a character string.

  Prototype: String getString(const String& strCategory,
			      const String& strKey,
			      const String& strDefault ) const
  Prototype: String getIndxString( const String& strCategory,
			      const String& strKey,
			      int index,
			      const String& strDefault ) const
*/
String Custom::getIndxString( const String& strCategory,
                              const String& strKey,
                              int index,
                              const String& strDefault ) const
{
	return getString( strCategory, makeKeyname(strKey,index), strDefault );
}

/*: Custom::getNumber

  Returns the numeric value of the specified key in the specified category.
  If the key has no defined value in the customization object, then the
  default value is returned.  If the key value cannot be interpreted as a
  number, then the default value is returned.

  The variant with an index is identical to searching for a key name consisting
  of the given key name followed by the index converted to a character string.

  Prototype: long getNumber( const String& strCategory,
			    const String& strKey,
			    long lDefault ) const

  Prototype: long getIndxNumber( const String& strCategory,
			    const String& strKey,
			    int index,
			    long lDefault ) const

*/
long Custom::getIndxNumber
(
    const String& strCategory,
    const String& strKey,
    int index,
    long lDefault ) const
{
	return getNumber( strCategory, makeKeyname(strKey,index), lDefault );
}

/*: Custom::putString

  Writes the value of the specified key in the specified category.

  The variant with an index is identical to setting the key name consisting
  of the given key name followed by the index converted to a character string.

  Prototype: void putString( const String& strCategory,
			    const String& strKey,
			    const String& strValue )
  Prototype: void putIndxString( const String& strCategory,
			    const String& strKey,
			    int index,
			    const String& strValue )
*/
void Custom::putIndxString( const String& strCategory,
                            const String& strKey,
                            int index,
                            const String& strValue )
{
	putString( strCategory, makeKeyname(strKey,index), strValue );
}

/*: Custom:: putNumber

  Writes the value of the specified key in the specified category.

  The variant with an index is identical to setting the key name consisting
  of the given key name followed by the index converted to a character string.

  Prototype: void putNumber( const String& strCategory,
			    const String& strKey,
			    long lValue )

  Prototype: void putIndxNumber( const String& strCategory,
			    const String& strKey,
			    int index,
			    long lValue )
*/
void Custom::putIndxNumber( const String& strCategory,
                            const String& strKey,
                            int index,
                            long lValue )
{
	putNumber( strCategory, makeKeyname(strKey,index), lValue );
}

/*: Custom::getKeys()

  Returns a list&lt;> of the keys in the specified category.

  Prototype: list<String> getKeys( const String& strCategory ) const
*/

/*: class CustomFile

  User customization parameters stored in a file or files.

  This is a subclass of the <a href="Custom.html">Custom</a> class.
  The routines to access and change customization values are defined in the
  base class.

  The file format approximatelly matches the ".INI" file format of KDE and
  Gnome.  This is an extension of the old Microsoft Windows INI file format.

*/


/*: CustomFile::CustomFile

  Constructor.  Parameters are the filename to access and the locale (if any)
  for the customizations.

  Customization files can contain locale specific parameters by terminating
  the key with [ locale ].  Eg: foo[fr], or abc123[fr_CA.iso-8859-1].

  If a locale is specified, then only the most specific matching localization
  for each key value will be loaded and the Custom object will not be writable.
*/
CustomFile::CustomFile(const String& filename, const char* locale)
	: m_fname(filename), m_locale(locale), m_isWritable( false )
{
	if (access(filename,F_OK)==0) {
		mergeFile(filename);
		if (*locale=='\0')
			m_isWritable = ( access(filename,W_OK)==0 );
	} else {
		m_isWritable = true;
	}
}

/*: CustomFile::~CustomFile()

  Destructor.
*/
CustomFile::~CustomFile()
{
}

static void trim(String& str)
{
	int s = 0;
	int e = str.length();
	while( isspace(str[s]) )
		++s;
	if (s>=e) {
		str = "";
		return;
	}
	while( isspace(str[e-1]) )
		--e;

	str = str.substring(s,e);
	return;
}

/*: CustomFile::mergeFile

  Merges another file of customization settings with this one.

  Duplicate settings are overwritten by the merged file.
*/
void CustomFile::mergeFile(const String& filename)
{
	const int maxLineLen=4000;

	char line[maxLineLen];
	char *pch;
	String key;
	String value;
	String section;
	String locale;
	int loc;

	std::ifstream in(filename);

	while (in) {
		*line = '\0';
		in.getline( line, maxLineLen, '\n' );
		pch = line;
		while ( isspace(*pch) )
			++pch;
		if (*pch=='#' || *pch=='\0' )
			continue;
		if (*pch=='[') {
			// section / category marker
			String tmpSection = ++pch;
			trim(tmpSection);
			if (tmpSection[tmpSection.length()-1]==']')
				section = tmpSection.substring(0,tmpSection.length()-1);
			continue;
		}
		key = pch;
		loc = key.indexOf('=');
		if (loc<0)
			continue;		// Ignore bad line
		value = key.substring(loc+1);
		key = key.substring(0,loc);
		trim(key);
		trim(value);

		if (m_locale!="" && key[key.length()-1]==']') {
			loc = key.indexOf("[");
			if (loc<0)
				continue;	// Bad line
			locale = key.substring(loc+1,key.length()-1);
			key = key.substring(0,loc);
			trim(key);
			trim(locale);

			if (!m_locale.startsWith(locale))
				continue;
		}
		if (key!="")
			m_val[section][key] = value;
	}
}


bool CustomFile::isWritable() const
{
	return m_isWritable;
}

const String* CustomFile::locate( const String& strCategory, const String& strKey ) const
{
	using std::map;

	ValType::const_iterator iter = m_val.find(strCategory);
	if (iter!=m_val.end()) {
		const map< String, String> &sect = (*iter).second;
		map< String, String>::const_iterator jter;
		jter = sect.find(strKey);
		if (jter!=sect.end())
			return &(*jter).second;
	}
	return 0;
}


String CustomFile::getString(
    const String& strCategory,
    const String& strKey,
    const String& strDefault ) const
{
	const String* res = locate(strCategory, strKey);
	if (res)
		return *res;

	return strDefault;
}



long CustomFile::getNumber( const String& strCategory,
                            const String& strKey,
                            long lDefault ) const
{
	long lres = lDefault;
	const String* res = locate(strCategory, strKey);
	if (res) {
		std::istringstream ist(res->c_str());
		ist >> lres;
		if (ist.fail())
			lres = lDefault;
	}

	return lres;
}


void CustomFile::putString( const String& strCategory,
                            const String& strKey,
                            const String& strValue )
{
	m_val[strCategory][strKey] = strValue;
	sync();
}


void CustomFile::putNumber( const String& strCategory,
                            const String& strKey,
                            long lValue )
{
	std::ostringstream ost;
	ost << lValue << std::ends;
	putString(strCategory,strKey,ost.str().c_str() );
}


std::list<String> CustomFile::getCategories() const
{
	ValType::const_iterator iter;
	std::list<String> res;

	for (iter=m_val.begin(); iter!=m_val.end(); ++iter)
		res.push_back((*iter).first);

	return res;
}


std::list<String> CustomFile::getKeys( const String& strCategory ) const
{
	using std::map;

	std::list<String> res;
	ValType::const_iterator iter = m_val.find(strCategory);
	if (iter!=m_val.end()) {
		const map< String, String> &sect = (*iter).second;
		map< String, String>::const_iterator jter;

		for (jter=sect.begin(); jter!=sect.end(); ++jter)
			res.push_back((*jter).first);
	}

	return res;
}

/* CustomFile::sync()

   Writes change(s) back to file.
*/
void CustomFile::sync()
{
	// This should be smarter, but right now it just writes the whole file.
	// We should also be catching signals through here.

	using std::ofstream;
	using std::ios;
	using std::map;
	using std::ostringstream;
	using std::ends;

	if (!m_isWritable)
		return;			// Don't even try

	ofstream ost;
	ostringstream tempname;
	tempname << m_fname << "~~" << ::getpid() << ends;;

	// If it's not open yet, assume old file problem
	ost.open( tempname.str().c_str(), ios::out|ios::trunc );

	ValType::const_iterator iter;

	for (iter=m_val.begin(); iter!=m_val.end(); ++iter) {
		if ((*iter).first!="")
			ost << "\n[" << (*iter).first << "]\n";

		const map< String, String> &sect = (*iter).second;
		map< String, String>::const_iterator jter;

		for (jter=sect.begin(); jter!=sect.end(); ++jter) {
			ost << (*jter).first << "=" << (*jter).second << "\n";
		}
	}

	if (ost) {
		// Everything written
		ost.close();

		rename(tempname.str().c_str(),m_fname);
		unlink(tempname.str().c_str());
	} else {
		// Problems writing file, don't try again
		m_isWritable = false;
	}
}

// Global access functions

Custom& TheSite()
{
	if (!pTheSite) {
		bwassert( BWAppName );
		bwassert( *BWAppName );
		String strFileName = pszSiteDir;
		strFileName.append( BWAppName );
		pTheSite = new CustomFile(strFileName);
	}
	return *pTheSite;
}

Custom& TheUser()
{
	if (!pTheUser) {
		bwassert( BWAppName );
		bwassert( *BWAppName );
		String m_strFileName = getHomeDir();
		m_strFileName.append( pszUserPrefix );
		m_strFileName.append( BWAppName );
		pTheUser = new CustomFile(m_strFileName);
	}
	return *pTheUser;
}


/*: routine TheApp()

  This routine returns a reference to a customization object for
  customizations independent of the user and the installation site,
  but dependent on the locale (language and country/territory) and the
  Application.

  Both the internal and external data is organized into default data,
  overiden by language specific data, overiden by territory specific data
  for that language.

  BW assumes the locale is made up of Language, Territory, and Charset fields.
  It assumes <Language>_<Territory>.<Charset> as the format.  By preference,
  ISO639 (RFC1766) codes should be used for the language, ISO 3166
  codes for the territories.

  When reading customization files, the value with longest matching locale
  will be used.  For example, with the file,

  foo=1
  foo[en]=2
  foo[en_US]=3
  foo[en_US.iso-8859-1]=4

  Foo will have the one of the following values:

  Locale		foo
  ======		===

  "C"			1
  fr_FR			1
  en_CA			2
  en_US.utf8		3

  Note that the default locale is probably not "".  "C" and "POSIX" are
  more likely.
*/
Custom& TheApp()
{
	if (!pTheApp) {
		bwassert( BWAppName );
		bwassert( *BWAppName );
		String m_strFileName = pszLocaleDir;
		m_strFileName.append( BWAppName );
		pTheApp = new CustomFile( m_strFileName,BWLocale );
	}
	return *pTheApp;
}


} //Namespace bw
