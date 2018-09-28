#pragma once

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/BitStream.h"

#include <string>
#include <vector>

enum GameMessages
{
	ID_CUSTOM_MESSAGE_START = ID_USER_PACKET_ENUM,

	ID_GAME_MESSAGE_1,

	ID_GAME_MESSAGE_GAMESTATE,
	ID_GAME_MESSAGE_USERNAME_REQUEST,
	ID_GAME_MESSAGE_CUP_SELECT,
	ID_GAME_MESSAGE_CUP_SELECT_PROMPT,
	ID_GAME_MESSAGE_STEAL_SELECT_PROMPT,
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

public:
	MancalaNetworking();
	~MancalaNetworking();
	void HandlePackets();
	void init();
	void deinit();

	std::vector<userID> users;
	bool isServer;
	std::string username;

	static MancalaNetworking* network;

	// For mancala gamestate
	int cupSelect;

	void broadcastBoardState(std::string boardstate);
	void sendCupSelectPrompt(std::string player);
	void sendStealSelectPrompt(std::string player);
	void sendBoardState(std::string boardstate, std::string player);
	void sendCupSelect();
	void broadcastWinnerMessage(std::string winnerName);
	void broadcastTieMessage();
};

#include "MancalaNetworking.h"

MancalaNetworking::MancalaNetworking()
{
}

MancalaNetworking::~MancalaNetworking()
{
	deinit();
}

void MancalaNetworking::init()
{
	// Gamestate
	cupSelect = -1;

	// Networking
	const unsigned int bufferSz = 512;

	char str[bufferSz];

	// create and return instance of peer interface
	peer = RakNet::RakPeerInterface::GetInstance();

	// global peer settings for this app
	int isServer = 0;
	unsigned short serverPort = 60000;

	// ask user for peer type
	printf("(C)lient or (S)erver?\n");
	fgets(str, bufferSz, stdin);

	// start networking
	if ((str[0] == 'c') || (str[0] == 'C'))
	{
		RakNet::SocketDescriptor sd;
		peer->Startup(1, &sd, 1);


		printf("Enter username: (max 16 characters)\n");
		fgets(str, bufferSz, stdin);
		username = str;

		printf("Enter server IP or hit enter for 127.0.0.1\n");
		fgets(str, bufferSz, stdin);

		if (str[0] == '\n')
		{
			strcpy(str, "127.0.0.1");
		}

		printf("Starting the client.\n");
		peer->Connect(str, serverPort, 0, 0);
	}
	else
	{
		printf("Enter maximum number of clients: \n");
		fgets(str, bufferSz, stdin);

		unsigned int maxClients = atoi(str);
		RakNet::SocketDescriptor sd(serverPort, 0);
		peer->Startup(maxClients, &sd, 1);

		// We need to let the server accept incoming connections from the clients
		printf("Starting the server.\n");
		peer->SetMaximumIncomingConnections(maxClients);
		isServer = 1;
		username = "server";
	}

}

void MancalaNetworking::deinit()
{
	RakNet::RakPeerInterface::DestroyInstance(peer);
}

