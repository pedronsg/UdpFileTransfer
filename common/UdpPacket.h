#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <vector>

#define BUFFER_SIZE 32012 //506
#define DATA_SIZE (BUFFER_SIZE-12)


enum PacketData
{
	CMD=0,
	FILEID=1,
	FILESIZE = 1,
	FILEDATAINDEX=5,
	FILENAMELEN=5,
	MISSINGPACKETCOUNT = 5,
	FILENAME=6,
	DATASIZE=9,
	FILETOTALPACKETS=11,
	DATA=11
};


enum PacketCommand
{
	NONE,
	INIT,
	START,
	SENDING,
	MISSING,
	FINISHED_WAITING,
	FINISHED

};

class UdpPacket
{
private:
	PacketCommand _command;
	std::uint32_t _fileSize;
	std::uint32_t _fileId;
	std::uint32_t _fileDataIndex;
	std::uint32_t _totalFilePackets;
	std::uint32_t _missingPacketsCount;
	
	std::string _filename;
	BYTE _filenameLength;
	std::uint16_t _dataSize;
	boost::array<BYTE, DATA_SIZE> _data;
	boost::array<BYTE, BUFFER_SIZE> _packet;
	std::vector<uint32_t> _missingPacketsList;

	void appendToArray(std::string str, boost::array<BYTE, BUFFER_SIZE> &arrayOfByte, const int index);
	std::string StringFromArray(const int startIndex, boost::array<BYTE, BUFFER_SIZE> &arrayOfByte, const int size);
public:
	
	void SetCommand(PacketCommand cmd);
	PacketCommand GetCommand();
	void AddMissingPackageIndex(std::uint32_t missingIndex, uint16_t bufferPos);
	std::vector<uint32_t> GetMissingPackageList();
	void SetFileSize(std::uint32_t filesize);
	std::uint32_t GetFileSize();
	void SetTotalFilePackets(std::uint32_t totalFilePackets);
	std::uint32_t GetTotalFilePackets();
	void SetMissingPacketsCount(std::uint32_t missingPacketsCount);
	std::uint32_t GetMissingPacketsCount();
	void SetFileId(std::uint32_t fileId);
	std::uint32_t GetFileId();
	void SetFileDataIndex(std::uint32_t &filedataIndex);
	std::uint32_t GetFileDataIndex();
	void SetFileName(std::string filename);
	std::string GetFileName();
	void SetFileNameLength(const BYTE &filenameLen);
	BYTE GetFileNameLength();
	void SetDataSize(const std::uint16_t &datasize);
	std::uint16_t GetDataSize();
	void SetData(boost::array<BYTE, DATA_SIZE> data);
	boost::array<BYTE, DATA_SIZE> GetData();
	void LoadPacket(boost::array<BYTE, BUFFER_SIZE> &pct);
	boost::array<BYTE, BUFFER_SIZE> GetPacket();

	UdpPacket();
};





