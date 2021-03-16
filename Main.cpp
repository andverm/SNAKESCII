#include <algorithm>
#include <string>
#include <Windows.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <thread>
#include <vector>

// TODO 
// - Disable go through walls
// - Option to go back to menu
// - Level select

// BUGS:
// 

constexpr int SCREEN_WIDTH = 120;
constexpr int SCREEN_HEIGHT = 40;

enum Difficulty
{
	normal = 0,
	expert = 2
};

enum Direction
{
	up,
	right,
	down,
	left
};

enum Options
{
	enterName = 0,
	changeDiff = 1,
	startGame = 3,
	viewLeaderBoard = 4,
	quit = 5
};

struct PlayerData
{
	std::wstring mName;
	int mScore;
	int mDifficulty;

	PlayerData(const wchar_t* name = L"AAA", int score = 0, int difficulty = normal)
		: mName(name), mScore(score), mDifficulty(difficulty) {}
};

struct SnakeElement
{
	int mX;
	int mY;

	SnakeElement(int x, int y)
		: mX(x), mY(y) {}
};

void ClearScreen(wchar_t* screen)
{
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
		screen[i] = L' ';
}

void Render(wchar_t* screen, HANDLE console, DWORD bytesWritten)
{
	WriteConsoleOutputCharacter(console, screen,
		SCREEN_WIDTH * SCREEN_HEIGHT, { 0, 0 }, &bytesWritten);
}

