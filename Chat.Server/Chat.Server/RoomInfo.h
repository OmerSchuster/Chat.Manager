#pragma once
#include<iostream>
#include <algorithm>
#include <vector>
#include <initializer_list>

class RoomInfo
{
private:
	std::string room_name;
	std::vector<int> room_users;
	bool is_public;
public:
	RoomInfo() = default;
	RoomInfo(const std::string& name, const bool& isPublic) : room_name(name), is_public(isPublic) {}
	void AddUser(const int& userId)
	{
		bool itemExists = Contains(userId);

		if (!itemExists)
			room_users.push_back(userId);
	}
	void AddUsers(std::initializer_list<int> values) {
		room_users.insert(room_users.end(), values.begin(), values.end());
	}
	void RemoveUser(const int& userId)
	{
		bool itemExists = Contains(userId);

		if (itemExists)
			room_users.erase(std::remove(room_users.begin(), room_users.end(), userId), room_users.end());
	}
	const bool& Contains(const int& value) const
	{
		return std::find(room_users.begin(), room_users.end(), value) != room_users.end();
	}
	const std::string& GetRoomName() const
	{
		return room_name;
	}
	const std::vector<int>& GetUsersInRoomIds() const
	{
		return room_users;
	}
};