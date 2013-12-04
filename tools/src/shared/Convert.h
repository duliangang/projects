#pragma once
#include <string>
using namespace std; 
 //base64的编码和解码函数

class CConvert
{
public:
	CConvert();
	void encode(FILE *, std::string& , bool add_crlf = true);
	void encode(const std::string&, std::string& , bool add_crlf = true);
	void encode(const char *, size_t, std::string& , bool add_crlf = true);
	void encode(const unsigned char *, size_t, std::string& , bool add_crlf = true);

	static std::string enBase64( const string &inbuf);

	static std::string deBase64(const std::string& src);
	
	void decode(const std::string&, std::string& );
	void decode(const std::string&, unsigned char *, size_t&);

	size_t decode_length(const std::string& );

private:
	CConvert(const CConvert& ) {}
	CConvert& operator=(const CConvert& ) { return *this; }
	static	const char *bstr;
	static	const char rstr[128];
};