#include <algorithm>
#include <string>
#include <Windows.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <array>

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
	nightmare = 2
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

	explicit PlayerData(const wchar_t* name = L"AAA", const int score = 0, const int difficulty = normal)
		: mName(name), mScore(score), mDifficulty(difficulty) {}
};

struct SnakeElement
{
	int mX;
	int mY;

	SnakeElement(const int x = 0, const int y = 0)
		: mX(x), mY(y) {}
};

class Snake
{
public:
	Snake()
	{
		Init();
	}

private:
	const int startX = 60;
	const int startY = 15;
	int size = 7;
	const static int capacity = SCREEN_WIDTH * SCREEN_HEIGHT / 2;
	std::array<SnakeElement, capacity> body;

	void Init()
	{
		body[0] = { startX, startY };

		int x = startX + 2;
		for (int i = 1; i < size; i++)
		{
			body[i] = { x, startY };
			x += 2;
		}
	}

public:
	const int Size() const
	{
		return size;
	}

	const SnakeElement& Head() const
	{
		return body[0];
	}

	const SnakeElement& BodyAt(int index)
	{
		return body[index];
	}

	void Move(int x, int y)
	{
		for (int i = size - 1; i > 0; --i)
		{
			body[i] = body[i - 1];
		}

		body[0].mX = x;
		body[0].mY = y;
	}

