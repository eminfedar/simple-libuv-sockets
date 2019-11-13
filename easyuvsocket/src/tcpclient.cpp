#include <tcpclient.h>
#include <iostream>
#include <string>
#include <thread>
#include <uv.h>

std::map<uv_stream_t *, TCPClient *> TCPClient::Clients = {};

TCPClient::TCPClient(uv_loop_t *loop, bool isSlave) : uvLoop(loop), isSlave(isSlave)
{
    this->uvInstance = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(this->uvLoop, this->uvInstance);

    this->uvConnect = (uv_connect_t *)malloc(sizeof(uv_connect_t));
}
TCPClient::~TCPClient()
{
    free(this->uvInstance);
    free(this->uvConnect);
}

// Sending bytes:
void TCPClient::Send(std::string message)
{
    this->Send(message.c_str(), message.length());
}

void TCPClient::Send(const char *bytes, size_t byteslength)
{
    char buffer[byteslength];
    uv_buf_t buf = uv_buf_init(buffer, byteslength);
    buf.len = byteslength;
    buf.base = (char *)bytes;

    uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    write_req->data = (void *)buf.base;
    uv_write(write_req, (uv_stream_t *)this->uvInstance, &buf, 1, uvWriteCallback);
}

// Connecting to the server:
void TCPClient::Connect(std::string ipv4, uint16_t port, std::function<void()> onConnect, std::function<void(int, std::string)> onError)
{
    // Set onConnected event:
    this->onConnected = onConnect;
    this->onConnectError = onError;

    int result;
    if ((result = uv_ip4_addr(ipv4.c_str(), port, &this->address)) != 0)
    {
        FDR_PRINT_ERROR(result, "Error while resolving the address: ");
        TCPClient::Clients.erase((uv_stream_t *)this->uvInstance);
        return;
    }

    // Add the client to check after the connection:
    TCPClient::Clients.insert(std::make_pair((uv_stream_t *)this->uvInstance, this));

    if ((result = uv_tcp_connect(this->uvConnect, this->uvInstance, (const sockaddr *)&this->address, uvConnectCallback)) != 0)
    {
        FDR_PRINT_ERROR(result, "Error while connecting to the server: ");
        TCPClient::Clients.erase((uv_stream_t *)this->uvInstance);
        return;
    }

    if ((result = uv_run(this->uvLoop, UV_RUN_DEFAULT)) != 0)
    {
        FDR_PRINT_ERROR(result, "Error while looping the server: ");
        return;
    }
}

void TCPClient::_uvReadStart()
{
    uv_read_start((uv_stream_t *)this->uvInstance, this->uvAllocationCallback, this->uvReadCallback);
}

void TCPClient::Close()
{
    uv_read_stop((uv_stream_t *)this->uvInstance);
    uv_close((uv_handle_t*)this->uvInstance, [](uv_handle_t* handle){
        std::cout << "CLOSED" << std::endl;
    });

    if(!isSlave) {
        uv_stop(this->uvLoop);
        uv_loop_close(this->uvLoop);
    }

    // Emit the event:
    this->onSocketClosed();

    // delete this;
}

std::string TCPClient::remoteAddress()
{
    char name[60];
    uv_ip4_name(&this->address, name, 60);

    return std::string(name);
}
int TCPClient::remotePort()
{
    return ntohs(this->address.sin_port);
}

// ---------- PRIVATE:
void TCPClient::uvAllocationCallback(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
    FDR_UNUSED(handle);

    *buf = uv_buf_init((char *)malloc(size), size);
}

void TCPClient::uvReadCallback(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    // Close the socket request:
    if (nread == UV_EOF)
    {
        uv_close((uv_handle_t *)client, NULL);

        TCPClient::Clients.at(client)->Close();
        TCPClient::Clients.erase(client);
    }
    else if (nread > 0)
    {
        if (TCPClient::Clients.at(client) != NULL) {
            std::string msg(buf->base, buf->base + nread);
            TCPClient::Clients.at(client)->onMessageReceived(msg);
        }
    }
}
void TCPClient::uvWriteCallback(uv_write_t *req, int status)
{
    if (status < 0)
    {
        std::cerr << "Error while sending packets:\n - " << uv_err_name(status) << ": " << uv_strerror(status) << std::endl;
    }

    free(req);
}
void TCPClient::uvConnectCallback(uv_connect_t *req, int status)
{
    if (status < 0)
    {
        std::string errmsg = std::string("Error while connecting to the server: ") +
            "\n - " + uv_err_name(status) + std::string(": ") + uv_strerror(status);
        
        TCPClient::Clients.at((uv_stream_t *)(req->handle))->onConnectError(status, errmsg);
        TCPClient::Clients.erase((uv_stream_t *)req->handle);
        return;
    }

    TCPClient::Clients.at((uv_stream_t *)req->handle)->_uvReadStart();
    TCPClient::Clients.at((uv_stream_t *)req->handle)->onConnected();
}