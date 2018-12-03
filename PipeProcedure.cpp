#include "stdafx.h"
#include "PipeProcedure.h"
#include "StimServer.h"
#include "StimServerDoc.h"

HANDLE hPipe;

UINT PipeProcedure( LPVOID pParam ) {
	
	extern CStimServerApp theApp;
	HRESULT hr = S_OK;
	short key;
	COMMANDBUFFER commandBuffer;
	DWORD messageLength;
	hr = CoInitializeEx(
		NULL,
		COINIT_MULTITHREADED);
	ASSERT(hr == S_OK);

	POSITION pos = theApp.GetFirstDocTemplatePosition();
	CDocTemplate* temp = theApp.GetNextDocTemplate(pos);
	pos = temp->GetFirstDocPosition();
	CStimServerDoc* pDoc = (CStimServerDoc*) temp->GetNextDoc(pos);

	hPipe = CreateNamedPipe(
		_T("\\\\.\\pipe\\StimServerPipe"),
		PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1,			// max. number of instances
		8,			// size of output buffer
		BUFFERSIZE,	// size of input buffer
		0,			// default time out 50ms
		NULL);
	ASSERT(hPipe != INVALID_HANDLE_VALUE);

	// Documentation of "CreateNamedPipe Function" states:
	// The pipe server should not perform a blocking read operation until the pipe client has started.
	// Otherwise, a race condition can occur. This typically occurs when initialization code, such as
	// the C run-time, needs to lock and examine inherited handles.
	while (true) {
		DWORD error = 0;
		VERIFY(ConnectNamedPipe(hPipe, NULL));
		TRACE("Client connected to Pipe.\n");
		while (true) {
//			TRACE("Waiting on Pipe.\n");
			if (error = !ReadFile(hPipe, &commandBuffer, BUFFERSIZE, &messageLength, NULL)) {
				error = GetLastError();
				switch (error) {
				case ERROR_BROKEN_PIPE:
					TRACE("The pipe has been ended.\n");
					break;
				default:
					TRACE("Error: %x\n", error);
					ASSERT(FALSE);
				}
			}
			if (error == ERROR_BROKEN_PIPE) break;
			ASSERT(error == 0);
//			TRACE("Message received. Length:%u; Key: %u\n", messageLength, commandBuffer.key);
			key = pDoc->Command(commandBuffer.key, &commandBuffer.body[0], messageLength-2);
			if (key != -1) {
				DWORD bytesWritten;
				VERIFY(WriteFile(hPipe, &key, 2, &bytesWritten, NULL));
				ASSERT(bytesWritten == 2);
			}
		}
		VERIFY(DisconnectNamedPipe(hPipe));	// neccessary for re-connect
	}
	CloseHandle(hPipe);
	TRACE("Destruct Pipe.\n");
	CoUninitialize();
	return 0;	// pretend success
}

void WritePipe(void* buffer, unsigned char bytesToWrite)
{
	DWORD bytesWritten;

	VERIFY(WriteFile(hPipe, buffer, bytesToWrite, &bytesWritten, NULL));
	ASSERT(bytesWritten == bytesToWrite);
}