#pragma once
#include <WinSock2.h>
#include<iostream>
#include <algorithm>

class MessageInfo
{
private:
	std::string message_content;
	int from_id;
	int to_id;
	int room_id;
public:
	MessageInfo() = default;
	MessageInfo(const std::string& content,
		const int& fromId,
		const int& toId,
		const int& roomId) :message_content(content), from_id(fromId), to_id(toId), room_id(roomId) {}
	MessageInfo(const std::string& content,
		const int& fromId,
		const int& roomId) :message_content(content), from_id(fromId), room_id(roomId) {}
	const std::string& GetMessageContent() const
	{
		return message_content;
	}
	std::string GetSenderName(const std::unordered_map<int, ClientInfo>& clients_map) const
	{
		auto it = clients_map.find(from_id);
		if (it != clients_map.end())
			return it->second.Name;

		return {};
	}
	std::string GetResponserName(const std::unordered_map<int, ClientInfo>& clients_map) const
	{
		auto it = clients_map.find(to_id);
		if (it != clients_map.end())
			return it->second.Name;

		return {};
	}
	std::string GetRoomName(const std::unordered_map<int, RoomInfo>& room_map) const
	{
		auto it = room_map.find(room_id);
		if (it != room_map.end())
			return it->second.GetRoomName();
		return {};
	}
};

