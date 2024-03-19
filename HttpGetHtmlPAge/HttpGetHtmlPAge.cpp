#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <string>
#include <ctime>
using namespace std;

string unixTimeToString(int unixTime) {
    time_t t = unixTime;
    struct tm tmStruct;
    localtime_s(&tmStruct, &t);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &tmStruct);
    return string(buffer);
}

int main()
{
    setlocale(0, "ru");

    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }

    addrinfo* result = NULL;
    addrinfo* ptr = NULL;
    addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo("api.openweathermap.org", "80", &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << endl;
        WSACleanup();
        return 2;
    }

    SOCKET connectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connectSocket == INVALID_SOCKET) {
        cout << "socket failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 3;
    }

    int iResult = connect(connectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "connect failed with error: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        connectSocket = INVALID_SOCKET;
        WSACleanup();
        return 4;
    }

    string request = "GET /data/2.5/weather?q=Odessa&appid=75f6e64d49db78658d09cb5ab201e483&units=metric HTTP/1.1\r\n";
    request += "Host: api.openweathermap.org\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";

    int iResult = send(connectSocket, request.c_str(), request.length(), 0);
    if (iResult == SOCKET_ERROR) {
        cout << "send failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 5;
    }

    string response;
    char resBuf[1024];
    int respLength;

    do {
        respLength = recv(connectSocket, resBuf, sizeof(resBuf), 0);
        if (respLength > 0) {
            response.append(resBuf, respLength);
        }
        else if (respLength == 0) {
            cout << "Connection closed" << endl;
        }
        else {
            cout << "recv failed: " << WSAGetLastError() << endl;
        }
    } while (respLength > 0);

    cout << "Response: " << response << endl << endl;

    size_t pos = response.find("\"name\"");
    if (pos != string::npos) {
        pos = response.find(":", pos);
        size_t endPos = response.find(",", pos);
        if (endPos != string::npos) {
            string cityName = response.substr(pos + 3, endPos - pos - 4);
            cout << "City: " << cityName << endl;
        }
    }

    closesocket(connectSocket);
    WSACleanup();

    return 0;
}
