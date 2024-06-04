#pragma once
#include <WinSock2.h>
#include<iostream>
class ClientInfo
{
public:
	SOCKET ClientSocket;
	std::string Name;
public:
	ClientInfo() = default;
	ClientInfo(const std::string& name, const SOCKET& clientSocket) :Name(name), ClientSocket(clientSocket) {}
};
