#define BOOST_TEST_MODULE UdpFileTests
#include <boost/test/included/unit_test.hpp>
#include "UdpPacket.h"
//#include "FileData.h"

BOOST_AUTO_TEST_CASE(CommandTest)
{
  UdpPacket udp;
  udp.SetCommand(PacketCommand::INIT);
  BOOST_TEST(PacketCommand::INIT == udp.GetCommand());
}

BOOST_AUTO_TEST_CASE(DataTest)
{
	boost::array<BYTE, DATA_SIZE> d = {0};
	UdpPacket udp;
	udp.SetData(d);
	BOOST_TEST(d == udp.GetData());
}

BOOST_AUTO_TEST_CASE(DataSizeTest)
{
	UdpPacket udp;
	udp.SetDataSize(5);
	BOOST_TEST(5 == udp.GetDataSize());
}

BOOST_AUTO_TEST_CASE(FileDataIndexTest)
{
	UdpPacket udp;
	uint32_t i = 4;
	udp.SetFileDataIndex(i);
	BOOST_TEST(i == udp.GetFileDataIndex());
}

BOOST_AUTO_TEST_CASE(FileIdTest)
{
	UdpPacket udp;
	uint32_t i = 3;
	udp.SetFileId(i);
	BOOST_TEST(i == udp.GetFileId());
}

BOOST_AUTO_TEST_CASE(FileNameTest)
{
	UdpPacket udp;
	std::string fn = "test";
	udp.SetFileName(fn);
	BOOST_TEST(fn == udp.GetFileName());
}

BOOST_AUTO_TEST_CASE(FileNameLengthTest)
{
	UdpPacket udp;
	udp.SetFileNameLength(4);
	BOOST_TEST(4 == udp.GetFileNameLength());
}

BOOST_AUTO_TEST_CASE(FileSizeTest)
{
	UdpPacket udp;
	udp.SetFileSize(10);
	BOOST_TEST(10 == udp.GetFileSize());
}

BOOST_AUTO_TEST_CASE(MissingPacketsCountTest)
{
	UdpPacket udp;
	udp.SetMissingPacketsCount(10);
	BOOST_TEST(10 == udp.GetMissingPacketsCount());
}

BOOST_AUTO_TEST_CASE(TotalFilePacketsTest)
{
	UdpPacket udp;
	udp.SetTotalFilePackets(10);
	BOOST_TEST(10 == udp.GetTotalFilePackets());
}
