/*******************************************************************************************
*
*   raylib [core] example - Basic 3d example
*
*   Welcome to raylib!
*
*   To compile example, just press F5.
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2020 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

enum GAME_STATE
{
	PLAY,
	WIN,
	LOSE,
};

typedef struct Tile
{
	bool isFlagged;
	bool isMine;
	bool isRevealed;
	int neighbouringMines;
} Tile;

typedef struct MineField
{
	int gameState;
	int tileSize;
	int boardWidth;
	int boardHeight;
	int mineCount;
	int revealedCount;
	Tile* tiles;

} MineField;


void ConstructMineField(MineField* minefield);
void FreeMineField(MineField* minefield);

Tile* GetTile(const MineField* minefield, int column, int row);
bool IsValidTile(const MineField* minefield, int column, int row);

void ProcessClick(MineField* minefield, Vector2 mousePosition);
void ClickTile(MineField* minefield, int column, int row);

void ProcessRightClick(const MineField* minefield, Vector2 mousePosition);
void RightClickTile(const MineField* minefield, int column, int row);


void DrawMineField(const MineField* minefield);

int main(int argc, char** argv) 
{
	const int tileSize = 25;
    // Initialization
    //--------------------------------------------------------------------------------------
	int gridWidth = 20;
	int gridHeight = 20;
	int mineCount = 40;

	if (argc >= 4)
	{
		gridWidth = TextToInteger(argv[1]);
		gridHeight = TextToInteger(argv[2]);
		mineCount = TextToInteger(argv[3]);
	}

    int screenWidth = gridWidth * tileSize;
    int screenHeight = gridHeight * tileSize;

	
    InitWindow(screenWidth, screenHeight, "raylib");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

	MineField minefield;
	minefield.gameState = PLAY;
	minefield.boardWidth = gridWidth;
	minefield.boardHeight = gridHeight;
	minefield.tileSize = tileSize;
	minefield.mineCount = mineCount;
	minefield.revealedCount = 0;
	ConstructMineField(&minefield);

	Vector2 mousePosition;
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
		
        // Update
        //----------------------------------------------------------------------------------
		if (minefield.gameState == PLAY)
		{
			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
			{
				mousePosition = GetMousePosition();
				ProcessClick(&minefield, mousePosition);
			}

			if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
			{
				mousePosition = GetMousePosition();
				ProcessRightClick(&minefield, mousePosition);
			}
		}
		//----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
			DrawMineField(&minefield);
			//DrawText(TextFormat("%d", minefield.boardHeight * minefield.boardWidth - minefield.revealedCount), 0, 0, 25, BLACK);
			if (minefield.gameState != PLAY)
			{
				char* gameOverMessage;
				if (minefield.gameState == LOSE)
					gameOverMessage = "Game Over!";
				else
					gameOverMessage = "You Win!";
				int textWidth = MeasureText(gameOverMessage, 32);
				int textHeight = MeasureText("M", 32);

				DrawText(gameOverMessage, screenWidth / 2 - textWidth / 2, screenHeight / 2 - textHeight, 32, BLACK);
			}

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
	FreeMineField(&minefield);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void ConstructMineField(MineField* minefield)
{
	minefield->tiles = MemAlloc(sizeof(Tile) * minefield->boardWidth * minefield->boardHeight);

	for (int i = 0; i < minefield->boardWidth; i++)
	{
		for (int j = 0; j < minefield->boardHeight; j++)
		{
			Tile* tile = GetTile(minefield, i, j);
			tile->isFlagged = false;
			tile->isMine = false;
			tile->isRevealed = false;
			tile->neighbouringMines = -1;
		}
	}

	int placedMineCount = 0;
	while (placedMineCount < minefield->mineCount)
	{
		int column = GetRandomValue(0, minefield->boardWidth - 1);
		int row = GetRandomValue(0, minefield->boardHeight - 1);
		
		while (GetTile(minefield, column, row)->isMine)
		{
			column = GetRandomValue(0, minefield->boardWidth - 1);
			row = GetRandomValue(0, minefield->boardHeight - 1);
		}

		GetTile(minefield, column, row)->isMine = true;
		placedMineCount += 1;
		
	}

}

void FreeMineField(MineField* minefield)
{
	MemFree(minefield->tiles);
}

Tile* GetTile(const MineField* minefield, int column, int row)
{
	return &(minefield->tiles[row * minefield->boardWidth + column]);
}

bool IsValidTile(const MineField* minefield, int column, int row)
{
	/*
	if (column >= 0 || column < minefield->boardWidth)
	{
		return true;
	}
	else if (row >= 0 || row < minefield->boardHeight)
	{
		return true;
	}
	else
	{
		return false;
	}
	*/
	return (column >= 0 && column < minefield->boardWidth) && (row >= 0 && row < minefield->boardHeight);
}


