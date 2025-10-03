#include <stdio.h>

#include "ReloadableFunction.h"

IMPL_RELOADABLE_FUNCTION(int, DrawAscii, int width, int height)
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			/*
				Feel free to comment, uncomment and modify any of
				the following blocks of code while the program is running.

				Recompile with:
					g++ -DHOT_RELOAD -shared ReloadableFunction.cpp -o DrawAscii.dll

				The function will automatically be hot reloaded.
			*/

			// Get coordinates from the center of the drawable area
			int cX = x - (width / 2);
			int cY = y - (height / 2);

			// Draw a circle
			if ((cX * cX + cY * cY) < 27)
				printf("@");

			// Draw a rhombus
//			int absX = cX > 0 ? cX : -cX;
//			int absY = cY > 0 ? cY : -cY;
//			if ((absX + absY) < 6)
//				printf("@");

			// Border
			else if ((x == 0) || (y == 0) || (x == width - 1) || (y == height - 1))
				printf("#");

			// Empty space
			else
				printf(" ");
		}
		printf("\n");
	}

	// Return the number of times that this function has been called
	// This counter resets when the function is reloaded
	static int counter = 0;
	counter++;
	return counter;
}

