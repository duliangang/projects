
#ifndef _CONFIG_H_
#define _CONFIG_H_
#include "../shared/Define.h"
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

class Config {
	// Data
private:
	std::_tstring m_FileName;
protected:
	std::_tstring m_Delimiter;  //!< separator between key and value
	std::_tstring m_Comment;    //!< separator between value and comments
	std::map<std::_tstring,std::_tstring> m_Contents;  //!< extracted keys and values

	typedef std::map<std::_tstring,std::_tstring>::iterator mapi;
	typedef std::map<std::_tstring,std::_tstring>::const_iterator mapci;
	// Methods
public:

	//Config( std::_tstring filename,std::_tstring delimiter = "=",std::string comment = "#" );
	Config();
	template<class T> T Read( const std::_tstring& in_key ) const;  //!<Search for key and read value or optional default value, call as read<T>

	const _tchar*  Read(const std::_tstring& in_key,const _tchar* def)const
	{
		mapci p = m_Contents.find(in_key);
		if( p == m_Contents.end() ) return def;
		return p->second.c_str();
	}

	template<class T> T Read( const std::_tstring& in_key, const T& in_value ) const;
	template<class T> bool ReadInto( T& out_var, const std::_tstring& in_key ) const;
	template<class T>
	bool ReadInto( T& out_var, const std::_tstring& in_key, const T& in_value ) const;
	bool FileExist(std::_tstring filename);
	bool  ReadFile(std::_tstring filename,std::_tstring delimiter = _T("="),std::_tstring comment = _T("#") );

	// Check whether key exists in configuration
	bool KeyExists( const std::_tstring& in_key ) const;

	// Modify keys and values
	template<class T> void Add( const std::_tstring& in_key, const T& in_value );
	void Remove( const std::_tstring& in_key );

	// Check or change configuration syntax
	std::_tstring GetDelimiter() const { return m_Delimiter; }
	std::_tstring GetComment() const { return m_Comment; }
	std::_tstring SetDelimiter( const std::_tstring& in_s )
	{ std::_tstring old = m_Delimiter;  m_Delimiter = in_s;  return old; }  
	std::_tstring SetComment( const std::_tstring& in_s )
	{ std::_tstring old = m_Comment;  m_Comment =  in_s;  return old; }

	// Write or read configuration
	friend std::_tostream& operator<<( std::_tostream& os, const Config& cf );
	friend std::_tistream& operator>>( std::_tistream& is, Config& cf );

protected:
	template<class T> static std::_tstring T_as_string( const T& t );
	template<class T> static T string_as_T( const std::_tstring& s );
	static void Trim( std::_tstring& inout_s );


	// Exception types
public:
	struct File_not_found {
		std::_tstring filename;
		File_not_found( const std::_tstring& filename_ = std::_tstring() )
			: filename(filename_) {} };
		struct Key_not_found {  // thrown only by T read(key) variant of read()
			std::_tstring key;
			Key_not_found( const std::_tstring& key_ = std::_tstring() )
				: key(key_) {} };
};


/* static */
template<class T>
std::_tstring Config::T_as_string( const T& t )
{
	// Convert from a T to a string
	// Type T must support << operator
	std::_tostringstream ost;
	ost << t;
	return ost.str();
}


/* static */
template<class T>
T Config::string_as_T( const std::_tstring& s )
{
	// Convert from a string to a T
	// Type T must support >> operator
	T t;
	std::_tistringstream ist(s);
	ist >> t;
	return t;
}
//
template<class T>
T Config::Read( const std::_tstring& key ) const
{
	// Read the value corresponding to key
	mapci p = m_Contents.find(key);
	if( p == m_Contents.end() ) throw Key_not_found(key);
	return string_as_T<T>( p->second );
}


template<class T>
T Config::Read( const std::_tstring& key, const T& value ) const
{
	// Return the value corresponding to key or given default value
	// if key is not found
	mapci p = m_Contents.find(key);
	if( p == m_Contents.end() ) return value;
	return string_as_T<T>( p->second );
}


template<class T>
bool Config::ReadInto( T& var, const std::_tstring& key ) const
{
	// Get the value corresponding to key and store in var
	// Return true if key is found
	// Otherwise leave var untouched
	mapci p = m_Contents.find(key);
	bool found = ( p != m_Contents.end() );
	if( found ) var = string_as_T<T>( p->second );
	return found;
}


template<class T>
bool Config::ReadInto( T& var, const std::_tstring& key, const T& value ) const
{
	// Get the value corresponding to key and store in var
	// Return true if key is found
	// Otherwise set var to given default
	mapci p = m_Contents.find(key);
	bool found = ( p != m_Contents.end() );
	if( found )
		var = string_as_T<T>( p->second );
	else
		var = value;
	return found;
}


template<class T>
void Config::Add( const std::_tstring& in_key, const T& value )
{
	// Add a key with given value
	std::_tstring v = T_as_string( value );
	std::_tstring key=in_key;
	Trim(key);
	Trim(v);
	m_Contents[key] = v;
	return;
}

#endif




























































