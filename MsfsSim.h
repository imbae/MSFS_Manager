#pragma once

#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <thread>
#include <mutex>
#include <future>

#include "SimConnect.h"
#include <functional>

using namespace std;

class MsfsSim
{
public:
	MsfsSim();
	~MsfsSim();

	static void DispatchProcSD(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);
	static void SimConnectDispatch();

	static void SimConnectException(DWORD id);

	void InitSimConnect();
	



	bool isAbort;
	HANDLE  hSimConnect;
	HRESULT hr;	
};

static MsfsSim* currentSimPtr;
