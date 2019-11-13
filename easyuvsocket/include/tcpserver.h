#ifndef FDR_TCPSERVERUV_H
#define FDR_TCPSERVERUV_H

#include <DllHelper.h>
#include <tcpclient.h>

class EASYUVSOCKET_API TCPServer
{
public:
    static TCPServer *instance;
    // Event Listeners:
    std::function<void(TCPClient *)> onNewConnection = [](TCPClient *) {};

    TCPServer(FDR_ON_ERROR);
    ~TCPServer();

    // Binding the server.
    void Bind(int port, FDR_ON_ERROR);
    void Bind(const char *address, uint16_t port, FDR_ON_ERROR);

    // Start listening the server.
    int Listen(FDR_ON_ERROR);

    // Close the server.
    void Close();

    sockaddr_in address;

private:
    static void uvConnectionCallback(uv_stream_t *server, int status);

    // LIBUV definitions:
    uv_loop_t *uvLoop;
    uv_tcp_t *uvTcpServer;
};

#endif