	void Grow()
	{
		body[size] = body[size - 1];
		size++;
	}
};

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
		std::wstring name;
		playerData >> name;

		if (name.empty())
			break;

		PlayerData pd;
		pd.mName = name;
		playerData >> pd.mScore >> pd.mDifficulty;
		leaderBoard.emplace_back(pd);
	}

	playerData.close();

	std::array<PlayerData, 10> easyData;
	std::array<PlayerData, 10> hardData;

	// Create a screen buffer
	wchar_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	{
		screen[i] = L' ';
	}

	HANDLE console = CreateConsoleScreenBuffer(GENERIC_WRITE, 0,
		nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;

	bool game = true;
	bool menu = true;

	int difficulty = normal;
	wchar_t* playerName = new wchar_t[]{ 'A', 'A', 'A', '\0' };
	int bestScore = 0;

	while (game)
	{
		// Menu
		int cursor = enterName;
		bool anyKeyPressed = true;
		bool nameInput = false;
		bool leaderBoardPanel = false;
		int nameCharacterPos = 0;
		const wchar_t* diffChar = difficulty == normal ? L"NORMAL" : L"NIGHTMARE";

		bool keyUp = false;
		bool keyDown = false;
		bool keyLeft = false;
		bool keyRight = false;
		bool enter = false;

		while (menu)
		{
			keyUp = (0x8000 & GetAsyncKeyState((unsigned char)'\x26')) != 0;
			keyDown = (0x8000 & GetAsyncKeyState((unsigned char)'\x28')) != 0;
			keyLeft = (0x8000 & GetAsyncKeyState((unsigned char)'\x25')) != 0;
			keyRight = (0x8000 & GetAsyncKeyState((unsigned char)'\x27')) != 0;
			enter = (0x8000 & GetAsyncKeyState((unsigned char)'\xD')) != 0
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
					difficulty = difficulty == normal ? nightmare : normal;
					diffChar = difficulty == normal ? L"NORMAL" : L"NIGHTMARE";
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

			for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
			{
				screen[i] = L' ';
			}

			if (leaderBoardPanel)
			{
				int easyIndex = 0;
				int hardIndex = 0;

				for (auto& pd : leaderBoard)
				{
					if (easyIndex < easyData.size() - 1 && pd.mDifficulty == normal)
					{
						easyData[easyIndex] = pd;
						easyIndex++;
					}

					if (hardIndex < hardData.size() - 1 && pd.mDifficulty == nightmare)
					{
						hardData[hardIndex] = pd;
						hardIndex++;
					}
				}

				wsprintf(&screen[SCREEN_WIDTH * 24 / 2 + SCREEN_WIDTH / 2 - 6], L"LEADERBOARD");
				wsprintf(&screen[SCREEN_WIDTH * 32 / 2 + SCREEN_WIDTH / 2 - 12], L"NORMAL");
				wsprintf(&screen[SCREEN_WIDTH * 32 / 2 + SCREEN_WIDTH / 2 + 4], L"NIGHTMARE");

				for (int i = 0; i < 10; i++)
				{
					int space = i < 9 ? 15 : 16;
					wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 - space], L"%d.", i + 1);
					screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1] = L'|';

					if (i < easyIndex)
					{
						const wchar_t* easyName = easyData[i].mName.c_str();
						space = easyData[i].mScore > 9 ? 6 : 5;
						wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 - 11], easyName);
						wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 - space], L"%d",
							easyData[i].mScore);
					}

					if (i < hardIndex)
					{
						const wchar_t* hardName = hardData[i].mName.c_str();
						space = hardData[i].mScore > 9 ? 10 : 11;
						wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 + 5], hardName);
						wsprintf(&screen[(i + 18) * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 1 + space], L"%d",
							hardData[i].mScore);
					}
				}

				wsprintf(&screen[32 * SCREEN_WIDTH + SCREEN_WIDTH / 2 - 4], L"+ BACK");
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
			WriteConsoleOutputCharacter(console, screen,
				SCREEN_WIDTH * SCREEN_HEIGHT, { 0, 0 }, &bytesWritten);
		}

		// Initialize Variables
		/*std::list<SnakeElement> snake = {
			{ 60, 15 },
			{ 62, 15 },
			{ 64, 15 },
			{ 66, 15 },
			{ 68, 15 },
			{ 70, 15 },
			{ 72, 15 },
		};*/

		Snake s;

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
			if (dead)
			{
				if (0x8000 & GetAsyncKeyState((unsigned char)'\x20'))
				{
					playing = false;
					dead = false;
					break;
				}

				if (0x8000 & GetAsyncKeyState((unsigned char)'\x1B'))
				{
					playing = false;
					dead = false;
					menu = true;
					break;
				}

				wsprintf(&screen[18 * SCREEN_WIDTH + 44], L" BEST SCORE: %d", bestScore);
				wsprintf(&screen[20 * SCREEN_WIDTH + 44], L" PRESS 'SPACE' TO PLAY AGAIN");
				wsprintf(&screen[21 * SCREEN_WIDTH + 44], L" PRESS 'ESC' TO RETURN TO MENU");
			}

			// Update Position Loop (slower than input loop)
			if (!dead && deltaTime >= 1 / (float)gameSpeed * 100)
			{
				deltaTime = 0.0f;

				int currentPositionX = s.Head().mX;
				int currentPositionY = s.Head().mY;

				int deltaPosition;

				switch (newDirection)
				{
				case 0: //UP
					deltaPosition = currentPositionY == 3 ? SCREEN_HEIGHT - 2 : currentPositionY - 1;
					s.Move(currentPositionX, deltaPosition);
					break;

				case 1: // RIGHT
					deltaPosition = currentPositionX == SCREEN_WIDTH - 2 ? 2 : currentPositionX + 2;
					s.Move(deltaPosition, currentPositionY);
					break;

				case 2: //DOWN
					deltaPosition = currentPositionY == SCREEN_HEIGHT - 2 ? 3 : currentPositionY + 1;
					s.Move(currentPositionX, deltaPosition);
					break;

				case 3: //LEFT
					deltaPosition = currentPositionX == 2 ? SCREEN_WIDTH - 2 : currentPositionX - 2;
					s.Move(deltaPosition, currentPositionY);
					break;

				default:
					break;
				}

				currentDirection = newDirection;

				// Collision Detection

				// With Food
				if (s.Head().mX == foodX && s.Head().mY == foodY)
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

					for (int i = 0; i < growthPerFood; i++)
					{
						s.Grow();
					}
				}

				// With Body --> DIE
				for (int i = 1; i < s.Size(); i++)
				{
					auto body = s.BodyAt(i);

					if (body.mX == s.Head().mX && body.mY == s.Head().mY)
					{
						dead = true;

						if (score > bestScore)
							bestScore = score;

						// Store Player Score
						leaderBoard.emplace_back(playerName, bestScore, difficulty);

						std::sort(leaderBoard.begin(), leaderBoard.end(),
							[](const PlayerData a, const PlayerData b)
							{
								return a.mScore > b.mScore;
							});

						playerData.open("leaderboard.txt");

						for (int i = 0; i < 40; i++)
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

				for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
				{
					screen[i] = L' ';
				}

				// Draw snake body
				for (int i = 0; i < s.Size(); i++)
				{
					screen[s.BodyAt(i).mY * SCREEN_WIDTH + s.BodyAt(i).mX] = dead ? L'+' : L'O';
				}

				// Draw snake head
				screen[s.Head().mY * SCREEN_WIDTH + s.Head().mX] = dead ? L'X' : L'@';

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

				wsprintfW(&screen[SCREEN_WIDTH + 4], L"S N A K E");
				wsprintfW(&screen[SCREEN_WIDTH * 2 - 12], L"SCORE: %d", score);

				wsprintfW(&screen[SCREEN_WIDTH * 2 - 26], L"PLAYER: %s", &playerName);

				/*wsprintf(&screen[SCREEN_WIDTH * 2 - 24], L"position: X: %d, Y: %d",
					snake.front().mX % SCREEN_WIDTH / 2,
					snake.front().mY % SCREEN_HEIGHT - 2);*/

				// Draw food
				screen[foodY * SCREEN_WIDTH + foodX] = '%';
			}

			// Output to the console
			WriteConsoleOutputCharacter(console, screen,
				SCREEN_WIDTH * SCREEN_HEIGHT, { 0, 0 }, &bytesWritten);
		}
	}

	return 0;
}
