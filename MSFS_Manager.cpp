#include "MSFS_Manager.h"
#include "crc16.h"

MSFS_Manager::MSFS_Manager()
{
	currentPtr = this;
	isUdpAbort = false;
	isSimAbort = false;
	hSimConnect = NULL;
	hr = NULL;
}
MSFS_Manager::~MSFS_Manager()
{

}

void MSFS_Manager::ReceivedFromClient()
{
	//Startup Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsok = WSAStartup(version, &data);
	if (wsok != 0) {
		cout << "Can't start Winsock!" << wsok;
	}

	// Bind socket to ip address and port
	currentPtr->in = socket(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN serverHint{};
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(7755);

	if (bind(currentPtr->in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		cout << "Can't bind socket!" << WSAGetLastError() << endl;
		return;
	}

	int clientLength = sizeof(currentPtr->client);
	ZeroMemory(&currentPtr->client, sizeof(currentPtr->client));

	// Enter a loop
	while (!currentPtr->isUdpAbort)
	{
		ZeroMemory(currentPtr->rcvBuf, MAX_BUF_SIZE);

		// Wait for message
		int byesIn = recvfrom(currentPtr->in, currentPtr->rcvBuf, MAX_BUF_SIZE, 0, (sockaddr*)&currentPtr->client, &clientLength);
		if (byesIn == SOCKET_ERROR) {
			cout << "Error receiving from client " << WSAGetLastError() << endl;
			continue;
		}

		ZeroMemory(currentPtr->downlinkBuf, MAX_BUF_SIZE);
		ZeroMemory(currentPtr->payload.Data, MAX_BUF_SIZE);

		int syncIndex = -1;
		int payloadLenth = 0;
		int messageID = -1;
		unsigned short crc16;
		unsigned short packetCrc16;

		for (int i = 0; i < byesIn; i++)
		{
			if (currentPtr->rcvBuf[i] == SYNC_BYTE && i + 3 < MAX_BUF_SIZE)
			{
				syncIndex = i;
				payloadLenth = currentPtr->rcvBuf[i + 1];
				messageID = currentPtr->rcvBuf[i + 3];
				break;
			}
		}

		if (syncIndex >= 0)
		{
			for (int i = 0; i < payloadLenth + PACKET_HEADER_LENTH + PACKET_TAIL_LENTH; i++)
			{
				currentPtr->downlinkBuf[i] = currentPtr->rcvBuf[syncIndex + i];
			}

			for (int i = 0; i < payloadLenth; i++)
			{
				currentPtr->payload.Data[i] = currentPtr->downlinkBuf[i + PACKET_HEADER_LENTH];
			}
		}

		packetCrc16 = (unsigned short)((currentPtr->downlinkBuf[payloadLenth + PACKET_HEADER_LENTH]) | currentPtr->downlinkBuf[payloadLenth + PACKET_HEADER_LENTH + 1] << 8);
		crc16 = crc16_ccitt(currentPtr->downlinkBuf, payloadLenth + PACKET_HEADER_LENTH);

		if (packetCrc16 == crc16)
		{
			switch (messageID)
			{
			case 0:
				SendPlanePositionToSim(currentPtr->payload.PlanePos);
				break;
			case 1:
				break;
			default:
				break;
			}
		}

		// Display message and client info
		//cout << "Message rect from " << currentPtr->rcvBuf << endl;
	}

	// close socket
	closesocket(currentPtr->in);

	// Shutdown winsock
	WSACleanup();
}

void MSFS_Manager::SendMessage()
{
	int size = 0;	//temp

	//Send to client
	int sendSize = sendto(currentPtr->in, reinterpret_cast<char*>(currentPtr->sendBuf),
		size, 0, (struct sockaddr*)&currentPtr->client, sizeof(currentPtr->client));

	if (sendSize < 0)
	{
		cout << "Failed to send " << " :" << currentPtr->sendBuf << endl;
	}
}

void MSFS_Manager::InitSimConnect()
{
	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Set Data", NULL, 0, 0, 0)))
	{
		printf("Connected to Flight Simulator!\n");

		// Set up a data definition for positioning data
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_INIT_POSITION, "Initial Position", NULL, SIMCONNECT_DATATYPE_INITPOSITION);

		// Set up the data definition, but do not yet do anything with it
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_PLANE_POSITION, "PLANE LATITUDE", "radians");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_PLANE_POSITION, "PLANE LONGITUDE", "radians");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_PLANE_POSITION, "PLANE ALTITUDE", "feet");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_PLANE_POSITION, "PLANE PITCH DEGREES", "radians");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_PLANE_POSITION, "PLANE BANK DEGREES", "radians");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_PLANE_POSITION, "PLANE HEADING DEGREES MAGNETIC", "radians");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_PLANE_POSITION, "AIRSPEED TRUE", "knots");

		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_CAMERA_POSITION, "CAMERA GAMEPLAY PITCH YAW:0", "radians");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_CAMERA_POSITION, "CAMERA GAMEPLAY PITCH YAW:1", "radians");

		// Request a simulation start event
		hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart");
		hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_CRASH, "Crashed");

		// Create a custom event
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_PILS_START);

		// Link the custom event to some keyboard keys, and turn the input event off
		hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT_PILS, "p", EVENT_PILS_START);
		hr = SimConnect_SetInputGroupState(hSimConnect, INPUT_PILS, SIMCONNECT_STATE_ON);

		// Sign up for notifications for EVENT_PILS_START
		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_PILS, EVENT_PILS_START);
	}
}


