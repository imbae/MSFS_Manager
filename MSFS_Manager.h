#pragma once
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "SimConnect.h"

#pragma comment (lib, "ws2_32.lib")

using namespace std;

constexpr auto MAX_BUF_SIZE = 128;

constexpr auto PI = 3.14159265359;
constexpr auto RAD_TO_DEG = 180 / PI;
constexpr auto DEG_TO_RAD = PI / 180;

constexpr auto M_TO_FEET = 3.28084;
constexpr auto MS_KNOT = 1.94384;

constexpr auto SYNC_BYTE = 'G';
constexpr auto PACKET_HEADER_LENTH = 4;
constexpr auto PACKET_TAIL_LENTH = 2;


typedef struct
{
	/// <summary> Latitude of aircraft, North is positive, South negative (Radians) </summary>
	double Latitude;

	/// <summary> Longitude of aircraft, East is positive, West negative (Radians) </summary>
	double Longitude;

	/// <summary> Altitude of aircraft (feet) </summary>
	double Altitude;

	/// <summary> Pitch angle (Radians) </summary>
	double Pitch;

	/// <summary> Bank angle (Radians) </summary>
	double Bank;

	/// <summary> Heading relative to magnetic north (Radians) </summary>
	double Heading;

	/// <summary> True airspeed (Knots) </summary>
	double Airspeed;

}SimAircraftPositionMessage;

typedef struct
{
	/// <summary> </summary>
	double zBody_u;

	/// <summary> </summary>
	double xBody_v;

	/// <summary> </summary>
	double yBody_w;

	/// <summary> </summary>
	double zBody_P;

	/// <summary> </summary>
	double xBody_Q;

	/// <summary> </summary>
	double yBody_R;

}SimAircraftVelocityMessage;

typedef struct
{
	/// <summary> deg </summary>
	float Pitch;

	/// <summary> deg </summary>
	float Bank;

	/// <summary> deg </summary>
	float Heading;

}SimCameraMessage;

typedef struct
{
	/// <summary> 2 = Cockpit, 3 = External/Chase 4 = Drone, 5 = Fixed on Plane, 6 = Environment, 8 = Showcase, 9 = Drone Plane </summary>
	double State;

	/// <summary> This can be used to reset the cockpit camera when the CAMERA_STATE is set to 2 (Cockpit) </summary>
	double Reset;

	/// <summary> Sets the zoom/FOV modifier for the cockpit camera. </summary>
	double Zoom;

}SimSetCockpitCameraMessage;

union Payload
{
	BYTE Data[MAX_BUF_SIZE];

	SimAircraftPositionMessage AircraftPosition;
	SimAircraftVelocityMessage AircraftVelocity;
	SimCameraMessage Camera;
	SimSetCockpitCameraMessage CockpitCamera;
};

enum GROUP_ID {
	GROUP_PILS
};

enum INPUT_ID {
	INPUT_PILS
};

enum EVENT_ID {
	EVENT_SIM_START,
	EVENT_PILS_START,

	EVENT_CRASH
};

enum DATA_DEFINE_ID {
	DEFINE_INIT_POSITION,
	DEFINE_PLANE_POSITION,
	DEFINE_PLANE_VELOCITY,
	DEFINE_COCKPIT_CAMERA
};

enum DATA_REQUEST_ID {
	SIM_AIRCRAFT_POSITION = 0,
	SIM_AIRCRAFT_VELOCITY,
	SIM_CAMERA,

	SIM_INIT_HOME_POSITION,
	SIM_SET_COCKPIT_CAMERA
};

enum MESSAGE_ID {
	REQUEST_PLANE_POSITION
};

class MSFS_Manager
{
public:
	MSFS_Manager();
	~MSFS_Manager();

	static void ReceivedFromClient();
	static void SendMessage();
	
	Payload payload;

	SOCKET in;
	sockaddr_in client;
	int clientLength;
	char rcvBuf[MAX_BUF_SIZE];
	char sendBuf[MAX_BUF_SIZE];
	BYTE downlinkBuf[MAX_BUF_SIZE];
	bool isUdpAbort;


	void InitSimConnect();
	static void DispatchProcSD(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);
	static void SimConnectDispatch();
	static void SendPlanePositionToSim(SimAircraftPositionMessage);
	static void SendPlaneVelocityToSim(SimAircraftVelocityMessage);
	static void SendCameraPositionToSim(SimCameraMessage);
	static void SendCockpitCameraToSim(SimSetCockpitCameraMessage);
	static void SimConnectException(DWORD id);
	HANDLE  hSimConnect;
	HRESULT hr;	
	bool isSimAbort;
};

static MSFS_Manager* currentPtr;