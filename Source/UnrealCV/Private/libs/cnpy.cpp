// Copyright (C) 2011  Carl Rogers
// Released under MIT License
// Simplied by Weichao Qiu (qiuwch@gmail.com) from https://github.com/rogersce/cnpy
#include "cnpy.h"
#include <vector>
#include <complex>

using byte = unsigned char;

namespace cnpy {

template 
std::vector<char> create_npy_header<unsigned char>(const unsigned char* data, const std::vector<int> shape);
template 
std::vector<char> create_npy_header<float>(const float* data, const std::vector<int> shape);

/** from: http://www.cplusplus.com/forum/beginner/155821/ */
// template< typename T > std::vector<byte>  to_bytes(const T& object)
// {
// }

char BigEndianTest() {
	unsigned char x[] = { 1,0 };
	short y = *(short*)x;
	return y == 1 ? '<' : '>';
}

// char map_type(const std::type_info& t)
// {
// 	if (t == typeid(float)) return 'f';
// 	if (t == typeid(double)) return 'f';
// 	if (t == typeid(long double)) return 'f';
//
// 	if (t == typeid(int)) return 'i';
// 	if (t == typeid(char)) return 'i';
// 	if (t == typeid(short)) return 'i';
// 	if (t == typeid(long)) return 'i';
// 	if (t == typeid(long long)) return 'i';
//
// 	if (t == typeid(unsigned char)) return 'u';
// 	if (t == typeid(unsigned short)) return 'u';
// 	if (t == typeid(unsigned long)) return 'u';
// 	if (t == typeid(unsigned long long)) return 'u';
// 	if (t == typeid(unsigned int)) return 'u';
//
// 	if (t == typeid(bool)) return 'b';
//
// 	if (t == typeid(std::complex<float>)) return 'c';
// 	if (t == typeid(std::complex<double>)) return 'c';
// 	if (t == typeid(std::complex<long double>)) return 'c';
//
// 	else return '?';
// }

template<typename T>
std::string tostring(T i) {
	std::stringstream s;
	s << i;
	return s.str();
}

template<>
std::vector<char>& operator+=(std::vector<char>& lhs, const std::string rhs) {
	lhs.insert(lhs.end(), rhs.begin(), rhs.end());
	return lhs;
}

template<>
std::vector<char>& operator+=(std::vector<char>& lhs, const char* rhs) {
	//write in little endian
	size_t len = strlen(rhs);
	lhs.reserve(len);
	for (size_t byte = 0; byte < len; byte++) {
		lhs.push_back(rhs[byte]);
	}
	return lhs;
}

/** Modified from npy_save */
// template<typename T> void npy_dump(const T* data, const unsigned int* shape, const unsigned int ndims) {
// 	std::vector<char> header = create_npy_header(data, shape, ndims);
//
// 	unsigned int nels = 1;
// 	for (int i = 0; i < ndims; i++) nels *= shape[i];
// }

char map_type(const double* ) { return 'f'; }
char map_type(const float* ) { return 'f'; }
char map_type(const long double* ) { return 'f'; }

char map_type(const int* ) { return 'i'; }
char map_type(const char* ) { return 'i'; }
char map_type(const short* ) { return 'i'; }
char map_type(const long* ) { return 'i'; }
char map_type(const long long* ) { return 'i'; }

char map_type(const unsigned int* ) { return 'u'; }
char map_type(const unsigned char* ) { return 'u'; }
char map_type(const unsigned short* ) { return 'u'; }
char map_type(const unsigned long* ) { return 'u'; }
char map_type(const unsigned long long* ) { return 'u'; }

char map_type(const bool* ) { return 'b'; }

/** data is mainly used for determining T */
template<typename T> 
std::vector<char> create_npy_header(const T* data, const std::vector<int> shape)
{
	std::vector<char> dict;
	dict += "{'descr': '";
	dict += BigEndianTest();
	// dict += map_type(typeid(T));
	dict += map_type(data);
	dict += tostring(sizeof(T));
	dict += "', 'fortran_order': False, 'shape': (";
	dict += tostring(shape[0]);

	size_t ndims = shape.size();
	for (int i = 1; i < ndims; i++) {
		dict += ", ";
		dict += tostring(shape[i]);
	}
	if (ndims == 1) dict += ",";
	dict += "), }";
	//pad with spaces so that preamble+dict is modulo 16 bytes. preamble is 10 bytes. dict needs to end with \n
	int remainder = 16 - (10 + dict.size()) % 16;
	dict.insert(dict.end(), remainder, ' ');
	dict.back() = '\n';

	std::vector<char> header;
	header += (char)0x93u;
	header += "NUMPY";
	header += (char)0x01u; //major version of numpy format
	header += (char)0x00u; //minor version of numpy format
	header += (unsigned short)dict.size();
	header.insert(header.end(), dict.begin(), dict.end());

	return header;
}
}
