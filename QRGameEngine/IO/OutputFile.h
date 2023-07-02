#pragma once

struct gzFile_s;

class OutputFile
{
public:
	enum class FileMode
	{
		READ = 0,
		WRITE = 1,
		READWRITE = 2,
	};

private:
	union
	{
		gzFile_s* compressed_file;
		FILE* file;
	} m_file;
	bool m_compressed_file;
	std::string m_file_name;
	FileMode m_file_mode;

public:
	OutputFile(const std::string& file_name, const FileMode& file_mode, bool compressed_file = true);
	~OutputFile();

	void Write(void* data, uint32_t data_size);
	template<typename T>
	void Write(const T& data);

	void Read(void* data, uint32_t data_size);
	template<typename T>
	T Read()
	{
		T data;
		Read((void*)&data, sizeof(T));
		return data;
	}

	void Close();
};

template<typename T>
inline void OutputFile::Write(const T& data)
{
	Write((void*)&data, sizeof(T));
}

//template<typename T>
//inline T Read()
//{
//	//T data;
//	//Read((void*) &data, sizeof(T));
//
//	return T();
//}