void MSFS_Manager::SimConnectDispatch()
{
	while (!currentPtr->isSimAbort)
	{
		SimConnect_CallDispatch(currentPtr->hSimConnect, DispatchProcSD, NULL);
		Sleep(1);
	}

	currentPtr->hr = SimConnect_Close(currentPtr->hSimConnect);
}


void MSFS_Manager::DispatchProcSD(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
	HRESULT hr;

	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_NULL:
		printf("SIMCONNECT_RECV_ID_NULL\n");
		break;
	case SIMCONNECT_RECV_ID_EXCEPTION:
	{
		SIMCONNECT_RECV_EXCEPTION* ex = (SIMCONNECT_RECV_EXCEPTION*)pData;

		SimConnectException(ex->dwException);
		//printf("EX ID: %d, Index: %d\n", ex->dwSendID, ex->dwIndex);
		break;
	}
	case SIMCONNECT_RECV_ID_OPEN:
		printf("SIMCONNECT_RECV_ID_OPEN\n");
		break;
	case SIMCONNECT_RECV_ID_QUIT:
	{
		printf("SIMCONNECT_RECV_ID_QUIT\n");
		currentPtr->isSimAbort = true;
		break;
	}
	case SIMCONNECT_RECV_ID_EVENT:
	{
		SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;

		switch (evt->uEventID)
		{
		case EVENT_SIM_START:
		{
			// Now the sim is running, request information on the user aircraft			
			//hr = SimConnect_RequestDataOnSimObject(currentSimPtr->hSimConnect, REQUEST_POSITION, DEFINE_POSITION, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_ONCE);

			break;
		}
		case EVENT_PILS_START:
		{
			SIMCONNECT_DATA_INITPOSITION Init;
			Init.Altitude = 1050.0;
			Init.Latitude = 34.611049;
			Init.Longitude = 127.206115;
			/*Init.Latitude = 35.021410;
			Init.Longitude = 128.388953;*/
			Init.Pitch = 0.0;
			Init.Bank = 0.0;
			Init.Heading = 180.0;
			Init.OnGround = 0;
			Init.Airspeed = 60;
			hr = SimConnect_SetDataOnSimObject(currentPtr->hSimConnect, DEFINE_INIT_POSITION, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(Init), &Init);

			/*StructPlanePosition pos;
			pos.Altitude = 300 * M_TO_FEET;
			pos.Latitude = 35.264846 * DEG_TO_RAD;
			pos.Longitude = 128.122446 * DEG_TO_RAD;
			pos.Pitch = -15 * DEG_TO_RAD;
			pos.Bank = -20 * DEG_TO_RAD;
			pos.Heading = 60 * DEG_TO_RAD;
			pos.Airspeed = 50 * MS_KNOT;

			hr = SimConnect_SetDataOnSimObject(currentPtr->hSimConnect, DEFINE_PLANE_POSITION, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(pos), &pos);
			hr = SimConnect_RequestDataOnSimObject(currentPtr->hSimConnect, REQUEST_PLANE_POSITION, DEFINE_PLANE_POSITION, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_ONCE);*/

			printf("EVENT_PILS_START received and data sent\n");

			break;
		}
		case EVENT_CRASH:
			printf("Crashed! \n");
			break;
		default:
			break;
		}
		break;
	}
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
	{
		SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;

		switch (pObjData->dwRequestID)
		{
		case REQUEST_PLANE_POSITION:
		{
			DWORD ObjectID = pObjData->dwObjectID;
			StructPlanePosition* pS = (StructPlanePosition*)&pObjData->dwData;

			printf("Lat=%f  Lon=%f  Alt=%f\n", pS->Latitude, pS->Longitude, pS->Altitude);
			printf("Pitch=%f  Bank=%f  Heading=%f\n", pS->Pitch, pS->Bank, pS->Heading);
			printf("Air Speed=%f\n", pS->Airspeed);
			break;
		}
		default:
			break;
		}

		printf("SIMCONNECT_RECV_ID_SIMOBJECT_DATA\n");
		break;
	}
	default:
		printf("\nReceived:%d\n", pData->dwID);
		break;
	}
}


