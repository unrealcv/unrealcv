// Copyright (C) 2011  Carl Rogers
// Released under MIT License
// Simplied by Weichao Qiu (qiuwch@gmail.com) from https://github.com/rogersce/cnpy
#pragma once
#include <string>
#include <vector>
#include <string>

namespace cnpy {
	template<typename T>
	std::vector<char> create_npy_header(const T* data, const std::vector<int> shape);

	template<typename T>
	std::vector<char>& operator+=(std::vector<char>& lhs, const T rhs) {
		//write in little endian
		for (char byte = 0; byte < sizeof(T); byte++) {
			char val = *((char*)&rhs + byte);
			lhs.push_back(val);
		}
		return lhs;
	}
	template<>
	std::vector<char>& operator+=(std::vector<char>& lhs, const std::string rhs);
	template<>
	std::vector<char>& operator+=(std::vector<char>& lhs, const char* rhs);
}
