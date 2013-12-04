
/** \file Base64.cpp
 **	\date  2004-02-13
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004-2007  Anders Hedstrom

This library is made available under the terms of the GNU GPL.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "Convert.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


const char *CConvert::bstr =
	"ABCDEFGHIJKLMNOPQ"
	"RSTUVWXYZabcdefgh"
	"ijklmnopqrstuvwxy"
	"z0123456789+/";

const char CConvert::rstr[] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  62,   0,   0,   0,  63, 
	 52,  53,  54,  55,  56,  57,  58,  59,  60,  61,   0,   0,   0,   0,   0,   0, 
	  0,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, 
	 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,   0,   0,   0,   0,   0, 
	  0,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40, 
	 41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,   0,   0,   0,   0,   0};


CConvert::CConvert()
{
}


void CConvert::encode(FILE *fil, std::string& output, bool add_crlf)
{
	size_t remain;
	size_t i = 0;
	size_t o = 0;
	char input[4];

	output = "";
	remain = fread(input,1,3,fil);
	while (remain > 0)
	{
		if (add_crlf && o && o % 76 == 0)
			output += "\n";
		switch (remain)
		{
		case 1:
			output += bstr[ ((input[i] >> 2) & 0x3f) ];
			output += bstr[ ((input[i] << 4) & 0x30) ];
			output += "==";
			break;
		case 2:
			output += bstr[ ((input[i] >> 2) & 0x3f) ];
			output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
			output += bstr[ ((input[i + 1] << 2) & 0x3c) ];
			output += "=";
			break;
		default:
			output += bstr[ ((input[i] >> 2) & 0x3f) ];
			output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
			output += bstr[ ((input[i + 1] << 2) & 0x3c) + ((input[i + 2] >> 6) & 0x03) ];
			output += bstr[ (input[i + 2] & 0x3f) ];
		}
		o += 4;
		//
		remain = fread(input,1,3,fil);
	}
}


void CConvert::encode(const std::string& str_in, std::string& str_out, bool add_crlf)
{
	encode(str_in.c_str(), str_in.size(), str_out, add_crlf);
}


void CConvert::encode(const char* input,size_t l,std::string& output, bool add_crlf)
{
	size_t i = 0;
	size_t o = 0;
	
	output = "";
	while (i < l)
	{
		size_t remain = l - i;
		if (add_crlf && o && o % 76 == 0)
			output += "\n";
		switch (remain)
		{
		case 1:
			output += bstr[ ((input[i] >> 2) & 0x3f) ];
			output += bstr[ ((input[i] << 4) & 0x30) ];
			output += "==";
			break;
		case 2:
			output += bstr[ ((input[i] >> 2) & 0x3f) ];
			output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
			output += bstr[ ((input[i + 1] << 2) & 0x3c) ];
			output += "=";
			break;
		default:
			output += bstr[ ((input[i] >> 2) & 0x3f) ];
			output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
			output += bstr[ ((input[i + 1] << 2) & 0x3c) + ((input[i + 2] >> 6) & 0x03) ];
			output += bstr[ (input[i + 2] & 0x3f) ];
		}
		o += 4;
		i += 3;
	}
}


void CConvert::encode(const unsigned char* input,size_t l,std::string& output,bool add_crlf)
{
	size_t i = 0;
	size_t o = 0;
	
	output = "";
	while (i < l)
	{
		size_t remain = l - i;
		if (add_crlf && o && o % 76 == 0)
			output += "\n";
		switch (remain)
		{
		case 1:
			output += bstr[ ((input[i] >> 2) & 0x3f) ];
			output += bstr[ ((input[i] << 4) & 0x30) ];
			output += "==";
			break;
		case 2:
			output += bstr[ ((input[i] >> 2) & 0x3f) ];
			output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
			output += bstr[ ((input[i + 1] << 2) & 0x3c) ];
			output += "=";
			break;
		default:
			output += bstr[ ((input[i] >> 2) & 0x3f) ];
			output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
			output += bstr[ ((input[i + 1] << 2) & 0x3c) + ((input[i + 2] >> 6) & 0x03) ];
			output += bstr[ (input[i + 2] & 0x3f) ];
		}
		o += 4;
		i += 3;
	}
}


void CConvert::decode(const std::string& input,std::string& output)
{
	size_t i = 0;
	size_t l = input.size();
	
	output = "";
	while (i < l)
	{
		while (i < l && (input[i] == 13 || input[i] == 10))
			i++;
		if (i < l)
		{
			char b1 = (char)((rstr[(int)input[i]] << 2 & 0xfc) +
					(rstr[(int)input[i + 1]] >> 4 & 0x03));
			output += b1;
			if (input[i + 2] != '=')
			{
				char b2 = (char)((rstr[(int)input[i + 1]] << 4 & 0xf0) +
						(rstr[(int)input[i + 2]] >> 2 & 0x0f));
				output += b2;
			}
			if (input[i + 3] != '=')
			{
				char b3 = (char)((rstr[(int)input[i + 2]] << 6 & 0xc0) +
						rstr[(int)input[i + 3]]);
				output += b3;
			}
			i += 4;
		}
	}
}


void CConvert::decode(const std::string& input, unsigned char *output, size_t& sz)
{
	size_t i = 0;
	size_t l = input.size();
	size_t j = 0;
	
	while (i < l)
	{
		while (i < l && (input[i] == 13 || input[i] == 10))
			i++;
		if (i < l)
		{
			unsigned char b1 = (unsigned char)((rstr[(int)input[i]] << 2 & 0xfc) +
					(rstr[(int)input[i + 1]] >> 4 & 0x03));
			if (output)
			{
				output[j] = b1;
			}
			j++;
			if (input[i + 2] != '=')
			{
				unsigned char b2 = (unsigned char)((rstr[(int)input[i + 1]] << 4 & 0xf0) +
						(rstr[(int)input[i + 2]] >> 2 & 0x0f));
				if (output)
				{
					output[j] = b2;
				}
				j++;
			}
			if (input[i + 3] != '=')
			{
				unsigned char b3 = (unsigned char)((rstr[(int)input[i + 2]] << 6 & 0xc0) +
						rstr[(int)input[i + 3]]);
				if (output)
				{
					output[j] = b3;
				}
				j++;
			}
			i += 4;
		}
	}
	sz = j;
}
std::string CConvert::deBase64(const std::string& src)
{
	std::string res;
	CConvert base64;
	base64.decode(src,res);
	return res;
}

std::string CConvert::enBase64(const string &inbuf)
{
	std::string res;
	CConvert base64;
	base64.encode(inbuf,res);
	return res;
}

size_t CConvert::decode_length(const std::string& str64)
{
	if (str64.empty() || str64.size() % 4)
		return 0;
	size_t l = 3 * (str64.size() / 4 - 1) + 1;
	if (str64[str64.size() - 2] != '=')
		l++;
	if (str64[str64.size() - 1] != '=')
		l++;
	return l;
}


#ifdef SOCKETS_NAMESPACE
}
#endif




//#include "stdafx.h"
//#include "Convert.h"
//
//void CConvert::_enBase64Help(unsigned char chasc[3],unsigned char chuue[4])
//{
//	int i, k=2;
//	unsigned char t = 0; 
//	for(i=0; i<3; i++)
//	{
//		*(chuue+i) = *(chasc+i)>>k;
//		*(chuue+i) |= t;
//		t = *(chasc+i)<<(8-k);
//		t >>= 2;
//		k += 2;
//	}
//	*(chuue+3) = *(chasc+2)&63;
//
//	for ( i=0; i<4; i++ ) {
//		if ( (*(chuue+i)<=128) && (*(chuue+i)<=25) ) {
//			*(chuue+i) += 65; // 'A'-'Z'
//		} else if ( (*(chuue+i)>=26) && (*(chuue+i)<=51) ) {
//			*(chuue+i) += 71; // 'a'-'z'
//		} else if ( (*(chuue+i)>=52) && (*(chuue+i)<=61) ) {
//			*(chuue+i) -= 4; // 0-9
//		} else if ( *(chuue+i)==62 ) {
//			*(chuue+i) = 43; // +
//		} else if ( *(chuue+i)==63 ) {
//			*(chuue+i) = 47; // /
//		}
//	}
//} 
//void CConvert::_deBase64Help(unsigned char chuue[4],unsigned char chasc[3]) {
//	int i,k=2;
//	unsigned char t=0; 
//
//	for( i=0; i<4; i++) {
//		if ( (*(chuue+i)>=65) && (*(chuue+i)<=90)) 
//			*(chuue+i) -= 65; // 'A'-'Z' -> 0-25
//		else if ( (*(chuue+i)>=97)&&(*(chuue+i)<=122) ) 
//			*(chuue+i) -= 71; // 'a'-'z' -> 26-51
//		else if ( (*(chuue+i)>=48)&&(*(chuue+i)<=57) ) 
//			*(chuue+i) += 4; // '0'-'9' -> 52-61
//		else if ( *(chuue+i)==43 ) 
//			*(chuue+i) = 62; // + -> 62
//		else if ( *(chuue+i)==47 ) 
//			*(chuue+i) = 63; // / -> 63
//		else if ( *(chuue+i)==61 ) 
//			*(chuue+i) = 0;  // = -> 0  Note: 'A'和'='都对应了0
//	}
//	for ( i=0; i<3; i++ ) {
//		*(chasc+i) = *(chuue+i) << k;
//		k += 2;
//		t = *(chuue+i+1) >> (8-k);
//		*(chasc+i) |= t;
//	}
//} 
//
//string CConvert::enBase64( const char* inbuf, size_t inbufLen ) {
//	if(inbufLen==0)
//	{
//		return "";
//	}
//	string outStr;
//	unsigned char in[8];
//	unsigned char out[8];
//	out[4] = 0;
//	size_t blocks = inbufLen / 3;
//	for ( size_t i=0; i<blocks; i++ ) {
//		in[0] = inbuf[i*3];
//		in[1] = inbuf[i*3+1];
//		in[2] = inbuf[i*3+2];
//		_enBase64Help(in,out);
//		outStr += out[0];
//		outStr += out[1];
//		outStr += out[2];
//		outStr += out[3];
//	}
//	if ( inbufLen % 3 == 1 ) {
//		in[0] = inbuf[inbufLen-1];
//		in[1] = 0;
//		in[2] = 0;
//		_enBase64Help(in,out);
//		outStr += out[0];
//		outStr += out[1];
//		outStr += '=';
//		outStr += '=';
//	} else if ( inbufLen % 3 == 2 ) {
//		in[0] = inbuf[inbufLen-2];
//		in[1] = inbuf[inbufLen-1];
//		in[2] = 0;
//		_enBase64Help(in,out);
//		outStr += out[0];
//		outStr += out[1];
//		outStr += out[2];
//		outStr += '=';
//	}
//	return string(outStr);
//} 
//
//string CConvert::enBase64( const string &inbuf)
//{
//	return CConvert::enBase64( inbuf.c_str(), inbuf.size() );
//} 
//
//int CConvert::deBase64( string src, char* outbuf ) { 
//
//	// Break when the incoming base64 coding is wrong
//	if((src.size() % 4 )!= 0 ||src.size()==0)
//	{
//		return 0;
//	} 
//
//	unsigned char in[4];
//	unsigned char out[3]; 
//
//	size_t blocks = src.size()/4;
//	for ( size_t i=0; i<blocks; i++ ) {
//		in[0] = src[i*4];
//		in[1] = src[i*4+1];
//		in[2] = src[i*4+2];
//		in[3] = src[i*4+3];
//		_deBase64Help(in,out);
//		outbuf[i*3]   = out[0];
//		outbuf[i*3+1] = out[1];
//		outbuf[i*3+2] = out[2];
//	}
//	int length = src.size() / 4 * 3;
//	if ( src[src.size()-1] == '=' ) {
//		length--;
//		if ( src[src.size()-2] == '=' ) {
//			length--;
//		}
//	}
//	return length;
//}
//string CConvert::deBase64( string src)
//{
//	if (src.empty())
//	{
//		return src;
//	}
//	char * buf=new char[src.length()*2];
//	int len=deBase64(src,buf);
//	buf[len]='\0';
//	string result=string(buf,len);
//	delete [] buf;
//	return result;
//} 
//CString CConvert::enBase64(CString inbuf)
//{
//	CString strResult;
//	strResult.Format("%s",enBase64((string)inbuf.GetBuffer()).c_str());
//	return strResult;
//}
//CString CConvert::deBase64(CString src)
//{
//   CString temp;
//   string str=deBase64((string)src.GetBuffer());
//   temp.Format("%s",str.c_str());
//   return temp;
//}