void ProcessClick(MineField* minefield, Vector2 mousePosition)
{
	int clickX = ((int)mousePosition.x) / minefield->tileSize;
	int clickY = ((int)mousePosition.y) / minefield->tileSize;

	ClickTile(minefield, clickX, clickY);
}

void ClickTile(MineField* minefield, int column, int row)
{
	Tile* tile = GetTile(minefield, column, row);

	
	if (tile->isFlagged || tile -> isRevealed)
	{
		return;
	}
	else if (tile->isMine)
	{
		tile->isRevealed = true;
		minefield->gameState = LOSE;
		return;
	}

	// Calculate neigbouring mines
	int neightbouringMines = 0;
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			if (j == 0 && i == 0)
				continue;
			
			if (!IsValidTile(minefield, column + i, row + j))
				continue;

			Tile* neigbour = GetTile(minefield, column + i, row + j);
			if (neigbour->isMine)
			{
				neightbouringMines += 1;
			}
		}
	}

	tile->neighbouringMines = neightbouringMines;
	tile->isRevealed = true;
	minefield->revealedCount += 1;

	if (neightbouringMines == 0)
	{
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				if (j == 0 && i == 0)
					continue;
				
				if (!IsValidTile(minefield, column + i, row + j))
					continue;

				Tile* neigbour = GetTile(minefield, column + i, row + j);
				if (!neigbour->isRevealed && !neigbour->isFlagged)
				{
					ClickTile(minefield, column + i, row + j);
				}

			}
		}
	}

	if (minefield->boardHeight * minefield->boardWidth - minefield->revealedCount == minefield->mineCount)
		minefield->gameState = WIN;

}

void ProcessRightClick(const MineField* minefield, Vector2 mousePosition)
{
	int clickX = ((int)mousePosition.x) / minefield->tileSize;
	int clickY = ((int)mousePosition.y) / minefield->tileSize;

	RightClickTile(minefield, clickX, clickY);
}

void RightClickTile(const MineField* minefield, int column, int row)
{
	Tile* tile = GetTile(minefield, column, row);
	if (tile->isRevealed)
	{
		return;
	}

	tile->isFlagged = !tile->isFlagged;
}

void DrawMineField(const MineField* minefield)
{
	for (int i = 0; i < minefield->boardWidth; i++)
	{
		for (int j = 0; j < minefield->boardHeight; j++)
		{
			Tile* tile = GetTile(minefield, i, j);
			
			Color tileBackground;

			if (tile->isFlagged)
			{
				tileBackground = RED;
			}
			else if (tile->isMine)
			{
				if (tile->isRevealed)
					tileBackground = BLACK;
				else
					tileBackground = (i + j) % 2 == 0 ? DARKGRAY : GRAY;
			}
			else
			{
				if (tile->isRevealed)
					tileBackground = (i + j) % 2 == 0 ? LIGHTGRAY : WHITE;
				else
					tileBackground = (i + j) % 2 == 0 ? DARKGRAY : GRAY;
			}

			DrawRectangle(i * 25, j * 25, 25, 25, tileBackground);

			if (tile->isRevealed && !tile->isMine)
			{
				// Show Digit
				if (tile->neighbouringMines != 0)
					DrawText(TextFormat("%d", tile->neighbouringMines), i * 25, j * 25, 25, BLACK);
			}
		}
	}
}