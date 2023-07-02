#include "pch.h"
#include "Output.h"
#include "Vendor/Include/zlib/zlib.h"

OutputFile Output::CreateOutputFile(const std::string& file_name)
{
	return OutputFile(file_name, OutputFile::FileMode::WRITE, false);
}

OutputFile Output::CreateCompressedOutputFile(const std::string& file_name)
{
	return OutputFile(file_name, OutputFile::FileMode::WRITE);
}

OutputFile Output::LoadCompressedOutputFile(const std::string& file_name)
{
	return OutputFile(file_name, OutputFile::FileMode::READ);
}

void Output::OutputFileCompressed(const std::string& file_name, void* data, uint32_t data_size)
{
	gzFile outfile = gzopen(file_name.c_str(), "wb");
	gzwrite(outfile, data, data_size);
	gzclose(outfile);
}