////Config.h
//#ifndef _CONFIG_H_
//#define _CONFIG_H_
//#include <string>
//#include <map>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//
//class Config {
//	// Data
//private:
//	std::string m_FileName;
//protected:
//	std::string m_Delimiter;  //!< separator between key and value
//	std::string m_Comment;    //!< separator between value and comments
//	std::map<std::string,std::string> m_Contents;  //!< extracted keys and values
//
//	typedef std::map<std::string,std::string>::iterator mapi;
//	typedef std::map<std::string,std::string>::const_iterator mapci;
//	// Methods
//public:
//
//	//Config( std::string filename,std::string delimiter = "=",std::string comment = "#" );
//	Config();
//	template<class T> T Read( const std::string& in_key ) const;  //!<Search for key and read value or optional default value, call as read<T>
//
//	const char*  Read(const std::string& in_key,const char* def)
//	{
//		mapci p = m_Contents.find(in_key);
//		if( p == m_Contents.end() ) return def;
//		return p->second.c_str();
//	}
//	
//	template<class T> T Read( const std::string& in_key, const T& in_value ) const;
//	template<class T> bool ReadInto( T& out_var, const std::string& in_key ) const;
//	template<class T>
//	bool ReadInto( T& out_var, const std::string& in_key, const T& in_value ) const;
//	bool FileExist(std::string filename);
//	bool  ReadFile(std::string filename,std::string delimiter = "=",std::string comment = "#" );
//
//	// Check whether key exists in configuration
//	bool KeyExists( const std::string& in_key ) const;
//
//	// Modify keys and values
//	template<class T> void Add( const std::string& in_key, const T& in_value );
//	void Remove( const std::string& in_key );
//
//	// Check or change configuration syntax
//	std::string GetDelimiter() const { return m_Delimiter; }
//	std::string GetComment() const { return m_Comment; }
//	std::string SetDelimiter( const std::string& in_s )
//	{ std::string old = m_Delimiter;  m_Delimiter = in_s;  return old; }  
//	std::string SetComment( const std::string& in_s )
//	{ std::string old = m_Comment;  m_Comment =  in_s;  return old; }
//
//	// Write or read configuration
//	friend std::ostream& operator<<( std::ostream& os, const Config& cf );
//	friend std::istream& operator>>( std::istream& is, Config& cf );
//
//protected:
//	template<class T> static std::string T_as_string( const T& t );
//	template<class T> static T string_as_T( const std::string& s );
//	static void Trim( std::string& inout_s );
//
//
//	// Exception types
//public:
//	struct File_not_found {
//		std::string filename;
//		File_not_found( const std::string& filename_ = std::string() )
//			: filename(filename_) {} };
//		struct Key_not_found {  // thrown only by T read(key) variant of read()
//			std::string key;
//			Key_not_found( const std::string& key_ = std::string() )
//				: key(key_) {} };
//};
//
//
///* static */
//template<class T>
//std::string Config::T_as_string( const T& t )
//{
//	// Convert from a T to a string
//	// Type T must support << operator
//	std::ostringstream ost;
//	ost << t;
//	return ost.str();
//}
//
//
///* static */
//template<class T>
//T Config::string_as_T( const std::string& s )
//{
//	// Convert from a string to a T
//	// Type T must support >> operator
//	T t;
//	std::istringstream ist(s);
//	ist >> t;
//	return t;
//}
//
//
///* static */
//template<>
//inline std::string Config::string_as_T<std::string>( const std::string& s )
//{
//	// Convert from a string to a string
//	// In other words, do nothing
//	return s;
//}
//
//
///* static */
//template<>
//inline bool Config::string_as_T<bool>( const std::string& s )
//{
//	// Convert from a string to a bool
//	// Interpret "false", "F", "no", "n", "0" as false
//	// Interpret "true", "T", "yes", "y", "1", "-1", or anything else as true
//	bool b = true;
//	std::string sup = s;
//	for( std::string::iterator p = sup.begin(); p != sup.end(); ++p )
//		*p = toupper(*p);  // make string all caps
//	if( sup==std::string("FALSE") || sup==std::string("F") ||
//		sup==std::string("NO") || sup==std::string("N") ||
//		sup==std::string("0") || sup==std::string("NONE") )
//		b = false;
//	return b;
//}
//
//
//template<class T>
//T Config::Read( const std::string& key ) const
//{
//	// Read the value corresponding to key
//	mapci p = m_Contents.find(key);
//	if( p == m_Contents.end() ) throw Key_not_found(key);
//	return string_as_T<T>( p->second );
//}
//
//
//template<class T>
//T Config::Read( const std::string& key, const T& value ) const
//{
//	// Return the value corresponding to key or given default value
//	// if key is not found
//	mapci p = m_Contents.find(key);
//	if( p == m_Contents.end() ) return value;
//	return string_as_T<T>( p->second );
//}
//
//
//template<class T>
//bool Config::ReadInto( T& var, const std::string& key ) const
//{
//	// Get the value corresponding to key and store in var
//	// Return true if key is found
//	// Otherwise leave var untouched
//	mapci p = m_Contents.find(key);
//	bool found = ( p != m_Contents.end() );
//	if( found ) var = string_as_T<T>( p->second );
//	return found;
//}
//
//
//template<class T>
//bool Config::ReadInto( T& var, const std::string& key, const T& value ) const
//{
//	// Get the value corresponding to key and store in var
//	// Return true if key is found
//	// Otherwise set var to given default
//	mapci p = m_Contents.find(key);
//	bool found = ( p != m_Contents.end() );
//	if( found )
//		var = string_as_T<T>( p->second );
//	else
//		var = value;
//	return found;
//}
//
//
//template<class T>
//void Config::Add( const std::string& in_key, const T& value )
//{
//	// Add a key with given value
//	std::string v = T_as_string( value );
//	std::string key=in_key;
//	trim(key);
//	trim(v);
//	m_Contents[key] = v;
//	return;
//}
//
//#endif
