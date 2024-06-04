#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include<unordered_map>
#include <set>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "ActiontTypeEnum.h"
#include <sstream>
#include <fstream>
#include "ClientInfo.h"
#include "RoomInfo.h"
#include "MessageInfo.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

const int PORT = 5001;
const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 1024;

std::unordered_map<int, ClientInfo> clients_map; // Mapping client ID to socket
std::unordered_map<int, MessageInfo> messages_map;
std::unordered_map<int, RoomInfo> rooms_map;
std::mutex clients_mutex;
static int client_counter = 1;
static int message_counter = 1;
static int room_counter = 0;

#define SAVE_DIRECTORY "E:\\dev\\ChatApp\\FileFromClient\\"

void SendMessageToClient(const std::string& message, SOCKET clientSocket)
{
	send(clientSocket, message.c_str(), message.size(), 0);
}

ActionType ParseCommand(std::string& command)
{
	if (command == "0") return Connect;
	if (command == "1") return Persons;
	if (command == "2") return MessageToPerson;
	if (command == "3") return RoomsGet;
	if (command == "4") return MessagesGet;
	if (command == "5") return RoomsAdd;
	if (command == "6") return MessageToRoom;
	if (command == "7") return AddPersonToRoom;
	if (command == "8") return LogoutUser;
	if (command == "9") return MessageWithPrivatePersonGet;
	if (command == "10") return InsertPrivateRoom;
	if (command == "11") return PrivateRoomGet;
	if (command == "12") return MessageWithFileToRoom;
	if (command == "13") return MessageWithImageToRoom;
	if (command == "14") return DownloadFile;
}

void HandleClient(SOCKET clientSocket, int clientId)
{
	char buffer[BUFFER_SIZE];
	while (true) {
		int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
		if (bytesReceived <= 0) {
			// Client disconnected
			closesocket(clientSocket);
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients_map.erase(clientId);
			std::cout << "Client disconnected: " << clientId << std::endl;
			break;
		}
		buffer[bytesReceived] = '\0';
		std::string message(buffer);
		std::cout << "Received message from client " << clientId << ": " << message << ", " << clientSocket << std::endl;

		std::string command = message.substr(0, message.find(','));
		std::string successAction = "0";
		ActionType parsedCommand = ParseCommand(command);

		switch (parsedCommand)
		{
		case Connect:
		{
			std::string connectPersonName = message.substr(message.find(',') + 1, message.length() - 1);
			clients_map[clientId] = ClientInfo(connectPersonName, clientSocket);
			successAction = "1";

			SendMessageToClient(successAction, clientSocket);
			break;
		}
		case Persons://need to fix the client 
		{
			std::ostringstream  allClientsNames;

			for (auto [k, v] : clients_map)
				allClientsNames << k << '#' << (v.Name) << ',';

			SendMessageToClient(allClientsNames.str(), clientSocket);
			break;
		}
		case MessageToPerson:
		{
			message_counter = messages_map.size();
			message_counter++;

			size_t firstCommaPos = message.find(',');
			size_t firstHashPos = message.find('#', firstCommaPos);
			size_t secondHashPos = message.find('#', firstHashPos + 1);
			size_t thirdHashPos = message.find('#', secondHashPos + 1);

			int fromId = std::stoi(message.substr(firstCommaPos + 1, firstHashPos - firstCommaPos - 1));
			std::string content = message.substr(firstHashPos + 1, secondHashPos - firstHashPos - 1);
			int toId = std::stoi(message.substr(secondHashPos + 1));
			int roomId = std::stoi(message.substr(thirdHashPos + 1));


			//send message to person
			//check if person still exists
			std::string answer;
			if (clients_map.find(toId) != clients_map.end())
			{
				SOCKET relevantSocket = clients_map[toId].ClientSocket;
				answer = clients_map[fromId].Name + '#' + content;
				messages_map[message_counter] = MessageInfo(content, fromId, toId, roomId);

				SendMessageToClient(answer, relevantSocket);
			}
			else
			{
				answer = "-1";
				SendMessageToClient(answer, clientSocket);
			}

			break;
		}
		case RoomsGet:
		{
			std::ostringstream allRooms;

			for (auto [k, v] : rooms_map)
			{
				std::ostringstream usersInRoomIds;
				for (auto val : v.GetUsersInRoomIds())
					usersInRoomIds << val << ',';

				allRooms << k << '#' << v.GetRoomName() << '#' << usersInRoomIds.str() << ',';
			}

			SendMessageToClient(allRooms.str(), clientSocket);
			break;
		}
		case MessagesGet:
		{
			std::ostringstream allMessages;

			for (auto [k, v] : messages_map)
			{
				allMessages << v.GetMessageContent() << '#' << v.GetResponserName(clients_map) << '#' << v.GetRoomName(rooms_map) << '#' << v.GetSenderName(clients_map) << ',';
			}

			SendMessageToClient(allMessages.str(), clientSocket);
			break;
		}

		case RoomsAdd:
		{
			room_counter = rooms_map.size();
			room_counter++;
			std::string roomName = message.substr(message.find(',') + 1, message.length() - 1);
			rooms_map[room_counter] = RoomInfo(roomName, true);
			successAction = "1";

			SendMessageToClient(successAction, clientSocket);
			break;
		}

		case MessageToRoom:
		{
			message_counter = messages_map.size();

			size_t commaSign = message.find(',');
			size_t firstHashSign = message.find('#', commaSign);
			size_t secHashSign = message.find('#', firstHashSign + 1);

			int roomId = std::stoi(message.substr(commaSign + 1, firstHashSign - commaSign - 1));
			std::string text = message.substr(firstHashSign + 1, secHashSign - firstHashSign - 1);
			int fromId = std::stoi(message.substr(secHashSign + 1, message.length() - 1));

			auto relevantUsers = rooms_map[roomId].GetUsersInRoomIds();

			for (int userId : relevantUsers)
			{
				if (clients_map.find(userId) != clients_map.end())
				{
					message_counter++;
					SOCKET relevantSocket = clients_map[userId].ClientSocket;
					messages_map[message_counter] = MessageInfo(text, fromId, userId, roomId);

					SendMessageToClient(text, relevantSocket);
				}
			}
			break;
		}

		case AddPersonToRoom:
		{
			size_t commaSign = message.find(',');
			size_t hashSign = message.find('#');

			int roomId = std::stoi(message.substr(commaSign + 1, hashSign - commaSign - 1));
			int clientId = std::stoi(message.substr(hashSign + 1, message.length() - 1));

			if (rooms_map.find(roomId) != rooms_map.end())
			{
				rooms_map[roomId].AddUser(clientId);
				successAction = "1";

				SendMessageToClient(successAction, clientSocket);
			}
			else
			{
				SendMessageToClient(successAction, clientSocket);
			}

			break;
		}
		case MessageWithFileToRoom:
		{
			size_t commaIndex = message.find(',');
			size_t hashIndex = message.find('#');
			std::string fileName = message.substr(commaIndex + 1, hashIndex - commaIndex - 1);
			int roomId = std::stoi(message.substr(hashIndex + 1, message.length() - 1));
			std::string fullPath = SAVE_DIRECTORY + fileName;

			std::ofstream outFile(fullPath, std::ios::binary);

			int bytesReceived;
			int totalBytesReceived = 0;
			std::string content = "new file";
			MessageInfo messageInfo(content, clientId, roomId, fileName, totalBytesReceived);

			while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0)
			{
				//flag finish transffering
				std::string stopStr(buffer);
				if (stopStr.find("!@#") <= stopStr.length())
					break;

				outFile.write(buffer, bytesReceived);
				totalBytesReceived += bytesReceived;
				messageInfo.SetFileBytes(totalBytesReceived);
			}

			if (bytesReceived == SOCKET_ERROR)
			{
				messageInfo.SetFileBytes(totalBytesReceived);
				std::cerr << "Error receiving data." << std::endl;
			}
			else
			{
				std::cout << "File received successfully: " << fullPath << std::endl;
				std::cout << "Total bytes received: " << totalBytesReceived << std::endl;
			}

			outFile.close();
			
			int messageCount = messages_map.size() + 1;
			messages_map[messageCount] = messageInfo;

			successAction = "1";
			
			SendMessageToClient(successAction, clientSocket);
			break;
		}
		case DownloadFile:
		{
			std::string fileName = message.substr(message.find(',') + 1, message.length() - 1);
			std::string fullPath = SAVE_DIRECTORY + fileName;
			
			std::ifstream file(fullPath, std::ios::in | std::ios::binary);

			if(!file.is_open())
			{
				std::cerr << "File not found" << std::endl;
				SendMessageToClient(successAction, clientSocket);
			}
			else
			{
				file.seekg(0, std::ios::end);
				std::streamsize fileSize = file.tellg();
				file.seekg(0, std::ios::beg);

				uint64_t fileSizeNetworkOrder = htonll(static_cast<uint64_t>(fileSize));
				send(clientSocket, reinterpret_cast<const char*>(&fileSizeNetworkOrder), sizeof(fileSizeNetworkOrder), 0);

				char fileBuffer[BUFFER_SIZE];
				while (file.read(fileBuffer,sizeof(fileBuffer)))
				{
					SendMessageToClient(fileBuffer, clientSocket);
				}

				if(file.gcount() > 0)
					SendMessageToClient(fileBuffer, clientSocket);

				file.close();
			}

			break;
		}
		default:
			std::cerr << "Unknown command" << std::endl;
			break;
		}
	}
}

