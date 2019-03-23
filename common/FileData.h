#pragma once

#include <cstdint>
#include <string>
#include <boost/array.hpp>
#include "UdpPacket.h"



using boost::asio::ip::udp;

enum DATA_STATUS
{
	NONE_DATA,
	INIT_DATA,
	RECEIVING_DATA,
	MISSING_DATA,
	FINISH_DATA
};


struct DataChunks
{
	boost::array<BYTE, DATA_SIZE> dataChunk;
	std::uint16_t  chunkSize;
	bool alreadySet;

	DataChunks() {
		chunkSize = 0;
		dataChunk = { 0 };
		alreadySet = false;
	}
};


class FileData
{
private:
	std::uint32_t  _fileSize;
	std::uint32_t  _fileId;
	std::string _filename;
	std::string _fileFullPath;
	

public:
	//index,data
	std::map<std::uint32_t, DataChunks> data;
	int64_t lastWritedIndex;

	std::uint32_t currentSize;
	DATA_STATUS data_status;

	FileData();
	~FileData();

	uint16_t GetPacketFromFile(int32_t packetIndex, boost::array<BYTE, DATA_SIZE> &data, uint32_t fileSize);
	bool AppendToFile(int32_t packetIndex, boost::array<BYTE, DATA_SIZE> data, const int16_t datasize);
	std::uint32_t  GetDataCount();
	void SetFileSize(std::uint32_t fileSize);
	std::uint32_t GetFileSize();
	void SetFileId(std::uint32_t fileId);
	std::uint32_t  GetFileId();
	void SetFileName(std::string fileName);
	std::string GetFileName();
	void SetFileFullPath(std::string filepath);
	std::string GetFileFullPath();
	void SendFile(boost::array<BYTE, BUFFER_SIZE> packet, udp::socket &socket, udp::endpoint &receiver_endpoint);
	void SetFileProperties(std::string filePath);
	void SetFileId(boost::array<BYTE, BUFFER_SIZE> &packet);

};

