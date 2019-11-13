#include <tcpserver.h>
#include <iostream>

TCPServer *TCPServer::instance = NULL;

TCPServer::TCPServer(std::function<void(int, std::string)> onError)
{
    int result;
    this->uvLoop = uv_default_loop();
    // Init TCP Server
    if ((result = uv_tcp_init(this->uvLoop, this->uvTcpServer)) != 0)
    {
        FDR_PRINT_ERROR(result, "Error while initializing tcp server: ");
        return;
    }

    TCPServer::instance = this;
}

TCPServer::~TCPServer()
{
    // Check this after.
    free(this->uvTcpServer);

    TCPServer::instance = NULL;
}

void TCPServer::Bind(int port, std::function<void(int, std::string)> onError)
{
    this->Bind("0.0.0.0", port, onError);
}

void TCPServer::Bind(const char *addressString, uint16_t port, std::function<void(int, std::string)> onError)
{
    int result;
    if ((result = uv_ip4_addr(addressString, port, &this->address)) != 0)
    {
        FDR_PRINT_ERROR(result, "Error while resolving the bind address:");
        return;
    }

    if ((result = uv_tcp_bind(this->uvTcpServer, (sockaddr *)&this->address, 0)) != 0)
    {
        FDR_PRINT_ERROR(result, "Error while binding the server:");
        return;
    }
}

int TCPServer::Listen(std::function<void(int, std::string)> onError)
{
    int result;
    if ((result = uv_listen((uv_stream_t *)this->uvTcpServer, 1, uvConnectionCallback)) != 0)
    {
        FDR_PRINT_ERROR(result, "Error while listening the server:");
        return result;
    }

    if ((result = uv_run(this->uvLoop, UV_RUN_DEFAULT)) != 0)
    {
        FDR_PRINT_ERROR(result, "Error while listening the server:");
        return result;
    }

    return result;
}

void TCPServer::Close()
{
    uv_stop(this->uvLoop);
    for (auto const &[key, val] : TCPClient::Clients)
    {
        TCPClient::Clients.at(key)->Close();
        TCPClient::Clients.erase(key);
    }
    uv_loop_close(this->uvLoop);
}

// PRIVATE:
void TCPServer::uvConnectionCallback(uv_stream_t *server, int status)
{
    if (status < 0)
    {
        std::cerr << uv_err_name(status) << ": " << uv_strerror(status) << std::endl;
        return;
    }

    TCPClient *newClient = new TCPClient(server->loop, true);

    int result;
    if ((result = uv_accept(server, (uv_stream_t *)newClient->uvInstance)) != 0)
    {
        std::cerr << "Couldn't accept connection: " << uv_err_name(result) << ": " << uv_strerror(result) << std::endl;
        uv_close((uv_handle_t *)newClient->uvInstance, NULL);
        delete newClient;
    }
    else
    {
        // Start client's reading with uv.
        newClient->_uvReadStart();

        // Save it's connection address info.
        int addressLength = sizeof(newClient->address);
        uv_tcp_getpeername(newClient->uvInstance, (sockaddr *)&newClient->address, &addressLength);

        // Add it to the clients list:
        TCPClient::Clients.insert(std::make_pair((uv_stream_t *)newClient->uvInstance, newClient));

        // Emit the event.
        TCPServer::instance->onNewConnection(newClient);
    }
}
