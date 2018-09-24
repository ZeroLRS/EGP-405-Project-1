#pragma once

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/BitStream.h"

#include <string>

class MancalaNetworking
{

private:
	RakNet::RakPeerInterface *peer;
	RakNet::Packet *packet;
	bool isServer;

	enum GameMessages
	{
		ID_CUSTOM_MESSAGE_START = ID_USER_PACKET_ENUM,

		ID_GAME_MESSAGE_1,
	};


#pragma pack(push, 1)
	struct GameMessageData
	{
		unsigned char typeID;

		// ****TO-DO: implement game message data struct

	};
#pragma pack (pop)

public:
	MancalaNetworking();
	~MancalaNetworking();
	void HandlePackets();
	void init();
	void deinit();

	void broadcastBoardState(std::string boardstate);
	void sendCupSelectPrompt(std::string player);
	void sendBoardState(std::string player);
	void sendCupSelect(int cup);
	void sendStealSelect(int steal);
	void broadcastWinnerMessage(std::string winnerName);
	void broadcastTieMessage();
};

