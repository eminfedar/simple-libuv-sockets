# Asynchronous Sockets for C++ [libuv]
# NOT FULLY FINISHED
Simple libuv-based single-threaded asynchronous TCP & UDP client & server classes in C++.
```cpp
// Initialize a tcp socket.
TCPSocket tcpSocket;

// Connect to the host.
tcpSocket.Connect("127.0.0.1", 8888, [&] {
    cout << "Connected to the server successfully." << endl;

    // Send String:
    tcpSocket.Send("Hello Server!");
});
```
Super Easy!

**CPU & RAM Usages (with 1000 tcp connection & with single udp server + client):**
imagehere.

Lightweight!

## Examples:
imagehere
You can compile all the examples by just going in the `examples/` directory and run `make` in terminal:
- [examples/tcp-client.cpp](https://github.com/eminfedar/simple-libuv-sockets/blob/master/examples/tcp-client.cpp)
- [examples/tcp-server.cpp](https://github.com/eminfedar/simple-libuv-sockets/blob/master/examples/tcp-server.cpp)
- [examples/udp-client.cpp](https://github.com/eminfedar/simple-libuv-sockets/blob/master/examples/udp-client.cpp)
- [examples/udp-server.cpp](https://github.com/eminfedar/simple-libuv-sockets/blob/master/examples/udp-server.cpp)
