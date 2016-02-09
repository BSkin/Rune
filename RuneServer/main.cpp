#include "Server.h"

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

void initConsole()
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR pCmdLine,
	int nCmdShow)
{
	initConsole();
	HWND hWnd = GetConsoleWindow();

	Server server = Server(hWnd);
	server.run(pCmdLine);

	return 0;
}