void MancalaNetworking::HandlePackets()
{
	for (
		packet = peer->Receive();
		packet;
		peer->DeallocatePacket(packet), packet = peer->Receive()
		)
	{
		switch (packet->data[0])
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			printf("Another client has disconnected.\n");
			break;
		case ID_REMOTE_CONNECTION_LOST:
			printf("Another client has lost the connection.\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			printf("Another client has connected.\n");
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			printf("Our connection request has been accepted.\n");
			{

				// Once the user has connected to the server, have them request a username
				GameMessageFromUser msg[1] = { ID_GAME_MESSAGE_USERNAME_REQUEST };
				strcpy(msg->playerName, username.c_str());

				peer->Send((char *)msg, sizeof(GameMessageFromUser), HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false);

			}
			break;
		case ID_GAME_MESSAGE_USERNAME_REQUEST:
		{

			GameMessageFromUser* usernameReq = (GameMessageFromUser*)packet->data;

			for (userID currID : users) {
				if (strcmp(usernameReq->playerName, currID.username.c_str()) == 0 || strcmp(usernameReq->playerName, username.c_str())) {
					peer->CloseConnection(packet->systemAddress, true);
					break;
				}
			}

			userID newUser;
			newUser.username = usernameReq->playerName;
			newUser.guid = packet->guid;

			users.push_back(newUser);

			printf("New user added");

		}
		break;
		case ID_NEW_INCOMING_CONNECTION:
			printf("A connection is incoming.\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			printf("The server is full.\n");
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			if (isServer) {
				printf("A client has disconnected.\n");
			}
			else {
				printf("We have been disconnected.\n");
			}
			break;
		case ID_CONNECTION_LOST:
			if (isServer) {
				printf("A client lost the connection.\n");
			}
			else {
				printf("Connection lost.\n");
			}
			break;

		case ID_GAME_MESSAGE_1:
			printf("DEBUG MESSAGE: received packet ID_GAME_MESSAGE_1.\n");
			{
				//	RakNet::RakString rs;
				//	RakNet::BitStream bsIn(packet->data, packet->length, false);
				//	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				//	bsIn.Read(rs);
				//	printf("%s\n", rs.C_String());

				// ****TO-DO: read packet without using bitstream

			}
			break;
		case ID_GAME_MESSAGE_GAMESTATE:
		{
			GameMessageData* msg = (GameMessageData*)packet->data;
			system("cls");
			printf("%s", msg->message);
		}
			break; 
		case ID_GAME_MESSAGE_CUP_SELECT:
		{
			GameMessageFromUser* msg = (GameMessageFromUser*)packet->data;
			printf("Selected: %i", msg->message[0]);
			cupSelect = msg->message[0];
		}
		break;
			case ID_GAME_MESSAGE_CUP_SELECT_PROMPT:
		{
			GameMessageData* msg = (GameMessageData*)packet->data;
			printf("%s", msg->message);
			sendCupSelect();
		}
			break;
		case ID_GAME_MESSAGE_STEAL_SELECT_PROMPT:
		{
			GameMessageData* msg = (GameMessageData*)packet->data;
			printf("%s", msg->message);
			sendCupSelect();
		}
			break;
		default:
			printf("Message with identifier %i has arrived.\n", packet->data[0]);
			break;
		}
	}
}

MancalaNetworking* MancalaNetworking::network;

void MancalaNetworking::broadcastBoardState(std::string boardstate)
{
	printf(boardstate.c_str());

	GameMessageData msg[1];
	msg->typeID = ID_GAME_MESSAGE_GAMESTATE;

	strcpy(msg->message, boardstate.c_str());

	peer->Send((char*)msg, sizeof(GameMessageData), HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_RAKNET_GUID, true);

}

void MancalaNetworking::sendBoardState(std::string boardstate, std::string playername)
{
	printf(boardstate.c_str());

	GameMessageData msg[1];
	msg->typeID = ID_GAME_MESSAGE_GAMESTATE;

	strcpy(msg->message, boardstate.c_str());

	RakNet::RakNetGUID destination;

	for (userID currID : users) {
		if (strcmp(playername.c_str(), currID.username.c_str()) == 0 ) {
			destination = currID.guid;
			break;
		}
	}

	peer->Send((char*)msg, sizeof(GameMessageData), HIGH_PRIORITY, RELIABLE_ORDERED, 0, destination, false);

}

void MancalaNetworking::broadcastWinnerMessage(std::string winnerName)
{
	std::string messageText = winnerName + " is the winner!";
	printf("%s", messageText.c_str());

	GameMessageData msg[1];
	msg->typeID = ID_GAME_MESSAGE_GAMESTATE;

	strcpy(msg->message, messageText.c_str());

	peer->Send((char*)msg, sizeof(GameMessageData), HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_RAKNET_GUID, true);

}

void MancalaNetworking::broadcastTieMessage()
{
	std::string messageText = "The game is a tie!";
	printf("%s", messageText.c_str());

	GameMessageData msg[1];
	msg->typeID = ID_GAME_MESSAGE_GAMESTATE;

	strcpy(msg->message, messageText.c_str());

	peer->Send((char*)msg, sizeof(GameMessageData), HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_RAKNET_GUID, true);

}

void MancalaNetworking::sendCupSelectPrompt(std::string player)
{
	std::string messageText = player + ", pick a cup with stones (1-6): ";
	printf("%s", messageText.c_str());

	GameMessageData msg[1];
	msg->typeID = ID_GAME_MESSAGE_CUP_SELECT_PROMPT;

	strcpy(msg->message, messageText.c_str());

	RakNet::RakNetGUID destination;

	for (userID currID : users) {
		if (strcmp(player.c_str(), currID.username.c_str()) == 0) {
			destination = currID.guid;
			break;
		}
	}

	peer->Send((char*)msg, sizeof(GameMessageData), HIGH_PRIORITY, RELIABLE_ORDERED, 0, destination, false);

}

void MancalaNetworking::sendStealSelectPrompt(std::string player)
{
	std::string messageText = player + ", you landed in empty cup, steal\? 0 = no, other = yes: ";
	printf("%s", messageText.c_str());

	GameMessageData msg[1];
	msg->typeID = ID_GAME_MESSAGE_STEAL_SELECT_PROMPT;

	strcpy(msg->message, messageText.c_str());

	RakNet::RakNetGUID destination;

	for (userID currID : users) {
		if (strcmp(player.c_str(), currID.username.c_str()) == 0) {
			destination = currID.guid;
			break;
		}
	}

	peer->Send((char*)msg, sizeof(GameMessageData), HIGH_PRIORITY, RELIABLE_ORDERED, 0, destination, false);
}

void MancalaNetworking::sendCupSelect()
{
	int cupIndex = getchar();
	while (getchar() != '\n');

	GameMessageData msg[1];
	msg->typeID = ID_GAME_MESSAGE_CUP_SELECT;

	msg->message[0] = cupIndex;

	peer->Send((char*)msg, sizeof(GameMessageData), HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
}