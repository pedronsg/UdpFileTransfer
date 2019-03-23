#include "FileData.h"
#include <boost/asio.hpp>
#include <boost/filesystem.hpp> 
#include <fstream>
#include <iostream>

using namespace boost::filesystem;

FileData::FileData()
{
	_fileSize=0;
	_fileId=0;
	_filename="";
	_fileFullPath="";
	currentSize = 0;
	data_status = DATA_STATUS::NONE_DATA;
	lastWritedIndex = -1;
}




FileData::~FileData()
{
}


void FileData::SendFile(boost::array<BYTE, BUFFER_SIZE> packet, udp::socket &socket, udp::endpoint &receiver_endpoint)
{
	FILE *fp = fopen(_fileFullPath.c_str(), "rb");

	if (fp) {
		char  byte_buffer[DATA_SIZE];
		std::uint16_t  bytes_read = 0;
		std::uint32_t i = 0;

		while ((bytes_read = fread(&byte_buffer, 1, DATA_SIZE, fp)) > 0)
		{
			std::memcpy(&packet[PacketData::FILEDATAINDEX], &i, sizeof(std::uint32_t));
			std::memcpy(&packet[PacketData::DATASIZE], &bytes_read, sizeof(std::uint16_t));
			std::memcpy(&packet[PacketData::DATA], &byte_buffer, bytes_read);
			socket.send_to(boost::asio::buffer(packet), receiver_endpoint);
			i++;
		}
	}
}

 uint16_t FileData::GetPacketFromFile(int32_t packetIndex, boost::array<BYTE, DATA_SIZE> &data, uint32_t fileSize)
{	
	ifstream file(_fileFullPath, std::ios::in | std::ios::binary);

	file.seekg(packetIndex*DATA_SIZE, std::ios::beg);

	char A[DATA_SIZE];
	file.read(A, DATA_SIZE);
	std::memcpy(&data, &A, sizeof(DATA_SIZE));

	
	if (fileSize - (packetIndex * DATA_SIZE)< DATA_SIZE)
	{
		return fileSize - (packetIndex * DATA_SIZE);
	}
	return DATA_SIZE;
}

 bool FileData::AppendToFile(int32_t packetIndex, boost::array<BYTE, DATA_SIZE> data, const int16_t datasize)
 {

	 fstream file(_filename, std::ios::out | std::ios::binary | std::ios::app);
	 file.seekg(0, std::ios::end);
	 uint32_t s = file.tellg();
	 if (file.tellg() != packetIndex * DATA_SIZE)
	 {
		 return false;
	 }

	 char chunk[DATA_SIZE];
	 std::memcpy(&chunk, &data, datasize);
	 file.write(chunk, DATA_SIZE);
	 return true;
 }


void FileData::SetFileProperties(std::string filePath)
{
	_fileSize = file_size(filePath);
	boost::filesystem::path p(filePath);
	_filename = p.filename().string();
	_fileFullPath = filePath;
}


std::uint32_t  FileData::GetDataCount()
{
	uint32_t i = 0;
	i = currentSize / DATA_SIZE;
	if (currentSize%DATA_SIZE)
		i++;
	return i;
}

void FileData::SetFileSize(std::uint32_t fileSize)
{
	_fileSize = fileSize;
}
std::uint32_t  FileData::GetFileSize()
{
	return _fileSize;
}

void FileData::SetFileId(std::uint32_t fileId)
{
	_fileId = fileId;
}
std::uint32_t  FileData::GetFileId()
{
	return _fileId;
}


void FileData::SetFileName(std::string fileName)
{
	_filename = fileName;
}
std::string FileData::GetFileName()
{
	return _filename;
}
void FileData::SetFileFullPath(std::string filepath)
{
	_fileFullPath = filepath;
}
std::string FileData::GetFileFullPath()
{
	return _fileFullPath;
}

void FileData::SetFileId(boost::array<BYTE, BUFFER_SIZE> &packet)
{
	std::memcpy(&_fileId, &packet[PacketData::FILEID], sizeof(std::uint32_t));
};