
#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <set> 
#include "..\common\UdpPacket.h"
#include "..\common\FileData.h"


using boost::asio::ip::udp;
using namespace boost::filesystem;



class udp_server
{
private:
	boost::mutex * _cachemutex, *_datamutex;
	udp::socket socket_;
	udp::endpoint remote_endpoint_;
	uint32_t fileId = 0;
	boost::array<BYTE, BUFFER_SIZE> recv_buffer_;
	//std::map<uint32_t, FileData> _fileDataMap;
	std::unique_ptr<std::deque<boost::array<BYTE, BUFFER_SIZE>>> _cache;
	std::unique_ptr<std::map<uint32_t, FileData>> _fileDataMap;
	std::set<uint32_t> missingData;
	//std::set<uint32_t> writeFileData;

	void start_receive()
	{
		socket_.async_receive_from(
			boost::asio::buffer(recv_buffer_), remote_endpoint_,
			boost::bind(&udp_server::handle_receive, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}


	void handle_receive(const boost::system::error_code& error,
		std::size_t bytes_transferred)
	{


		if (!error || error == boost::asio::error::message_size)
		{
			_cachemutex->lock();
			_cache->push_back(recv_buffer_);
			_cachemutex->unlock();
			start_receive();

		}
		else
			std::cout << "Error packet " << std::endl;
	}

	void handle_send(boost::shared_ptr<boost::array<BYTE, BUFFER_SIZE>> /*message*/,
		const boost::system::error_code& /*error*/,
		std::size_t bytes_transferred)
	{
		std::cout << "sending message" << std::endl;
	}



public:

	void wait(int mseconds)
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds{ mseconds });
	}




	udp_server(boost::asio::io_context& io_context, boost::mutex* cachemutex, boost::mutex* datamutex)
		: socket_(io_context, udp::endpoint(udp::v4(), 13)), _cachemutex(cachemutex), _datamutex(datamutex)
	{
		_cache = std::make_unique<std::deque<boost::array<BYTE, BUFFER_SIZE>>>();
		_fileDataMap = std::make_unique<std::map<uint32_t, FileData>>();
		start_receive();
	}

