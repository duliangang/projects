#include "config.h"



// Config.cpp


using namespace std;




Config::Config()
	: m_Delimiter( wstring(1,'=') ), m_Comment( wstring(1,'#') )
{
	// Construct a Config without a file; empty
}



bool Config::KeyExists( const _tstring& key ) const
{
	// Indicate whether key is found
	mapci p = m_Contents.find( key );
	return ( p != m_Contents.end() );
}


/* static */
void Config::Trim( _tstring& inout_s )
{
	// Remove leading and trailing whitespace
	static const _tchar whitespace[] = _T(" \n\t\v\r\f");
	inout_s.erase( 0, inout_s.find_first_not_of(whitespace) );
	inout_s.erase( inout_s.find_last_not_of(whitespace) + 1U );
}


std::_tostream& operator<<( std::_tostream& os, const Config& cf )
{
	// Save a Config to os
	for( Config::mapci p = cf.m_Contents.begin();
		p != cf.m_Contents.end();
		++p )
	{
		os << p->first << _T(" ") << cf.m_Delimiter << _T(" ");
		os << p->second << std::endl;
	}
	return os;
}

void Config::Remove( const _tstring& key )
{
	// Remove key and its value
	m_Contents.erase( m_Contents.find( key ) );
	return;
}

std::_tistream& operator>>( std::_tistream& is, Config& cf )
{
	// Load a Config from is
	// Read in keys and values, keeping internal whitespace
	typedef _tstring::size_type pos;
	const _tstring& delim  = cf.m_Delimiter;  // separator
	const _tstring& comm   = cf.m_Comment;    // comment
	const pos skip = delim.length();        // length of separator

	_tstring nextline;  // might need to read ahead to see where value ends

	while( is || nextline.length() > 0 )
	{
		// Read an entire line at a time
		_tstring line;
		if( nextline.length() > 0 )
		{
			line = nextline;  // we read ahead; use it now
			nextline.clear();
		}
		else
		{
			std::getline( is, line );
		}

		// Ignore comments
		line = line.substr( 0, line.find(comm) );

		// Parse the line if it contains a delimiter
		pos delimPos = line.find( delim );
		if( delimPos != string::npos )
		{
			// Extract the key
			_tstring key = line.substr( 0, delimPos );
			line.replace( 0, delimPos+skip,_T("") );

			// See if value continues on the next line
			// Stop at blank line, next line with a key, end of stream,
			// or end of file sentry
			bool terminate = false;
			while( !terminate && is )
			{
				std::getline( is, nextline );
				terminate = true;

				_tstring nlcopy = nextline;
				Config::Trim(nlcopy);
				if( nlcopy.empty() ) continue;

				nextline = nextline.substr( 0, nextline.find(comm) );
				if( nextline.find(delim) != string::npos )
					continue;

				nlcopy = nextline;
				Config::Trim(nlcopy);
				if( !nlcopy.empty() ) line += _T("\n");
				line += nextline;
				terminate = false;
			}

			// Store key and value
			Config::Trim(key);
			Config::Trim(line);
			cf.m_Contents[key] = line;  // overwrites if key is repeated
		}
	}

	return is;
}
bool Config::FileExist(std::_tstring filename)
{
	bool exist= false;
	std::ifstream in( filename.c_str() );
	if( in ) 
		exist = true;
	return exist;
}

bool  Config::ReadFile( _tstring filename, _tstring delimiter,
	_tstring comment )
{
	m_FileName=filename;
	m_Delimiter = delimiter;
	m_Comment = comment;
	std::_tifstream in( filename.c_str() );

	if( !in ) return false;

	in >> (*this);
	return true;
}







































































//// Config.cpp
//
//#include "Config.h"
//
//using namespace std;
//
//
//
//
//Config::Config()
//	: m_Delimiter( string(1,'=') ), m_Comment( string(1,'#') )
//{
//	// Construct a Config without a file; empty
//}
//
//
//
//bool Config::KeyExists( const string& key ) const
//{
//	// Indicate whether key is found
//	mapci p = m_Contents.find( key );
//	return ( p != m_Contents.end() );
//}
//
//
///* static */
//void Config::Trim( string& inout_s )
//{
//	// Remove leading and trailing whitespace
//	static const char whitespace[] = " \n\t\v\r\f";
//	inout_s.erase( 0, inout_s.find_first_not_of(whitespace) );
//	inout_s.erase( inout_s.find_last_not_of(whitespace) + 1U );
//}
//
//
//std::ostream& operator<<( std::ostream& os, const Config& cf )
//{
//	// Save a Config to os
//	for( Config::mapci p = cf.m_Contents.begin();
//		p != cf.m_Contents.end();
//		++p )
//	{
//		os << p->first << " " << cf.m_Delimiter << " ";
//		os << p->second << std::endl;
//	}
//	return os;
//}
//
//void Config::Remove( const string& key )
//{
//	// Remove key and its value
//	m_Contents.erase( m_Contents.find( key ) );
//	return;
//}
//
//std::istream& operator>>( std::istream& is, Config& cf )
//{
//	// Load a Config from is
//	// Read in keys and values, keeping internal whitespace
//	typedef string::size_type pos;
//	const string& delim  = cf.m_Delimiter;  // separator
//	const string& comm   = cf.m_Comment;    // comment
//	const pos skip = delim.length();        // length of separator
//
//	string nextline = "";  // might need to read ahead to see where value ends
//
//	while( is || nextline.length() > 0 )
//	{
//		// Read an entire line at a time
//		string line;
//		if( nextline.length() > 0 )
//		{
//			line = nextline;  // we read ahead; use it now
//			nextline = "";
//		}
//		else
//		{
//			std::getline( is, line );
//		}
//
//		// Ignore comments
//		line = line.substr( 0, line.find(comm) );
//
//		// Parse the line if it contains a delimiter
//		pos delimPos = line.find( delim );
//		if( delimPos < string::npos )
//		{
//			// Extract the key
//			string key = line.substr( 0, delimPos );
//			line.replace( 0, delimPos+skip, "" );
//
//			// See if value continues on the next line
//			// Stop at blank line, next line with a key, end of stream,
//			// or end of file sentry
//			bool terminate = false;
//			while( !terminate && is )
//			{
//				std::getline( is, nextline );
//				terminate = true;
//
//				string nlcopy = nextline;
//				Config::Trim(nlcopy);
//				if( nlcopy == "" ) continue;
//
//				nextline = nextline.substr( 0, nextline.find(comm) );
//				if( nextline.find(delim) != string::npos )
//					continue;
//
//				nlcopy = nextline;
//				Config::Trim(nlcopy);
//				if( nlcopy != "" ) line += "\n";
//				line += nextline;
//				terminate = false;
//			}
//
//			// Store key and value
//			Config::Trim(key);
//			Config::Trim(line);
//			cf.m_Contents[key] = line;  // overwrites if key is repeated
//		}
//	}
//
//	return is;
//}
//bool Config::FileExist(std::string filename)
//{
//	bool exist= false;
//	std::ifstream in( filename.c_str() );
//	if( in ) 
//		exist = true;
//	return exist;
//}
//
//bool  Config::ReadFile( string filename, string delimiter,
//	string comment )
//{
//	m_FileName=filename;
//	m_Delimiter = delimiter;
//	m_Comment = comment;
//	std::ifstream in( filename.c_str() );
//
//	if( !in ) return false;
//
//	in >> (*this);
//	return true;
//}
//




