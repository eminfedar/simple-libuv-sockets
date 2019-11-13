#ifndef FDR_TCPCLIENTUV_H
#define FDR_TCPCLIENTUV_H

#include <DllHelper.h>

// Empty error handling function:
#define FDR_UNUSED(expr){ (void)(expr); } 
#define FDR_ON_ERROR std::function<void(int, std::string)> onError = [](int errorCode, std::string errorMessage){FDR_UNUSED(errorCode); FDR_UNUSED(errorMessage)}
#define FDR_PRINT_ERROR(result, msg) { onError(result, std::string(msg) + "\n - " + uv_err_name(result) + std::string(": ") + uv_strerror(result)); }

#include <string>
#include <functional>
#include <uv.h>
#include <map>

class EASYUVSOCKET_API TCPClient
{
public:
    // Event Listeners:
    std::function<void(std::string)> onMessageReceived;
    std::function<void()> onSocketClosed = [](){};
    std::function<void()> onConnected = [](){};

    // LIBUV:
    uv_tcp_t* uvInstance;
    uv_connect_t* uvConnect;
    uv_loop_t* uvLoop;

    TCPClient(uv_loop_t* loop = uv_default_loop(), bool isSlave = false);
    ~TCPClient();

    void Send(std::string message);
    void Send(const char *bytes, size_t byteslength);

    void Connect(std::string ipv4, uint16_t port, std::function<void()> onConnected, FDR_ON_ERROR);

    sockaddr_in address;
    std::string remoteAddress();
    int remotePort();

    // Please do not use this.
    void _uvReadStart();

    void Close();

    static std::map<uv_stream_t*, TCPClient*> Clients;

private:
    static void uvAllocationCallback(uv_handle_t *handle, size_t size, uv_buf_t *buf);
    static void uvReadCallback(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
    static void uvWriteCallback(uv_write_t *req, int status);
    static void uvConnectCallback(uv_connect_t* req, int status);

    std::function<void(int, std::string)> onConnectError = [](int, std::string){};

    bool isSlave = false;
};

#endif