int main()
{
	//Read Player Score Data
	std::wfstream playerData;
	playerData.open("leaderboard.txt");

	if (!playerData)
	{
		std::wcout << "ERROR: \"leaderboard.txt\" not found" << std::endl;
		std::cin.get();
		return 0;
	}

	std::vector<PlayerData> leaderBoard;
	while (playerData.good())
	{
		PlayerData pd;
		playerData >> pd.mName >> pd.mScore >> pd.mDifficulty;
		leaderBoard.emplace_back(pd);
	}

	playerData.close();

	// Create a screen buffer
	wchar_t* screen = new wchar_t[SCREEN_WIDTH * SCREEN_HEIGHT];
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
		screen[i] = L' ';
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_WRITE, 0,
		nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;

	bool game = true;
	bool menu = true;

	int difficulty = normal;
	wchar_t playerName[] = { 'A', 'A', 'A', 0 };
	int bestScore = 0;

	while (game)
	{
		// Menu
		int cursor = enterName;
		bool anyKeyPressed = true;
		bool nameInput = false;
		bool leaderBoardPanel = false;
		int nameCharacterPos = 0;

		std::vector<PlayerData> easyData;
		std::vector<PlayerData> hardData;

		for (auto& pd : leaderBoard)
		{
			if (pd.mDifficulty == normal)
			{
				easyData.emplace_back(pd);
				continue;
			}

			hardData.emplace_back(pd);
		}

		while (menu)
		{
			bool keyUp = (0x8000 & GetAsyncKeyState((unsigned char)'\x26')) != 0;
			bool keyDown = (0x8000 & GetAsyncKeyState((unsigned char)'\x28')) != 0;
			bool keyLeft = (0x8000 & GetAsyncKeyState((unsigned char)'\x25')) != 0;
			bool keyRight = (0x8000 & GetAsyncKeyState((unsigned char)'\x27')) != 0;
			bool enter = (0x8000 & GetAsyncKeyState((unsigned char)'\xD')) != 0
				|| (0x8000 & GetAsyncKeyState((unsigned char)'\x20')) != 0;

			if (nameInput)
			{
				wchar_t letter = playerName[nameCharacterPos];

				if (keyUp & !anyKeyPressed)
				{
					letter--;
					if (letter < 65)
						letter = 90;
				}

				if (keyDown & !anyKeyPressed)
				{
					letter++;
					if (letter > 90)
						letter = 65;
				}

				if (keyRight && !anyKeyPressed)
				{
					nameCharacterPos++;

					if (nameCharacterPos > 2)
						nameCharacterPos = 0;

					letter = playerName[nameCharacterPos];
				}

				if (keyLeft && !anyKeyPressed)
				{
					nameCharacterPos--;

					if (nameCharacterPos < 0)
						nameCharacterPos = 2;

					letter = playerName[nameCharacterPos];
				}

				playerName[nameCharacterPos] = letter;

				if (enter && !anyKeyPressed)
				{
					nameCharacterPos = 0;
					nameInput = false;
					anyKeyPressed = true;
				}
			}

			if (leaderBoardPanel)
			{
				if (enter && !anyKeyPressed)
				{
					leaderBoardPanel = false;
					anyKeyPressed = true;
				}
			}

			if (enter && !anyKeyPressed)
			{
				switch (cursor)
				{
				case enterName:
					nameInput = nameInput ? false : true;
					break;

				case changeDiff:
					difficulty = difficulty == normal ? expert : normal;
					break;

				case startGame:
					menu = false;
					break;

				case viewLeaderBoard:
					leaderBoardPanel = true;
					break;

				case quit:
					return 0;

				default:
					break;
				}
			}

			if (!nameInput && keyUp && !anyKeyPressed)
			{
				cursor--;

				if (cursor == 2)
					cursor--;

				if (cursor < enterName)
					cursor = quit;
			}

			if (!nameInput && keyDown && !anyKeyPressed)
			{
				cursor++;

				if (cursor == 2)
					cursor++;

				if (cursor > quit)
					cursor = enterName;
			}

			anyKeyPressed = keyUp || keyDown || keyLeft || keyRight || enter;

			int cursorPosition = (SCREEN_HEIGHT / 2 + cursor) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 10;
			if (nameInput)
				cursorPosition -= SCREEN_WIDTH - (8 + nameCharacterPos);

			const wchar_t* diffChar = difficulty == normal ? L"NORMAL" : L"NIGHTMARE";

			ClearScreen(screen);

			if (leaderBoardPanel)
			{
				wsprintf(&screen[SCREEN_WIDTH * 24 / 2 + SCREEN_WIDTH / 2 - 6], L"LEADERBOARD");
				wsprintf(&screen[SCREEN_WIDTH * 32 / 2 + SCREEN_WIDTH / 2 - 12], L"NORMAL");
				wsprintf(&screen[SCREEN_WIDTH * 32 / 2 + SCREEN_WIDTH / 2 + 4], L"NIGHTMARE");

				for (int i = 0; i < 10; i++)
				{
					int space = i < 9 ? 15 : 16;
					wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 - space], L"%d.", i + 1);
					screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1] = L'|';
					//screen[(i + 10) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 + 2] = L'X';

					if (i < easyData.size())
					{
						const wchar_t* easyName = easyData[i].mName.c_str();
						space = easyData[i].mScore > 9 ? 6 : 5;
						wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 - 11], easyName);
						wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 - space], L"%d",
							easyData[i].mScore);
					}

					if (i < hardData.size())
					{
						const wchar_t* hardName = hardData[i].mName.c_str();
						space = hardData[i].mScore > 9 ? 10 : 11;
						wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 + 5], hardName);
						wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 + space], L"%d",
							hardData[i].mScore);
					}
				}

				wsprintf(&screen[32 * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 4],
					L"+ BACK");
			}
			else
			{
				wsprintf(&screen[8], L"1. Adjust console WIDTH so that the [+] signs are in the top two corners.");
				wsprintf(&screen[SCREEN_WIDTH + 8], L"2. Adjust console HEIGHT so that bottom [+] signs are visible.");
				wsprintf(&screen[0], L"[+]");
				wsprintf(&screen[SCREEN_WIDTH - 3], L"[+]");
				wsprintf(&screen[SCREEN_WIDTH * SCREEN_HEIGHT - SCREEN_WIDTH], L"[+]");
				wsprintf(&screen[SCREEN_WIDTH * SCREEN_HEIGHT - 3], L"[+]");

				wsprintf(&screen[cursorPosition], L"+");
				wsprintf(&screen[(SCREEN_HEIGHT / 2 + enterName) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 2], playerName);
				wsprintf(&screen[(SCREEN_HEIGHT / 2 + changeDiff) * SCREEN_WIDTH + SCREEN_WIDTH / 2 + 4], diffChar);
				wsprintf(&screen[(SCREEN_HEIGHT / 2 - 3) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 10],
					L"WELCOME TO SNAKESCII");
				wsprintf(&screen[(SCREEN_HEIGHT / 2 + enterName) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 8], L"NAME:");
				wsprintf(&screen[(SCREEN_HEIGHT / 2 + changeDiff) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 8],
					L"DIFFICULTY:");
				wsprintf(&screen[(SCREEN_HEIGHT / 2 + startGame) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 8], L"START GAME");
				wsprintf(&screen[(SCREEN_HEIGHT / 2 + viewLeaderBoard) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 8],
					L"LEADERBOARD");
				wsprintf(&screen[(SCREEN_HEIGHT / 2 + quit) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 8], L"QUIT");
			}
			// Output to the console
			Render(screen, console, bytesWritten);
		}

		// Initialize Variables
		std::list<SnakeElement> snake = {
			{ 60, 15 },
			{ 62, 15 },
			{ 64, 15 },
			{ 66, 15 },
			{ 68, 15 },
			{ 70, 15 },
			{ 72, 15 },
		};

		int gameSpeed = 500 + difficulty * 100;
		const int speedUpPerFood = 25 + 25 * difficulty;
		const int growthPerFood = 1 + 1 * difficulty;

		int foodX = 30;
		int foodY = 15;
		int score = 0;

		int currentDirection = left;
		int newDirection = currentDirection;

		bool dead = false;
		bool playing = true;

		auto timeOne = std::chrono::system_clock::now();
		auto timeTwo = std::chrono::system_clock::now();
		float deltaTime = 0.0f;

		// Player Input Loop
		while (playing)
		{
			// Timing and input
			timeTwo = std::chrono::system_clock::now();
			std::chrono::duration<float> elapsed = timeTwo - timeOne;
			timeOne = timeTwo;

			deltaTime += elapsed.count();

			// Get Input
			bool keyUp = (0x8000 & GetAsyncKeyState((unsigned char)'\x26')) != 0;
			bool keyRight = (0x8000 & GetAsyncKeyState((unsigned char)'\x27')) != 0;
			bool keyDown = (0x8000 & GetAsyncKeyState((unsigned char)'\x28')) != 0;
			bool keyLeft = (0x8000 & GetAsyncKeyState((unsigned char)'\x25')) != 0;

			if (keyUp && currentDirection != down)
				newDirection = up;
			if (keyRight && currentDirection != left)
				newDirection = right;
			if (keyDown && currentDirection != up)
				newDirection = down;
			if (keyLeft && currentDirection != right)
				newDirection = left;

			// Game Logic

			// Update Position Loop (slower than input loop)
			if (!dead && deltaTime >= 1 / (float)gameSpeed * 100)
			{
				deltaTime = 0.0f;

				int currentPositionX = snake.front().mX;
				int currentPositionY = snake.front().mY;

				int deltaPosition;

				switch (newDirection)
				{
				case 0: //UP
					deltaPosition = currentPositionY == 3 ? SCREEN_HEIGHT - 2 : currentPositionY - 1;
					snake.push_front({ snake.front().mX, deltaPosition });
					break;

				case 1: // RIGHT
					deltaPosition = currentPositionX == SCREEN_WIDTH - 2 ? 2 : currentPositionX + 2;
					snake.push_front({ deltaPosition, snake.front().mY });
					break;

				case 2: //DOWN
					deltaPosition = currentPositionY == SCREEN_HEIGHT - 2 ? 3 : currentPositionY + 1;
					snake.push_front({ snake.front().mX, deltaPosition });
					break;

				case 3: //LEFT
					deltaPosition = currentPositionX == 2 ? SCREEN_WIDTH - 2 : currentPositionX - 2;
					snake.push_front({ deltaPosition, snake.front().mY });
					break;

				default:
					break;
				}

				currentDirection = newDirection;
				snake.pop_back();

				// Collision Detection

				// With Food
				if (snake.front().mX == foodX && snake.front().mY == foodY)
				{
					score++;
					gameSpeed += speedUpPerFood;

					while (screen[foodY * SCREEN_WIDTH + foodX] != L' ')
					{
						foodX = rand() % SCREEN_WIDTH;
						if (foodX % 2 == 1)
							foodX++;

						foodY = rand() % (SCREEN_HEIGHT - 3) + 3;
					}

					// Grow Snake
					for (int i = 0; i < growthPerFood; i++)
						snake.emplace_back(snake.back().mX, snake.back().mY);
				}

				// With Body --> DIE
				for (auto itr = snake.begin(); itr != snake.end(); ++itr)
				{
					if (itr != snake.begin() && itr->mX == snake.front().mX && itr->mY == snake.front().mY)
					{
						dead = true;

						if (score > bestScore)
							bestScore = score;

						// Store Player Score
						leaderBoard.emplace_back(playerName, bestScore, difficulty);

						std::sort(leaderBoard.begin(), leaderBoard.end(),
							[](PlayerData a, PlayerData b) { return a.mScore > b.mScore; });

						playerData.open("leaderboard.txt");

						for (int i = 0; i < 20; i++)
						{
							if (i == leaderBoard.size())
								break;

							playerData << leaderBoard[i].mName << " "
								<< leaderBoard[i].mScore << " "
								<< leaderBoard[i].mDifficulty << std::endl;
						}

						playerData.close();

						break;
					}
				}

				// Screen Routine

				// Clear Screen
				for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
					screen[i] = L' ';

				// Draw snake body
				for (auto s : snake)
					screen[s.mY * SCREEN_WIDTH + s.mX] = dead ? '+' : 'O';

				// Draw snake head
				screen[snake.front().mY * SCREEN_WIDTH + snake.front().mX] = dead ? 'X' : '@';

				// Draw Level
				for (int i = 0; i < SCREEN_HEIGHT; i++)
				{
					screen[i * SCREEN_WIDTH] = L'|';
					screen[i * SCREEN_WIDTH + SCREEN_WIDTH - 1] = L'|';
				}

				for (int i = 0; i < SCREEN_WIDTH; i++)
				{
					screen[i] = L'=';
					screen[i + SCREEN_WIDTH * 2] = L'=';
					screen[i + SCREEN_WIDTH * (SCREEN_HEIGHT - 1)] = L'=';
				}

				wsprintf(&screen[SCREEN_WIDTH + 4], L"S N A K E");
				wsprintf(&screen[SCREEN_WIDTH * 2 - 12], L"SCORE: %d", score);

				wsprintf(&screen[SCREEN_WIDTH * 2 - 26], L"PLAYER: %s", &playerName);

				/*wsprintf(&screen[SCREEN_WIDTH * 2 - 24], L"position: X: %d, Y: %d",
					snake.front().mX % SCREEN_WIDTH / 2,
					snake.front().mY % SCREEN_HEIGHT - 2);*/

				// Draw food
				screen[foodY * SCREEN_WIDTH + foodX] = '%';
			}

			if (dead)
			{
				if (0x8000 & GetAsyncKeyState((unsigned char)'\x20'))
				{
					playing = true;
					dead = false;
				}

				if (0x8000 & GetAsyncKeyState((unsigned char)'\x1B'))
				{
					playing = false;
					dead = false;
					menu = true;
				}

				wsprintf(&screen[18 * SCREEN_WIDTH + 44], L" BEST SCORE: %d", bestScore);
				wsprintf(&screen[20 * SCREEN_WIDTH + 44], L" PRESS 'SPACE' TO PLAY AGAIN");
				wsprintf(&screen[21 * SCREEN_WIDTH + 44], L" PRESS 'ESC' TO RETURN TO MENU");
			}

			// Output to the console
			Render(screen, console, bytesWritten);
		}
	}

	return 0;
}
