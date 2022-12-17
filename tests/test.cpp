#include "pch.h"
#include "iocplib/acceptor.h"
#include "iocplib/socket_session.h"


class MockSocketSession : public iocplib::SocketSession
{
public:
	MockSocketSession() {}
	virtual ~MockSocketSession() {}

	virtual void OnReceivePacket(uint8_t* buffer, uint32_t received) {}
};


TEST(AcceptorTest, Acceptor_Open_성공_테스트) {
	// given
	auto acceptor = std::make_shared<iocplib::Acceptor<MockSocketSession> >();

	SOCKADDR_IN addr_in = { 0, };
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = ::htons(3000);
	addr_in.sin_addr.s_addr = ::htonl(INADDR_ANY);

	// when
	bool acceptor_open = acceptor->Open(1U, &addr_in);

	// then
	EXPECT_TRUE(acceptor_open);

	acceptor->Close();
}

TEST(AcceptorTest, Acceptor_Open_실패_테스트) {
	// given
	auto acceptor1 = std::make_shared<iocplib::Acceptor<MockSocketSession> >();
	auto acceptor2 = std::make_shared<iocplib::Acceptor<MockSocketSession> >();

	SOCKADDR_IN addr_in = { 0, };
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = ::htons(3000);
	addr_in.sin_addr.s_addr = ::htonl(INADDR_ANY);

	// when, then
	EXPECT_TRUE(acceptor1->Open(1U, &addr_in));
	
	EXPECT_THROW({
		acceptor2->Open(1U, &addr_in);
	}, WinSockException);

	acceptor1->Close();
	acceptor2->Close();
}
