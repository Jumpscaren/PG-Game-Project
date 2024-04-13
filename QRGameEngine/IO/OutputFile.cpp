#include "pch.h"
#include "OutputFile.h"
#include "Vendor/Include/zlib/zlib.h"

OutputFile::OutputFile(const std::string& file_name, const FileMode& file_mode, bool compressed_file) : m_file_name(file_name), m_compressed_file(compressed_file), m_file_mode
(file_mode)
{
	std::string file_mode_text = "rb";
	if (file_mode == FileMode::WRITE)
		file_mode_text = "wb";

	if (m_compressed_file)
		m_file.compressed_file = gzopen(file_name.c_str(), file_mode_text.c_str());
	else
		fopen_s(&m_file.file, file_name.c_str(), file_mode_text.c_str());

	if (!FileExists())
		std::cout << "ERROR: Couldn't create/find file name requested (" << file_name << ")\n";
}

OutputFile::~OutputFile()
{

}

void OutputFile::Close()
{
	if (m_compressed_file)
		gzclose(m_file.compressed_file);
	else
		fclose(m_file.file);
}

bool OutputFile::FileExists()
{
	return m_file.compressed_file != nullptr || m_file.file != nullptr;
}

void OutputFile::Read(void* data, uint32_t data_size)
{
	assert(m_file_mode != FileMode::WRITE);

	if (m_compressed_file)
		gzread(m_file.compressed_file, data, data_size);
	else
		fread(data, data_size, 1, m_file.file);
}

void OutputFile::Write(void* data, uint32_t data_size)
{
	assert(m_file_mode != FileMode::READ);

	if (m_compressed_file)
		gzwrite(m_file.compressed_file, data, data_size);
	else
		fwrite(data, data_size, 1, m_file.file);
}
