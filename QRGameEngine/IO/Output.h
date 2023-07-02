#pragma once
#include "OutputFile.h"

class Output
{
public:
	static OutputFile CreateOutputFile(const std::string& file_name);
	static OutputFile CreateCompressedOutputFile(const std::string& file_name);
	static OutputFile LoadCompressedOutputFile(const std::string& file_name);
	static void OutputFileCompressed(const std::string& file_name, void* data, uint32_t data_size);
};