void MSFS_Manager::SendPlanePositionToSim(StructPlanePosition data)
{
	HRESULT hr;

	data.Pitch = -data.Pitch;
	data.Bank = -data.Bank;	

	hr = SimConnect_SetDataOnSimObject(currentPtr->hSimConnect, DEFINE_PLANE_POSITION, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(data), &data);
	//hr = SimConnect_RequestDataOnSimObject(currentPtr->hSimConnect, REQUEST_PLANE_POSITION, DEFINE_PLANE_POSITION, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_ONCE);
}







void MSFS_Manager::SimConnectException(DWORD id)
{
	switch (id)
	{
	case SIMCONNECT_EXCEPTION_NONE:
		printf("SIMCONNECT_EXCEPTION_NONE\n");
		break;
	case SIMCONNECT_EXCEPTION_ERROR:
		printf("SIMCONNECT_EXCEPTION_ERROR \n");
		break;
	case SIMCONNECT_EXCEPTION_SIZE_MISMATCH:
		printf("SIMCONNECT_EXCEPTION_SIZE_MISMATCH \n");
		break;
	case SIMCONNECT_EXCEPTION_UNRECOGNIZED_ID:
		printf("SIMCONNECT_EXCEPTION_UNRECOGNIZED_ID \n");
		break;
	case SIMCONNECT_EXCEPTION_UNOPENED:
		printf("SIMCONNECT_EXCEPTION_UNOPENED \n");
		break;
	case SIMCONNECT_EXCEPTION_VERSION_MISMATCH:
		printf("SIMCONNECT_EXCEPTION_VERSION_MISMATCH \n");
		break;
	case SIMCONNECT_EXCEPTION_TOO_MANY_GROUPS:
		printf("SIMCONNECT_EXCEPTION_TOO_MANY_GROUPS \n");
		break;
	case SIMCONNECT_EXCEPTION_NAME_UNRECOGNIZED:
		printf("SIMCONNECT_EXCEPTION_NAME_UNRECOGNIZED \n");
		break;
	case SIMCONNECT_EXCEPTION_TOO_MANY_EVENT_NAMES:
		printf("SIMCONNECT_EXCEPTION_TOO_MANY_EVENT_NAMES \n");
		break;
	case SIMCONNECT_EXCEPTION_EVENT_ID_DUPLICATE:
		printf("SIMCONNECT_EXCEPTION_EVENT_ID_DUPLICATE \n");
		break;
	case SIMCONNECT_EXCEPTION_TOO_MANY_MAPS:
		printf("SIMCONNECT_EXCEPTION_TOO_MANY_MAPS \n");
		break;
	case SIMCONNECT_EXCEPTION_TOO_MANY_OBJECTS:
		printf("SIMCONNECT_EXCEPTION_TOO_MANY_OBJECTS \n");
		break;
	case SIMCONNECT_EXCEPTION_TOO_MANY_REQUESTS:
		printf("SIMCONNECT_EXCEPTION_TOO_MANY_REQUESTS \n");
		break;
	case SIMCONNECT_EXCEPTION_WEATHER_INVALID_PORT:
		printf("SIMCONNECT_EXCEPTION_WEATHER_INVALID_PORT \n");
		break;
	case SIMCONNECT_EXCEPTION_WEATHER_INVALID_METAR:
		printf("SIMCONNECT_EXCEPTION_WEATHER_INVALID_METAR  \n");
		break;
	case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_GET_OBSERVATION:
		printf("SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_GET_OBSERVATION  \n");
		break;
	case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_CREATE_STATION:
		printf("SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_CREATE_STATION  \n");
		break;
	case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_REMOVE_STATION:
		printf("SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_REMOVE_STATION  \n");
		break;
	case SIMCONNECT_EXCEPTION_INVALID_DATA_TYPE:
		printf("SIMCONNECT_EXCEPTION_INVALID_DATA_TYPE  \n");
		break;
	case SIMCONNECT_EXCEPTION_INVALID_DATA_SIZE:
		printf("SIMCONNECT_EXCEPTION_INVALID_DATA_SIZE  \n");
		break;
	case SIMCONNECT_EXCEPTION_DATA_ERROR:
		printf("SIMCONNECT_EXCEPTION_DATA_ERROR  \n");
		break;
	case SIMCONNECT_EXCEPTION_INVALID_ARRAY:
		printf("SIMCONNECT_EXCEPTION_INVALID_ARRAY  \n");
		break;
	case SIMCONNECT_EXCEPTION_CREATE_OBJECT_FAILED:
		printf("SIMCONNECT_EXCEPTION_CREATE_OBJECT_FAILED  \n");
		break;
	case SIMCONNECT_EXCEPTION_LOAD_FLIGHTPLAN_FAILED:
		printf("SIMCONNECT_EXCEPTION_LOAD_FLIGHTPLAN_FAILED  \n");
		break;
	case SIMCONNECT_EXCEPTION_OPERATION_INVALID_FOR_OBJECT_TYPE:
		printf("SIMCONNECT_EXCEPTION_OPERATION_INVALID_FOR_OJBECT_TYPE   \n");
		break;
	case SIMCONNECT_EXCEPTION_ILLEGAL_OPERATION:
		printf("SIMCONNECT_EXCEPTION_ILLEGAL_OPERATION  \n");
		break;
	case SIMCONNECT_EXCEPTION_ALREADY_SUBSCRIBED:
		printf("SIMCONNECT_EXCEPTION_ALREADY_SUBSCRIBED  \n");
		break;
	case SIMCONNECT_EXCEPTION_INVALID_ENUM:
		printf("SIMCONNECT_EXCEPTION_INVALID_ENUM  \n");
		break;
	case SIMCONNECT_EXCEPTION_DEFINITION_ERROR:
		printf("SIMCONNECT_EXCEPTION_DEFINITION_ERROR   \n");
		break;
	case SIMCONNECT_EXCEPTION_DUPLICATE_ID:
		printf("SIMCONNECT_EXCEPTION_DUPLICATE_ID   \n");
		break;
	case SIMCONNECT_EXCEPTION_DATUM_ID:
		printf("SIMCONNECT_EXCEPTION_DATUM_ID   \n");
		break;
	case SIMCONNECT_EXCEPTION_OUT_OF_BOUNDS:
		printf("SIMCONNECT_EXCEPTION_OUT_OF_BOUNDS   \n");
		break;
	case SIMCONNECT_EXCEPTION_ALREADY_CREATED:
		printf("SIMCONNECT_EXCEPTION_ALREADY_CREATED   \n");
		break;
	case SIMCONNECT_EXCEPTION_OBJECT_OUTSIDE_REALITY_BUBBLE:
		printf("SIMCONNECT_EXCEPTION_OBJECT_OUTSIDE_REALITY_BUBBLE   \n");
		break;
	case SIMCONNECT_EXCEPTION_OBJECT_CONTAINER:
		printf("SIMCONNECT_EXCEPTION_OBJECT_CONTAINER    \n");
		break;
	case SIMCONNECT_EXCEPTION_OBJECT_AI:
		printf("SIMCONNECT_EXCEPTION_OBJECT_AI    \n");
		break;
	case SIMCONNECT_EXCEPTION_OBJECT_ATC:
		printf("SIMCONNECT_EXCEPTION_OBJECT_ATC    \n");
		break;
	case SIMCONNECT_EXCEPTION_OBJECT_SCHEDULE:
		printf("SIMCONNECT_EXCEPTION_OBJECT_SCHEDULE     \n");
		break;
	}
}


int main()
{
	MSFS_Manager* manager = new MSFS_Manager();

	manager->InitSimConnect();

	thread udpThread(manager->ReceivedFromClient);
	thread simThread(manager->SimConnectDispatch);

	udpThread.join();
	simThread.join();

	return 0;
}