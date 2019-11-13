#include <tcpclient.h>
#include <iostream>
#include <thread>

using namespace std;

void startInputLoop(TCPClient* tcpClient)
{
    string input; cin >> input;
    while( input != "exit" ) {
        tcpClient->Send(input);
        cin >> input;
    }
}

int main()
{
    // Initialize socket.
    TCPClient* tcpClient = new TCPClient;

    thread inputThread(startInputLoop, tcpClient);

    // Start receiving from the host.
    tcpClient->onMessageReceived = [&](string message) {
        cout << "Message from the Server: " << message << endl;
    };

    // On socket closed:
    tcpClient->onSocketClosed = [&]() {
        cout << "Connection lost with the server!" << endl;
    };

    // Connect to the host.
    tcpClient->Connect("127.0.0.1", 8888, [&] {
        cout << "Connected to the server successfully." << endl;
        
        // Send String after the connection:
        tcpClient->Send("Hello Server!");
    },
    [&](int errorCode, string errorMessage) {
        cerr << errorMessage << endl;
    });

    tcpClient->Close();
    delete(tcpClient);

    return 0;
}
