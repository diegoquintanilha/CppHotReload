#include <stdio.h>
#include <Windows.h>

#include "ReloadableFunction.h"

int main()
{
	while (true)
	{
		// Clear the console window
		system("cls");

		// Call the function
		int callCount = DrawAscii(50, 20);
		printf("Function call count: %d\n", callCount);

		// Quit if 'Q' is pressed
		printf("\nPress 'Q' to exit\n");
		if (GetAsyncKeyState('Q') & 0x8000)
			break;

		// Update window at every 500 milliseconds
		Sleep(500);
	}
	
	system("pause");
	return 0;
}

