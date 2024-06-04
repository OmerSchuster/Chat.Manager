#pragma once
enum ActionType
{
    Connect = 0,
    Persons = 1,
    MessageToPerson = 2,
    RoomsGet = 3,
    MessagesGet = 4,
    RoomsAdd = 5,
    MessageToRoom = 6,
    AddPersonToRoom = 7,
    LogoutUser = 8,
    MessageWithPrivatePersonGet = 9,
    InsertPrivateRoom = 10,
    PrivateRoomGet = 11,
    MessageWithFileToRoom = 12,
    MessageWithImageToRoom = 13,
    DownloadFile = 14
};