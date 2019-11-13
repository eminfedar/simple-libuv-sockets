#include <tcpserver.h>
#include <iostream>

using namespace std;

int main()
{
    // Initialize server socket..
    TCPServer tcpServer;

    // When a new client connected:
    tcpServer.onNewConnection = [&](TCPClient *newClient) {
        // Print the client info:
        cout << "New client: [" << newClient->remoteAddress() << ":" << newClient->remotePort() << "]" << endl;
        newClient->Send("Hello client!");

        // When received a message from the client:
        newClient->onMessageReceived = [&, newClient](string message) {
            cout << newClient->remoteAddress() << ":" << newClient->remotePort() << " => " << message << endl;
        };

        // When client's socket closed:
        newClient->onSocketClosed = [&, newClient] {
            cout << newClient->remoteAddress() << ":" << newClient->remotePort() << "'s connection closed normally." << endl;
        };
    };

    // Bind the server to a port.
    tcpServer.Bind(8888, [](int errorCode, string errorMessage) {
        cerr << errorMessage << endl;
    });

    // Start Listening the server.
    tcpServer.Listen([](int errorCode, string errorMessage) {
        cerr << errorMessage << endl;
    });


    return 0;
}
