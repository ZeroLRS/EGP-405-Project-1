#pragma once

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/BitStream.h"

#include <string>
#include <list>

enum GameMessages
{
	ID_CUSTOM_MESSAGE_START = ID_USER_PACKET_ENUM,

	ID_GAME_MESSAGE_1,

	ID_GAME_MESSAGE_GAMESTATE,
	ID_GAME_MESSAGE_USERNAME_REQUEST,
	ID_GAME_MESSAGE_CUP_SELECT,
	ID_GAME_MESSAGE_STEAL_SELECT,
	ID_GAME_MESSAGE_CUP_SELECT_PROMPT,
	ID_GAME_MESSAGE_STEAL_SELECT_PROMPT,
	ID_GAME_MESSAGE_WINNER_ANNOUNCE,
};

#pragma pack(push, 1)
// This is for data from the server, like the boardstate
struct GameMessageData
{
	unsigned char typeID;

	// Max total packet size incl. typeID is 1KB
	char message[1023];

};
#pragma pack (pop)

#pragma pack(push, 1)
// Smaller data from users, like cup selection
struct GameMessageFromUser
{
	unsigned char typeID;

	char playerName[16];
	char message[238];

};
#pragma pack (pop)

#pragma pack(push, 1)
// For when the user is first joining and requesting a name
struct GameMessageUsernameRequest
{
	unsigned char typeID;

	char playerName[16];
};
#pragma pack (pop)

struct userID
{
	std::string username;
	RakNet::RakNetGUID guid;
};

class MancalaNetworking
{

private:
	RakNet::RakPeerInterface *peer;
	RakNet::Packet *packet;
	bool isServer;
	std::string username;

	std::list<userID> users;

public:
	MancalaNetworking();
	~MancalaNetworking();
	void HandlePackets();
	void init();
	void deinit();

	//void broadcastBoardState(std::string boardstate);
	//void sendCupSelectPrompt(std::string player);
	//void sendBoardState(std::string player);
	//void sendCupSelect(int cup);
	//void sendStealSelect(int steal);
	//void broadcastWinnerMessage(std::string winnerName);
	//void broadcastTieMessage();
};

