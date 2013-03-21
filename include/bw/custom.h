/* custom.h -- Interface to Localization, Customization, and Preferences

Copyright (C) 1997-2013, Brian Bray

*/

/*
#include "bw/string.h"
#include <list>
#include <map>
*/

namespace bw {

class Custom {
public:
	virtual ~Custom() {}
	virtual bool isWritable() const = 0;
	virtual String getString( const String& strCategory,
	                          const String& strKey,
	                          const String& strDefault ) const = 0;
	virtual long getNumber( const String& strCategory,
	                        const String& strKey,
	                        long lDefault ) const = 0;
	virtual void putString( const String& strCategory,
	                        const String& strKey,
	                        const String& strValue ) = 0;
	virtual void putNumber( const String& strCategory,
	                        const String& strKey,
	                        long lValue ) = 0;

	virtual String getIndxString( const String& strCategory,
	                              const String& strKey,
	                              int index,
	                              const String& strDefault ) const;
	virtual long getIndxNumber( const String& strCategory,
	                            const String& strKey,
	                            int index,
	                            long lDefault ) const;
	virtual void putIndxString( const String& strCategory,
	                            const String& strKey,
	                            int index,
	                            const String& strValue );
	virtual void putIndxNumber( const String& strCategory,
	                            const String& strKey,
	                            int index,
	                            long lValue );

	virtual std::list<String> getCategories() const =0;
	virtual std::list<String> getKeys( const String& strCategory ) const =0;
};

class CustomFile : public Custom {
public:
	CustomFile(const String& filename, const char* locale="");
	~CustomFile();

	void mergeFile(const String& filename);

	virtual bool isWritable() const;
	virtual String getString( const String& strCategory,
	                          const String& strKey,
	                          const String& strDefault ) const;
	virtual long getNumber( const String& strCategory,
	                        const String& strKey,
	                        long lDefault ) const;
	virtual void putString( const String& strCategory,
	                        const String& strKey,
	                        const String& strValue );
	virtual void putNumber( const String& strCategory,
	                        const String& strKey,
	                        long lValue );

	virtual std::list<String> getCategories() const;
	virtual std::list<String> getKeys( const String& strCategory ) const;

private:
	const String* locate( const String& strCategory, const String& strKey) const;
	void	sync();

	String	m_fname;
	String	m_locale;
	typedef std::map< String, std::map<String,String> >	ValType;
	ValType	m_val;
	bool	m_isWritable;
};

//
// Global routines for app customizations
//
Custom&		TheSite();
Custom&		TheUser();
Custom&		TheApp();


}   // namespace bw



