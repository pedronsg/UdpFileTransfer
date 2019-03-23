#include <iostream>
#include <stdio.h>
#include "..\common\UdpPacket.h"
#include "..\common\FileData.h"
#include <conio.h>


using boost::asio::ip::udp;



int main(int argc, char* argv[])
{
	try
	{
		
		if (argc != 3)
		{
			std::cerr << "Usage: client <host> <filename>" << std::endl;
			return 1;
		}
		
		FileData fileData;
		fileData.SetFileProperties(argv[2]);

		UdpPacket udpPacket;

		boost::asio::io_service io_service;
		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), argv[1], "daytime");
		udp::endpoint receiver_endpoint = *resolver.resolve(query);
		udp::socket socket(io_service);
		socket.open(udp::v4());

		//command to send
		//INIT#FILESIZE#FILENAMELEN#FILENAME
		udpPacket.SetCommand(PacketCommand::INIT);
		udpPacket.SetFileSize(fileData.GetFileSize());
		udpPacket.SetFileNameLength(fileData.GetFileName().length());
		udpPacket.SetFileName(fileData.GetFileName());
		//send to server
		socket.send_to(boost::asio::buffer(udpPacket.GetPacket()), receiver_endpoint);



		boost::array<BYTE, BUFFER_SIZE> recv_buf;
		udp::endpoint sender_endpoint;
		char c = '\0';
		while (c != 27)
		{
			size_t len = socket.receive_from(
				boost::asio::buffer(recv_buf), sender_endpoint);

			udpPacket.LoadPacket(recv_buf);

			switch (udpPacket.GetCommand())
			{
			case PacketCommand::START: // START#FILEID#FILENAME
				std::cout << "Sending file..." << std::endl;
				//set file id from server
				fileData.SetFileId(recv_buf);
			case PacketCommand::SENDING:
			{
				//command to send
				//SENDING#FILEID#CHUNKID#CHUNKSIZE#CHUNKDATA
				udpPacket.SetCommand(PacketCommand::SENDING);
				udpPacket.SetFileId(fileData.GetFileId());
				fileData.SendFile(udpPacket.GetPacket(), socket, receiver_endpoint);
				
				uint32_t nitens = fileData.GetFileSize() / DATA_SIZE;
				if (fileData.GetFileSize() % DATA_SIZE)
				{
					nitens++;
				}
				udpPacket.SetTotalFilePackets(nitens);
				Sleep(1);
				std::cout << "File transmission ended..." << " total packets= " << nitens << std::endl;
				std::cout << "Waiting for confirmation... " << std::endl;
				udpPacket.SetCommand(PacketCommand::FINISHED_WAITING);
			
				socket.send_to(boost::asio::buffer(udpPacket.GetPacket()), receiver_endpoint);
				Sleep(1);

			}
			break;
			case PacketCommand::MISSING:
			{
				std::cout << "Missing " << udpPacket.GetMissingPacketsCount() << " packets ---> " ;
				std::vector<uint32_t> v = udpPacket.GetMissingPackageList();
				for (auto &i : v)
				{
					std::cout << i << " ";

					boost::array<BYTE, DATA_SIZE> data;
					uint16_t datasize = fileData.GetPacketFromFile(i, data, fileData.GetFileSize());
					udpPacket.SetCommand(PacketCommand::SENDING);
					udpPacket.SetDataSize(datasize);
					udpPacket.SetData(data);
					udpPacket.SetFileDataIndex(i);

					socket.send_to(boost::asio::buffer(udpPacket.GetPacket()), receiver_endpoint);
					//Sleep(1);
				}
				Sleep(1);
				uint32_t nitens = fileData.GetFileSize() / DATA_SIZE;
				if (fileData.GetFileSize() % DATA_SIZE)
				{
					nitens++;
				}
				udpPacket.SetTotalFilePackets(nitens);
				udpPacket.SetCommand(PacketCommand::FINISHED_WAITING);
				socket.send_to(boost::asio::buffer(udpPacket.GetPacket()), receiver_endpoint);
			}
			break;
			case PacketCommand::FINISHED:
				std::cout << std::endl<<"Ok, File sent sucessefully. " << std::endl;
				std::cout << "Press esc to exit! " << std::endl;
				c = _getch();
				break;
			default:
				break;
			}
		}
		std::cout << "exited: " << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}