#include "pch.h"
#include "client_session.h"

#include <ws2tcpip.h>

int main()
{
    winsocklib::WinSockInitializer initializer;

    SOCKADDR_IN addr_in = { 0, };
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = ::htons(4000U);
    ::InetPton(AF_INET, L"127.0.0.1", &addr_in.sin_addr);

    std::shared_ptr<ClientSession> session = std::make_shared<ClientSession>();
    if (!session->Connect(addr_in)) {
        std::cout << "failed to connect" << std::endl;
        return 1;
    }

    // send
    session->SendLoginRequest("test_id", "test_password");

    // close, wait
    system("pause");
    session->Close();
}