void SetDefualtData(SOCKET clientSocket)
{
	//set some default connects removea later
	clients_map[2] = ClientInfo("tester-2", clientSocket + 1);
	clients_map[3] = ClientInfo("tester-3", clientSocket + 2);
	clients_map[4] = ClientInfo("tester-4", clientSocket + 3);

	rooms_map[1] = RoomInfo("Lobby", false);
	rooms_map[1].AddUsers({ 1,2,3 });
	rooms_map[2] = RoomInfo("Room1", false);
	rooms_map[2].AddUsers({ 4,7,18 });
	rooms_map[3] = RoomInfo("Room2", false);
	rooms_map[3].AddUsers({ 3,22,16 });

	messages_map[1] = MessageInfo("hello this is test message 1", 1, 2, 1);
	messages_map[2] = MessageInfo("hello this is test message 2", 2, 1, 2);
	messages_map[3] = MessageInfo("hello this is test message 3", 1, 2, 3);
}

void StartProcess()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed" << std::endl;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create socket" << std::endl;
		WSACleanup();
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(PORT);

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
	}

	if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR) {
		std::cerr << "Listen failed" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
	}

	std::cout << "Server listening on port " << PORT << std::endl;

	SetDefualtData(320);

	while (true) {
		sockaddr_in clientAddr;
		int clientAddrSize = sizeof(clientAddr);
		SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "Failed to accept client" << std::endl;
			continue;
		}

		std::lock_guard<std::mutex> lock(clients_mutex);
		int clientId = client_counter++;

		std::cout << "Client connected: " << clientId << std::endl;

		std::thread clientThread(HandleClient, clientSocket, clientId);
		clientThread.detach();
	}

	closesocket(serverSocket);
	WSACleanup();
}
int main()
{
	StartProcess();
}
