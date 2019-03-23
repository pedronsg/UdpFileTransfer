#include "UdpPacket.h"



UdpPacket::UdpPacket()
{
	_command= PacketCommand::INIT;
	_fileSize =0;
	_fileId = 0;
	_fileDataIndex = 0;
	_filename = "";
	_filenameLength = 0;
	_totalFilePackets = 0;
	_missingPacketsCount = 0;
	_dataSize=0;
	_data = { 0 };
	_packet = { 0 };
}



void UdpPacket::appendToArray(std::string str, boost::array<BYTE, BUFFER_SIZE> &arrayOfByte, const int index)
{
	for (int i = 0; i < str.length(); i++)
	{
		arrayOfByte[i + index] = str[i];
	}
}

std::string UdpPacket::StringFromArray(const int startIndex, boost::array<BYTE, BUFFER_SIZE> &arrayOfByte, const int size)
{
	std::string str = "";
	for (int i = 0; i < size; i++)
	{
		str = str + (char)arrayOfByte[i + startIndex];
	}
	return str;
}

void UdpPacket::SetCommand(PacketCommand cmd)
{
	_command = cmd;
	_packet[PacketData::CMD] = (BYTE)cmd;
};

PacketCommand UdpPacket::GetCommand()
{
	return _command;
};

void UdpPacket::AddMissingPackageIndex(std::uint32_t missingIndex, uint16_t bufferPos)
{
	std::memcpy(&_packet[bufferPos], &missingIndex, sizeof(std::uint32_t));
};

std::vector<uint32_t> UdpPacket::GetMissingPackageList()
{
	return _missingPacketsList;
};


void UdpPacket::SetFileSize(std::uint32_t filesize)
{
	_fileSize = filesize;
	std::memcpy(&_packet[PacketData::FILESIZE], &filesize, sizeof(std::uint32_t));
};

std::uint32_t UdpPacket::GetFileSize()
{
	return _fileSize;
};
void UdpPacket::SetTotalFilePackets(std::uint32_t totalFilePackets)
{
	_totalFilePackets = totalFilePackets;
	std::memcpy(&_packet[PacketData::FILETOTALPACKETS], &totalFilePackets, sizeof(std::uint32_t));
};

std::uint32_t UdpPacket::GetTotalFilePackets()
{
	return _totalFilePackets;
};

void UdpPacket::SetMissingPacketsCount(std::uint32_t missingPacketsCount)
{
	_missingPacketsCount = missingPacketsCount;
	std::memcpy(&_packet[PacketData::MISSINGPACKETCOUNT], &missingPacketsCount, sizeof(std::uint32_t));
};

std::uint32_t UdpPacket::GetMissingPacketsCount()
{
	return _missingPacketsCount;
};


void UdpPacket::SetFileId(std::uint32_t fileId)
{
	_fileId = fileId;
	std::memcpy(&_packet[PacketData::FILEID], &fileId, sizeof(std::uint32_t));
};

std::uint32_t UdpPacket::GetFileId()
{
	return _fileId;
};

void UdpPacket::SetFileDataIndex(std::uint32_t &filedataIndex)
{
	_fileDataIndex = filedataIndex;
	std::memcpy(&_packet[PacketData::FILEDATAINDEX], &filedataIndex, sizeof(std::uint32_t));
};

std::uint32_t UdpPacket::GetFileDataIndex()
{
	return _fileDataIndex;
};

void UdpPacket::SetFileName(std::string filename)
{
	_filename = filename;
	appendToArray(filename, _packet, PacketData::FILENAME);
};

std::string UdpPacket::GetFileName()
{
	return _filename;
};

void UdpPacket::SetFileNameLength(const BYTE &filenameLen)
{
	_filenameLength = filenameLen;
	_packet[PacketData::FILENAMELEN] = filenameLen;
}

BYTE UdpPacket::GetFileNameLength()
{
	return _filenameLength;
}

void UdpPacket::SetDataSize(const std::uint16_t &datasize)
{
	_dataSize = datasize;
	std::memcpy(&_packet[PacketData::DATASIZE], &datasize, sizeof(std::uint16_t));
}

std::uint16_t UdpPacket::GetDataSize()
{
	return _dataSize;
}

void UdpPacket::SetData(boost::array<BYTE, DATA_SIZE> data)
{
	_data = data;
	std::memcpy(&_packet[PacketData::DATA], &data, DATA_SIZE);
}

boost::array<BYTE, DATA_SIZE> UdpPacket::GetData()
{
	return _data;
}

void UdpPacket::LoadPacket(boost::array<BYTE, BUFFER_SIZE> &pct)
{
	std::memcpy(&_command, &pct[PacketData::CMD], sizeof(BYTE));

	//std::memcpy(&_missingPacketsList, &pct[PacketData::MISSINGPACKETCOUNT+ sizeof(uint32_t)], sizeof(uint32_t));
	std::memcpy(&_fileId, &pct[PacketData::FILEID], sizeof(uint32_t));

	//SENDING#FILEID#CHUNKID#CHUNKSIZE#CHUNKDATA
	if (_command == PacketCommand::SENDING)
	{
		std::memcpy(&_dataSize, &pct[PacketData::DATASIZE], sizeof(uint16_t));
		std::memcpy(&_data, &pct[PacketData::DATA], DATA_SIZE);
		std::memcpy(&_fileDataIndex, &pct[PacketData::FILEDATAINDEX], sizeof(uint32_t));
	}
	else
		//INIT#FILESIZE#FILENAMELEN#FILENAME
		if (_command == PacketCommand::INIT)
		{
			std::memcpy(&_fileSize, &pct[PacketData::FILESIZE], sizeof(uint32_t));
			std::memcpy(&_filenameLength, &pct[PacketData::FILENAMELEN], sizeof(BYTE));
			_filename = StringFromArray(PacketData::FILENAME, pct, _filenameLength);
		}
		else
			if (_command == PacketCommand::MISSING)
			{
				std::memcpy(&_missingPacketsCount, &pct[PacketData::MISSINGPACKETCOUNT], sizeof(uint32_t));
				int32_t i = PacketData::MISSINGPACKETCOUNT + sizeof(uint32_t);
				int count = 0;
				while (i < BUFFER_SIZE && count < _missingPacketsCount)
				{
					int32_t missedId;
					std::memcpy(&missedId, &pct[i], sizeof(uint32_t));
					_missingPacketsList.push_back(missedId);
					i += sizeof(uint32_t);
					count++;
				}
			}
			else if (_command == PacketCommand::FINISHED_WAITING)
			{
				std::memcpy(&_totalFilePackets, &pct[PacketData::FILETOTALPACKETS], sizeof(uint32_t));
			}
	_packet = pct;
}

boost::array<BYTE, BUFFER_SIZE> UdpPacket::GetPacket()
{
	return _packet;
};