	void handleCache()
	{
		try
		{
			while (1)
			{
				//wait(1);
				if (_datamutex->try_lock())
				{
					//boost::mutex::scoped_lock lock(*_mutex);
					if (_cache->size() > 0)
					{

						boost::array<BYTE, BUFFER_SIZE> buf;
						_cachemutex->lock();
						buf = _cache->front();
						//execute async to improve performance
					/*	std::future<UdpPacket> func = std::async(std::launch::async, [&buf]() {

							return udpPacket;
						});*/

						//UdpPacket udpPacket = func.get();
						_cache->pop_front();
						_cachemutex->unlock();
						UdpPacket udpPacket;
						udpPacket.LoadPacket(buf);

						//	func.wait();



						if (udpPacket.GetCommand() == PacketCommand::INIT)
						{
							FileData fileData;
							if (_fileDataMap->count(udpPacket.GetFileId()) == 0)
							{
								fileData.SetFileId(++fileId);
								fileData.SetFileName(udpPacket.GetFileName());
								fileData.SetFileSize(udpPacket.GetFileSize());
								fileData.data_status = DATA_STATUS::RECEIVING_DATA;

								(*_fileDataMap)[fileId] = fileData;
								//remove file if exits
								remove(udpPacket.GetFileName());

							}

							udpPacket.SetCommand(PacketCommand::START);
							udpPacket.SetFileId(fileData.GetFileId());

							// START#FILEID#FILENAME
							boost::shared_ptr<boost::array<BYTE, BUFFER_SIZE>>
								message(new boost::array<BYTE, BUFFER_SIZE>(udpPacket.GetPacket()));

							socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
								boost::bind(&udp_server::handle_send, this, message,
									boost::asio::placeholders::error,
									boost::asio::placeholders::bytes_transferred));

						}



						else if ((*_fileDataMap)[udpPacket.GetFileId()].data_status == DATA_STATUS::RECEIVING_DATA && udpPacket.GetCommand() == PacketCommand::SENDING)
						{
							//check for duplicated packet
							if (!(*_fileDataMap)[udpPacket.GetFileId()].data[udpPacket.GetFileDataIndex()].alreadySet)
							{

								(*_fileDataMap)[udpPacket.GetFileId()].data[udpPacket.GetFileDataIndex()].alreadySet = true;
								(*_fileDataMap)[udpPacket.GetFileId()].data[udpPacket.GetFileDataIndex()].dataChunk = udpPacket.GetData();
								(*_fileDataMap)[udpPacket.GetFileId()].data[udpPacket.GetFileDataIndex()].chunkSize = udpPacket.GetDataSize();
								(*_fileDataMap)[udpPacket.GetFileId()].currentSize += udpPacket.GetDataSize();
								std::cout << "Packet Index " << udpPacket.GetFileDataIndex()
									<< "  " << (*_fileDataMap)[udpPacket.GetFileId()].currentSize
									<< "/" << (*_fileDataMap)[udpPacket.GetFileId()].GetFileSize()
									<< " " << (int)(((float)((*_fileDataMap)[udpPacket.GetFileId()].currentSize) /
									(*_fileDataMap)[udpPacket.GetFileId()].GetFileSize()) * 100) << "%" << std::endl;

								//finish command
								if ((*_fileDataMap)[udpPacket.GetFileId()].currentSize == (*_fileDataMap)[udpPacket.GetFileId()].GetFileSize())
								{
									//UdpPacket udpPacket;
									udpPacket.SetCommand(PacketCommand::FINISHED);
									//udpPacket.SetFileId(fm.first);

									boost::shared_ptr<boost::array<BYTE, BUFFER_SIZE>>
										message(new boost::array<BYTE, BUFFER_SIZE>(udpPacket.GetPacket()));

									socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
										boost::bind(&udp_server::handle_send, this, message,
											boost::asio::placeholders::error,
											boost::asio::placeholders::bytes_transferred));

									missingData.erase(udpPacket.GetFileId());
									//_fileDataMap->erase(udpPacket.GetFileId());
								}
							}

						}
						else if ((*_fileDataMap)[udpPacket.GetFileId()].data_status == DATA_STATUS::RECEIVING_DATA && udpPacket.GetCommand() == PacketCommand::FINISHED_WAITING)
							//else if(udpPacket.GetCommand() == PacketCommand::FINISHED)
						{//enable missing check
							(*_fileDataMap)[udpPacket.GetFileId()].data_status = DATA_STATUS::MISSING_DATA;
							missingData.insert(udpPacket.GetFileId());
						}
						//_packetsmutex->unlock();

					}
					_datamutex->unlock();
				}
			}

		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void handleMissingPackets()
	{
		try
		{
			while (1)
			{
				//	if (_cachemutex->try_lock())
				//	{
				wait(100);
				if (_datamutex->try_lock())
				{
					//_datamutex->lock();

					for (auto &missingId : missingData)
					{

						std::cout << "Client said finished !!! " << std::endl;
						//missing packets, we need to request new ones
						uint32_t t = (*_fileDataMap)[missingId].GetFileSize() / DATA_SIZE;
						if ((*_fileDataMap)[missingId].GetFileSize() % DATA_SIZE)
						{
							t++;
						}

						//check if we have missing packages
						if ((*_fileDataMap)[missingId].GetDataCount() < t)
						{
							std::cout << "Missing some packets " << std::endl;
							uint32_t missing = 0;
							uint16_t start_index = PacketData::MISSINGPACKETCOUNT + sizeof(uint32_t);

							UdpPacket udpPacket;

							for (uint32_t i = 0; i < t; i++)
							{
								//check if is a missing packet
								if (!(*_fileDataMap)[missingId].data[i].alreadySet)
								{
									if (start_index < BUFFER_SIZE)
									{
										std::cout << "Missing packet Index " << i << std::endl;
										udpPacket.AddMissingPackageIndex(i, start_index);
										missing++;
										start_index += sizeof(uint32_t);
									}
								}
							}
							udpPacket.SetCommand(PacketCommand::MISSING);
							udpPacket.SetFileId(missingId);
							udpPacket.SetMissingPacketsCount(missing);


							boost::shared_ptr<boost::array<BYTE, BUFFER_SIZE>>
								message(new boost::array<BYTE, BUFFER_SIZE>(udpPacket.GetPacket()));

							socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
								boost::bind(&udp_server::handle_send, this, message,
									boost::asio::placeholders::error,
									boost::asio::placeholders::bytes_transferred));
						}
						(*_fileDataMap)[missingId].data_status = DATA_STATUS::RECEIVING_DATA;
						missingData.erase(missingId);
						break;
						//	}

					}
					_datamutex->unlock();
				}
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}

	}

	void handleWriteToFile()
	{
		try
		{
			while (1)
			{
				wait(100);
				if (_datamutex->try_lock())
				{
					for (auto &fm : *_fileDataMap)
					{
						if (fm.second.currentSize == fm.second.GetFileSize())
						{
							std::ofstream fou(fm.second.GetFileName(), std::ifstream::binary | std::ofstream::app);

							for (auto &i : fm.second.data)
							{
								fou.write((char *)i.second.dataChunk.c_array(),
									i.second.chunkSize);
								fm.second.lastWritedIndex++;
							}
							fou.close();
							std::cout << "Ok, File " << fm.second.GetFileName() << " saved." << std::endl;
							fm.second.currentSize = 0;
							_fileDataMap->erase(fm.first);

							break;
						}
					}
					_datamutex->unlock();
				}

			}

		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
};






int main()
{
	boost::mutex cachemutex, datamutex;


	try
	{
		boost::asio::io_context io_context;
		udp_server server(io_context, &cachemutex, &datamutex);

		boost::thread threadWriteCache(std::bind(&udp_server::handleCache, &server));
		boost::thread threadMissingPackets(std::bind(&udp_server::handleMissingPackets, &server));
		boost::thread threadWriteToFile(std::bind(&udp_server::handleWriteToFile, &server));

		io_context.run();
		threadWriteCache.join();
		threadWriteToFile.join();
		threadMissingPackets.join();


	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}