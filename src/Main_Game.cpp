#include "platform_layer.h"
#include "main_game.h"

#include "stdlib.h" //for random
#include "time.h" //seed for random

#define CHESS_LAYOUT 0

void mainUpdateAndRender(GameInput gameInput, float fps, GameState &gameState)
{
	MainState &mainState = gameState.mainState;
	Assets &assets = gameState.assets;
	Piece &heldPiece = mainState.heldPiece;
	Vec2 &shadowPos = mainState.shadowPos;
	PieceTypeProperties &pieceTypeProperties = mainState.pieceTypeProperties;

	Piece *whiteHand = mainState.hands[PC_WHITE]; //shouldn't need these (should be able to simplify code to loop instead of repeat it's self)
	Piece *blackHand = mainState.hands[PC_BLACK];

	GameTurn &gameTurn = mainState.gameTurn;
	TurnPhase &turnPhase = mainState.turnPhase;

	//don't know if i will load all assets at the start, but this can stay here for now
	//in the future make a assets.cpp that has a funciton so i can call it here and move it around if i need
	//and then maybe move it into another thread for speed
	if (!assets.init)
	{
		assets.pieceTextures[PC_WHITE][PT_PAWN] = loadTexture("resources/images/pieces/white/pawn.png");
		assets.pieceTextures[PC_WHITE][PT_ROOK] = loadTexture("resources/images/pieces/white/rook.png");
		assets.pieceTextures[PC_WHITE][PT_KNIGHT] = loadTexture("resources/images/pieces/white/knight.png");
		assets.pieceTextures[PC_WHITE][PT_BISHOP] = loadTexture("resources/images/pieces/white/bishop.png");
		assets.pieceTextures[PC_WHITE][PT_QUEEN] = loadTexture("resources/images/pieces/white/queen.png");
		assets.pieceTextures[PC_WHITE][PT_KING] = loadTexture("resources/images/pieces/white/king.png");
		assets.pieceTextures[PC_WHITE][PT_SPIRIT] = loadTexture("resources/images/pieces/white/spirit.png");
		assets.pieceTextures[PC_WHITE][PT_ROCK] = loadTexture("resources/images/pieces/white/rock.png");

		assets.pieceTextures[PC_BLACK][PT_PAWN] = loadTexture("resources/images/pieces/black/pawn.png");
		assets.pieceTextures[PC_BLACK][PT_ROOK] = loadTexture("resources/images/pieces/black/rook.png");
		assets.pieceTextures[PC_BLACK][PT_KNIGHT] = loadTexture("resources/images/pieces/black/knight.png");
		assets.pieceTextures[PC_BLACK][PT_BISHOP] = loadTexture("resources/images/pieces/black/bishop.png");
		assets.pieceTextures[PC_BLACK][PT_QUEEN] = loadTexture("resources/images/pieces/black/queen.png");
		assets.pieceTextures[PC_BLACK][PT_KING] = loadTexture("resources/images/pieces/black/king.png");
		assets.pieceTextures[PC_BLACK][PT_SPIRIT] = loadTexture("resources/images/pieces/black/spirit.png");
		assets.pieceTextures[PC_BLACK][PT_ROCK] = loadTexture("resources/images/pieces/black/rock.png");

		assets.otherTextures[OT_CLOCK] = loadTexture("resources/images/clock.png");

		assets.hemiHead = loadFont("resources/fonts/hemi_head.ttf");

		assets.init = true;
	}

	//board info
	Vec2 boardSquares = { BOARD_SQUARES_DOWN, BOARD_SQUARES_ACROSS };
	Vec2 squareSize = { 100, 100 };
	Vec2 boardStart = { (WINDOW_WIDTH / 2) - ((boardSquares.x * squareSize.x) / 2), (WINDOW_HEIGHT / 2) - ((boardSquares.y * squareSize.y) / 2) }; //y = 100
	bool colour = 0;
	unsigned char boardLightColour = 150;
	unsigned char boardDarkColour = 50;

	if (!mainState.init)
	{
		srand(time(0)); //seed for random

		gameTurn = GT_WHITE;
		turnPhase = DRAW_PHASE;
		mainState.health[PC_WHITE] = 3;
		mainState.health[PC_BLACK] = 3;

		//properites for piece types
		for (int i = 0; i < PT_SIZE; i++)
		{
			pieceTypeProperties.canBeTaken[i] = true;
			pieceTypeProperties.canBeSquashedByFriendly[i] = true;
			pieceTypeProperties.canBeSquashedByEnimy[i] = true;
			pieceTypeProperties.canDirectAttack[i] = true;
			pieceTypeProperties.canBePlacedFromHandToBlank[i] = true;
			pieceTypeProperties.maxCooldown[i] = 1;
		}

		pieceTypeProperties.canBeTaken[PT_ROCK] = false;
		pieceTypeProperties.canBeSquashedByEnimy[PT_ROCK] = false;
		pieceTypeProperties.canDirectAttack[PT_ROCK] = false;
		pieceTypeProperties.canProduce[PT_ROCK] = true;

		pieceTypeProperties.canSquashEnemyFromHand[PT_PAWN] = true;

		pieceTypeProperties.canBePlacedFromHandToBlank[PT_QUEEN] = false;
		pieceTypeProperties.canSquashFriendlyFromHand[PT_QUEEN] = true;

		//add pieces to each hand
		int handStartPieces = 5;
		for (int colourNum = 0; colourNum < PC_SIZE; colourNum++)
		{
			for (int handIndex = 0; handIndex < 5; handIndex++)
			{
				Piece &piece = mainState.hands[colourNum][handIndex];
				piece.colour = (PieceColour)colourNum;
				piece.type = (PieceType)((rand() % (PT_SIZE - 1)) + 1);
			}
		}

		//add pieces to the board
		mainState.boardState[2][1] = Piece{ PT_ROCK, PC_BLACK };
		mainState.boardState[2][6] = Piece{ PT_ROCK, PC_BLACK };

		mainState.boardState[5][1] = Piece{ PT_ROCK, PC_WHITE };
		mainState.boardState[5][6] = Piece{ PT_ROCK, PC_WHITE };

		mainState.init = true;
	}

	//find the square that the mouse is on
	Vec2 mouseSquare = { -1, -1 };
	for (int downIndex = 0; downIndex < boardSquares.y; downIndex++)
	{
		for (int acrossIndex = 0; acrossIndex < boardSquares.x; acrossIndex++)
		{
			Vec2 boardSqaureNum = { (float)downIndex, (float)acrossIndex };
			if (pointInsideRect(gameInput.mousePos, boardStart + (boardSqaureNum * squareSize), squareSize))
			{
				mouseSquare = boardSqaureNum;
				break;
			}
		}
	}

	bool isMouseInsideBoard = (mouseSquare.x >= 0 && mouseSquare.y >= 0);
	Piece &mouseOverPiece = mainState.boardState[(int)mouseSquare.y][(int)mouseSquare.x];
	Vec2 boardSize = { (boardSquares.x * squareSize.x), (boardSquares.y * squareSize.y) };

	//calculate where the pieces can go
	bool boardMovesAllowed[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS] = {};

	if (heldPiece.type != PT_BLANK && turnPhase == MOVE_PHASE)
	{
		checkMoves(heldPiece, shadowPos, boardMovesAllowed, mainState.boardState, pieceTypeProperties);
	}
	else if (heldPiece.type != PT_BLANK)
	{
		checkMoves(heldPiece, mouseSquare, boardMovesAllowed, mainState.boardState, pieceTypeProperties);
	}
	else if (mouseOverPiece.type != PT_BLANK)
	{
		checkMoves(mouseOverPiece, mouseSquare, boardMovesAllowed, mainState.boardState, pieceTypeProperties);
	}

	//calculate where the pieces can be placed
	bool spawnsAllowed[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS] = {};

	if (turnPhase == HAND_PHASE && heldPiece.type != PT_BLANK)
	{
		for (int downIndex = 0; downIndex < BOARD_SQUARES_DOWN; downIndex++)
		{
			for (int acrossIndex = 0; acrossIndex < BOARD_SQUARES_ACROSS; acrossIndex++)
			{
				Piece boardPiece = mainState.boardState[downIndex][acrossIndex];
				if (boardPiece.type != PT_BLANK)
				{
					if (pieceTypeProperties.canProduce[boardPiece.type] && pieceTypeProperties.canBePlacedFromHandToBlank[heldPiece.type] && boardPiece.colour == heldPiece.colour)
					{
						Vec2 pos = Vec2{ (float)acrossIndex, (float)downIndex };
						calculateSpawns(spawnsAllowed, pos, mainState.boardState);
					}
					else if (mainState.boardState[downIndex][acrossIndex].colour == heldPiece.colour && pieceTypeProperties.canSquashFriendlyFromHand[heldPiece.type])
					{
						Vec2 pos = Vec2{ (float)acrossIndex, (float)downIndex };
						spawnsAllowed[downIndex][acrossIndex] = true;
					}
					else if (mainState.boardState[downIndex][acrossIndex].colour != heldPiece.colour && pieceTypeProperties.canSquashEnemyFromHand[heldPiece.type])
					{
						Vec2 pos = Vec2{ (float)acrossIndex, (float)downIndex };
						spawnsAllowed[downIndex][acrossIndex] = true;
					}
#if 1
					if ((heldPiece.type == PT_BISHOP && boardPiece.type == PT_ROOK) ||
						(heldPiece.type == PT_ROOK && boardPiece.type == PT_BISHOP))
					{
						spawnsAllowed[downIndex][acrossIndex] = true;
					}
#endif
				}
			}
		}
		for (int downIndex = 0; downIndex < BOARD_SQUARES_DOWN; downIndex++)
		{
			for (int acrossIndex = 0; acrossIndex < BOARD_SQUARES_ACROSS; acrossIndex++)
			{
				bool boardSquare = spawnsAllowed[downIndex][acrossIndex];
				if ((boardSquare && heldPiece.colour == PC_WHITE && downIndex == 0) ||
					(boardSquare && heldPiece.colour == PC_BLACK && downIndex == (BOARD_SQUARES_DOWN - 1)))
				{
					spawnsAllowed[downIndex][acrossIndex] = false;
				}
				Piece boardPiece = mainState.boardState[downIndex][acrossIndex];
				if ((pieceTypeProperties.canBeSquashedByFriendly[boardPiece.type] == false && heldPiece.colour == boardPiece.colour) || 
					(pieceTypeProperties.canBeSquashedByEnimy[boardPiece.type] == false && heldPiece.colour != boardPiece.colour))
				{
					spawnsAllowed[downIndex][acrossIndex] = false;
				}
			}
		}
	}

	//calculate whether the piece that is held can be placed from the mouse position
	bool heldPieceCanBePlacedHere = false;

	if (heldPiece.type != PT_BLANK && isMouseInsideBoard)
	{
		if (turnPhase == HAND_PHASE && spawnsAllowed[(int)mouseSquare.y][(int)mouseSquare.x])
		{
			heldPieceCanBePlacedHere = true;
		}
		else if (turnPhase == MOVE_PHASE && boardMovesAllowed[(int)mouseSquare.y][(int)mouseSquare.x])
		{
			heldPieceCanBePlacedHere = true;
		}
	}

	//*

#if 1 //update the game based on the turnPhase (should change to update based on input)
	if (turnPhase == DRAW_PHASE)
	{
		//add piece to hand
		if (gameTurn == GT_WHITE)
		{
			int whiteHandSize = getHandSize(whiteHand);
			whiteHand[whiteHandSize] = Piece{ PieceType((rand() % (PT_SIZE - 1)) + 1), PC_WHITE };
		}
		else if (gameTurn == GT_BLACK)
		{
			int blackHandSize = getHandSize(blackHand);
			blackHand[blackHandSize] = Piece{ PieceType((rand() % (PT_SIZE - 1)) + 1), PC_BLACK };
		}

		//update anything that needs to work each turn
		for (int downIndex = 0; downIndex < BOARD_SQUARES_DOWN; downIndex++)
		{
			for (int acrossIndex = 0; acrossIndex < BOARD_SQUARES_ACROSS; acrossIndex++)
			{
				Piece &piece = mainState.boardState[downIndex][acrossIndex];
				if (piece.colour != gameTurn)
				{
					if (piece.cooldown > 0)
					{
						piece.cooldown--;
					}

					//update spirit things every draw phase (do i want this here)
					if (piece.type == PT_SPIRIT)
					{
						Piece &potentialSpawnPiece = mainState.boardState[(int)(downIndex + relative(piece.colour, 1))][acrossIndex];

						if (potentialSpawnPiece.type == PT_BLANK)
						{
							potentialSpawnPiece.type = PT_PAWN;
							potentialSpawnPiece.colour = piece.colour;
						}
					}
				}
			}
		}

		turnPhase = HAND_PHASE;
	}
	else if (turnPhase == HAND_PHASE) //should put the taking from hand code in the same place
	{
		if (gameInput.mouseButton.leftClicked && heldPiece.type != PT_BLANK && heldPieceCanBePlacedHere)
		{
			if ((heldPiece.type == PT_BISHOP && mouseOverPiece.type == PT_ROOK) ||
				(heldPiece.type == PT_ROOK && mouseOverPiece.type == PT_BISHOP))
			{
				heldPiece.type = PT_QUEEN;
				mouseOverPiece = heldPiece;
				heldPiece = {};
				turnPhase = MOVE_PHASE;
			}
			else
			{
				mouseOverPiece = heldPiece;
				heldPiece = {};
				mouseOverPiece.cooldown = pieceTypeProperties.maxCooldown[mouseOverPiece.type];
				turnPhase = MOVE_PHASE;
			}
		}
	}
	else if (turnPhase == MOVE_PHASE && gameInput.mouseButton.leftClicked && isMouseInsideBoard)
	{
		if (heldPiece.type != PT_BLANK) //putting down piece
		{
			if (shadowPos == mouseSquare)
			{
				mouseOverPiece = heldPiece;
				heldPiece = {};
			}
			else if (boardMovesAllowed[(int)mouseSquare.y][(int)mouseSquare.x] == true)
			{
				if (((gameTurn == GT_WHITE && mouseSquare.y == 0) || (gameTurn == GT_BLACK && mouseSquare.y == (BOARD_SQUARES_DOWN - 1))) && pieceTypeProperties.canDirectAttack[heldPiece.type])
				{
					heldPiece = {};
					mainState.health[!gameTurn] -= 1;
				}
				else
				{
					mouseOverPiece = heldPiece;
					heldPiece = {};
				}

				gameTurn = (GameTurn)(!gameTurn);
				turnPhase = DRAW_PHASE;
			}
		}
		else //picking up piece
		{
			if ((mouseOverPiece.type != PT_BLANK) && (mouseOverPiece.colour == gameTurn) && (mouseOverPiece.cooldown == 0))
			{
				heldPiece = mouseOverPiece;
				shadowPos = mouseSquare;
				mouseOverPiece = {};
			}
		}
	}

	//render

	//draw background
	drawRect(0, 0, 1920, 1080, 80, 80, 120); //150, 0, 200

	//update and draw hand
	Vec2 handsCenter[GT_SIZE] =
	{
		{ boardStart.x + (boardSize.x / 2.0f), boardStart.y / 2.0f },
		{ boardStart.x + (boardSize.x / 2.0f), boardStart.y + boardSize.y + (boardStart.y / 2.0f) }
	};

	for (int turnIndex = 0; turnIndex < GT_SIZE; turnIndex++)
	{
		Piece(&hand)[20] = mainState.hands[turnIndex];
		Vec2 handCenter = handsCenter[turnIndex];
		int handSize = getHandSize(hand);

		for (int handIndex = 0; handIndex < handSize; handIndex++)
		{
			Piece &piece = hand[handIndex];
			Vec2 pos = { (handCenter.x - ((squareSize.x * handSize) / 2)) + (handIndex * squareSize.x), handCenter.y - (squareSize.y / 2.0f) };

			if (imgButton(gameInput, assets.pieceTextures[turnIndex][piece.type], pos, squareSize) && turnIndex == gameTurn && turnPhase == HAND_PHASE) //this should be somewhere else
			{
				if (heldPiece.type == PT_BLANK)
				{
					heldPiece = piece;
					piece = {};
					bunchHand(hand);
				}
				else
				{
					//put the piece at the end 
					//(maybe should put it after the piece in the hand so the hand can be organized by the player)
					hand[handSize] = heldPiece;
					heldPiece = {};
				}
				break;
			}
		}
	}

#endif

	//draw board
	for (int downIndex = 0; downIndex < boardSquares.y; downIndex++)
	{
		colour = !colour;
		for (int acrossIndex = 0; acrossIndex < boardSquares.x; acrossIndex++)
		{
			if (colour)
			{
				drawRect(boardStart.x + (acrossIndex * squareSize.x), boardStart.y + (downIndex * squareSize.y), squareSize.x, squareSize.y, boardLightColour, boardLightColour, boardLightColour);
			}
			else
			{
				drawRect(boardStart.x + (acrossIndex * squareSize.x), boardStart.y + (downIndex * squareSize.y), squareSize.x, squareSize.y, boardDarkColour, boardDarkColour, boardDarkColour);
			}
			colour = !colour;
		}
	}

	//draw pieces
	for (int downIndex = 0; downIndex < boardSquares.y; downIndex++)
	{
		for (int acrossIndex = 0; acrossIndex < boardSquares.x; acrossIndex++)
		{
			Vec2 moveIndex = { (float)acrossIndex, (float)downIndex };
			Piece &piece = mainState.boardState[downIndex][acrossIndex];
			drawTexture(assets.pieceTextures[piece.colour][piece.type], boardStart + (moveIndex * squareSize), squareSize);
		}
	}

	//draw shadow Piece
	if (turnPhase == MOVE_PHASE)
	{
		drawTexture(assets.pieceTextures[heldPiece.colour][heldPiece.type], boardStart + (shadowPos * squareSize), squareSize, 50);
	}

	//draw possible moves, spawns
	if (turnPhase == HAND_PHASE)
	{
		if (heldPieceCanBePlacedHere)
		{
			makeBoardHighlights(boardMovesAllowed, Colour{ 0, 255, 0 }, Colour{ 255, 0, 0 }, heldPiece, turnPhase, mainState.boardState, boardStart, squareSize);
		}
		else
		{
			makeBoardHighlights(spawnsAllowed, Colour{ 0, 0, 255 }, Colour{ 0, 0, 100 }, heldPiece, turnPhase, mainState.boardState, boardStart, squareSize);
		}
	}
	else
	{
		makeBoardHighlights(boardMovesAllowed, Colour{ 0, 255, 0 }, Colour{ 255, 0, 0 }, heldPiece, turnPhase, mainState.boardState, boardStart, squareSize);
	}

	//draw selection box
	if (isMouseInsideBoard)
	{
		if (gameTurn == GT_WHITE)
		{
			drawOutlineRect(boardStart + (mouseSquare * squareSize), squareSize, Colour{ 255, 255, 255, 255 }, 5);
		}
		else if (gameTurn == GT_BLACK)
		{
			drawOutlineRect(boardStart + (mouseSquare * squareSize), squareSize, Colour{ 0, 0, 0, 255 }, 5);
		}
	}

	//draw cooldown overlay
	for (int downIndex = 0; downIndex < BOARD_SQUARES_DOWN; downIndex++)
	{
		for (int acrossIndex = 0; acrossIndex < BOARD_SQUARES_ACROSS; acrossIndex++)
		{
			Piece &piece = mainState.boardState[downIndex][acrossIndex];
			Vec2 moveIndex = Vec2{ (float)acrossIndex, (float)downIndex };
			if (piece.cooldown > 0)
			{
				drawTexture(assets.otherTextures[OT_CLOCK], boardStart + (moveIndex * squareSize), squareSize, 170);
			}
		}
	}

	//draw held piece
	if (heldPieceCanBePlacedHere)
	{
		Vec2 pos = ((gameInput.mousePos - squareSize / 2.0f) + ((boardStart + (mouseSquare * squareSize))) * 5.0f) / 6.0f;
		drawTexture(assets.pieceTextures[heldPiece.colour][heldPiece.type], pos, squareSize);
	}
	else
	{
		drawTexture(assets.pieceTextures[heldPiece.colour][heldPiece.type], gameInput.mousePos - (squareSize + 10) / 2.0f, squareSize + 10);
	}


	//gui
	char text[SMALL_TEXT] = "";
	char text2[SMALL_TEXT] = "";

	if (gameTurn == GT_WHITE)
	{
		concat(text, "white turn");
	}
	else if (gameTurn == GT_BLACK)
	{
		concat(text, "black turn");
	}

	if (turnPhase == DRAW_PHASE)
	{
		concat(text, " - draw phase");
	}
	else if (turnPhase == HAND_PHASE)
	{
		concat(text, " - hand phase");
	}
	else if (turnPhase == MOVE_PHASE)
	{
		concat(text, " - board move phase");
	}

	renderTextOrigin(text, assets.hemiHead, Vec2{ 10, boardStart.y + (boardSize.y / 2.0f) }, Vec2{ boardStart.x - 20, 0 }, Vec2{ 0, 0.5 }, Colour{ 0,0,0,255 });

	toCharList(text, mainState.health[PC_WHITE]);
	concat(text2, "white health: ");
	concat(text2, text);
	renderTextOrigin(text2, assets.hemiHead, Vec2{ boardStart.x - 30.0f, (boardStart.y + boardSize.y) + ((WINDOW_HEIGHT - (boardStart.y + boardSize.y)) / 2.0f) }, Vec2{ 0, 30 }, Vec2{ 0.5, 0.5 }, Colour{ 0,0,0,255 });

	toCharList(text, mainState.health[PC_BLACK]);
	text2[0] = 0;
	concat(text2, "black health: ");
	concat(text2, text);
	renderTextOrigin(text2, assets.hemiHead, Vec2{ boardStart.x - 30.0f, boardStart.y / 2.0f }, Vec2{ 0, 30 }, Vec2{ 0.5, 0.5 }, Colour{ 0,0,0,255 });

	//update and draw piece adding menu
#if DEBUG
	if (mainState.debugMenuShowing)
	{
		Vec2 posIndex = { 0, 0 };
		Vec2 menuStart = { 1400, 10 };
		for (int colNum = 0; colNum < PC_SIZE; colNum++)
		{
			for (int i = 0; i < PT_SIZE; i++)
			{
				if (i == PT_BLANK) { continue; }

				if (imgButton(gameInput, assets.pieceTextures[colNum][i], menuStart + (posIndex * squareSize), squareSize))
				{
					mainState.selectedPiece = { (PieceType)i, (PieceColour)colNum };
				}

				if (posIndex.x++ > 3)
				{
					posIndex.x = 0;
					posIndex.y++;
				}
			}
		}

		if (gameInput.mouseButton.middle)
		{
			mainState.selectedPiece = { PT_BLANK, PC_WHITE };
		}

		if (gameInput.mouseButton.right && mouseSquare.x != -1 && mouseSquare.y != -1)
		{
			mouseOverPiece = mainState.selectedPiece;
		}
	}
	if (gameInput.actionPressed)
	{
		mainState.debugMenuShowing = !mainState.debugMenuShowing;
	}

#endif

}

void makeBoardHighlights(bool board[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Colour blankSquareColour, Colour pieceSquareColour,  Piece heldPiece, TurnPhase turnPhase, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Vec2 boardStart, Vec2 squareSize)
{
	for (int downIndex = 0; downIndex < BOARD_SQUARES_DOWN; downIndex++)
	{
		for (int acrossIndex = 0; acrossIndex < BOARD_SQUARES_ACROSS; acrossIndex++)
		{
			if (board[downIndex][acrossIndex] == true)
			{
				Vec2 moveIndex = { (float)acrossIndex, (float)downIndex };
				unsigned char alpha = 0;
				if (heldPiece.type != PT_BLANK && turnPhase == MOVE_PHASE)
				{
					alpha = 70;
				}
				else
				{
					alpha = 40;
				}

				blankSquareColour.a = alpha;
				pieceSquareColour.a = alpha;

				if (boardState[downIndex][acrossIndex].type == PT_BLANK)
				{
					drawRect(boardStart + (moveIndex * squareSize), squareSize, blankSquareColour);
				}
				else
				{
					drawRect(boardStart + (moveIndex * squareSize), squareSize, pieceSquareColour);
				}
			}
		}
	}
}

float relative(PieceColour pieceColour, float valueIfWhite)
{
	//negative value if black
	float result = ((pieceColour * 2) - 1) * valueIfWhite;
	return result;
}

void toCharList(char (&text)[SMALL_TEXT], int num)
{
	int digits[20];
	int runningTotal = 0;
	int digitsEnd = 0;

	for (int i = 0;; i++)
	{
		int tensValue = (int)pow(10, i + 1);
		if (num % tensValue == num)
		{
			digits[i] = (num % tensValue) - runningTotal;
			digitsEnd = i + 1;
			break;
		}
		else
		{
			digits[i] = (num % tensValue) - runningTotal;
			runningTotal = num % tensValue;
		}
	}

	for (int i = 0; i < digitsEnd; i++)
	{
		while (digits[i] % 10 == 0 && digits[i] != 0)
		{
			digits[i] /= 10;
		}
	}

	int newDigits[20];

	for (int i = 0; i < digitsEnd; i++)
	{
		newDigits[i] = digits[digitsEnd - 1 - i];
	}

	for (int i = 0; i < digitsEnd; i++)
	{
		text[i] = toChar(newDigits[i]);
	}

	text[digitsEnd] = 0;
}

char toChar(int num)
{
	char result = '0' + num;
	return result;
}

void concat(char (&start)[SMALL_TEXT], char* end)
{
	int endOfStartNum = 0;
	for (int i = 0;; i++)
	{
		if (start[i] == 0)
		{
			endOfStartNum = i;
			break;
		}
	}

	for (int i = 0;; i++)
	{
		if (end[i] == 0)
		{
			start[endOfStartNum + i] = 0;
			break;
		}
		else
		{
			start[endOfStartNum + i] = end[i];
		}
	}
}

void concat(char(&start)[BIG_TEXT], char* end)
{
	int endOfStartNum = 0;
	for (int i = 0;; i++)
	{
		if (start[i] == 0)
		{
			endOfStartNum = i;
			break;
		}
	}

	for (int i = 0;; i++)
	{
		if (end[i] == 0)
		{
			start[endOfStartNum + i] = 0;
			break;
		}
		else
		{
			start[endOfStartNum + i] = end[i];
		}
	}
}

int getHandSize(Piece hand[MAX_HAND_SIZE])
{
	int handSize = 0;
	for (int handIndex = 0; handIndex < MAX_HAND_SIZE; handIndex++)
	{
		if (hand[handIndex].type != PT_BLANK)
		{
			handSize = handIndex + 1;
		}
	}

	return handSize;
}

void bunchHand(Piece hand[MAX_HAND_SIZE])
{
	Piece newHand[MAX_HAND_SIZE] = {};
	int index = 0;

	//make new hand with the correct items
	for (int handIndex = 0; handIndex < MAX_HAND_SIZE; handIndex++)
	{
		if (hand[handIndex].type != PT_BLANK)
		{
			newHand[index++] = hand[handIndex];
		}
	}

	//copy new hand to hand
	for (int i = 0; i < MAX_HAND_SIZE; i++)
	{
		hand[i] = newHand[i];
	}

}

bool pointInsideRect(Vec2 point, Vec2 rectPos, Vec2 rectSize)
{
	Vec2 rectMin = rectPos;
	Vec2 rectMax = rectPos + rectSize;

	if (point.x >= rectMin.x &&
		point.x <= rectMax.x &&
		point.y >= rectMin.y &&
		point.y <= rectMax.y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool imgButton(GameInput gameInput, Texture texture, Vec2 pos, Vec2 size)
{
	if (pointInsideRect(gameInput.mousePos, pos, size))
	{
		drawTexture(texture, pos - 10.0f, size + 20.0f);

		if (gameInput.mouseButton.leftClicked)
		{
			return true;
		}
	}
	else
	{
		drawTexture(texture, pos, size);
		return false;
	}
}

void calculateSpawns(bool(&spawnsAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS])
{
	Vec2 checklist[8] =
	{
		{ piecePos.x + 1, piecePos.y },
		{ piecePos.x + 1, piecePos.y + 1 },
		{ piecePos.x + 1, piecePos.y - 1 },
		{ piecePos.x - 1, piecePos.y },
		{ piecePos.x - 1, piecePos.y + 1 },
		{ piecePos.x - 1, piecePos.y - 1 },
		{ piecePos.x, piecePos.y + 1 },
		{ piecePos.x, piecePos.y - 1 }
	};

	for (int i = 0; i < 8; i++)
	{
		Vec2 checkPos = checklist[i];
		if (boardState[(int)checkPos.y][(int)checkPos.x].type == PT_BLANK && checkPos.x >= 0 && checkPos.x < BOARD_SQUARES_ACROSS && checkPos.y >= 0 && checkPos.y < BOARD_SQUARES_DOWN)
		{
			spawnsAllowed[(int)checkPos.y][(int)checkPos.x] = true;
		}
	}
}

void checkMoves(Piece piece, Vec2 piecePos, bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties)
{
	switch (piece.type)
	{
	case PT_ROOK:
	{
		rookMoves(boardMovesAllowed, piece, piecePos, boardState, pieceTypeProperties);
		break;
	}
	case PT_KNIGHT:
	{
		knightMoves(boardMovesAllowed, piece, piecePos, boardState, pieceTypeProperties);
		break;
	}
	case PT_BISHOP:
	{
		bishopMoves(boardMovesAllowed, piece, piecePos, boardState, pieceTypeProperties);
		break;
	}
	case PT_QUEEN:
	{
		queenMoves(boardMovesAllowed, piece, piecePos, boardState, pieceTypeProperties);
		break;
	}
	case PT_KING:
	{
		kingMoves(boardMovesAllowed, piece, piecePos, boardState, pieceTypeProperties);
		break;
	}
	case PT_PAWN:
	{
		pawnMoves(boardMovesAllowed, piece, piecePos, boardState, pieceTypeProperties);
		break;
	}
	case PT_SPIRIT:
	{
		spiritMoves(boardMovesAllowed, piece, piecePos, boardState, pieceTypeProperties);
		break;
	}
	case PT_ROCK:
	{
		rockMoves(boardMovesAllowed, piece, piecePos, boardState, pieceTypeProperties);
		break;
	}
	}
}

bool changeSquare(Piece myPiece, Vec2 &checkingSpace, bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties, MoveRuleFlags flags)
{
	bool jumping = ((flags & 1) == 1); //make jumpingEnemy and jumpingFriendly instead
	bool takingEnemy = ((flags & (1 << 1)) == (1 << 1));
	bool movingOnBlanks = ((flags & (1 << 2)) == (1 << 2));
	bool takingFriendly = ((flags & (1 << 3)) == (1 << 3));

	Piece &checkingPiece = boardState[(int)checkingSpace.y][(int)checkingSpace.x];
	bool &square = boardMovesAllowed[(int)checkingSpace.y][(int)checkingSpace.x];

	if (movingOnBlanks)
	{
		if (checkingPiece.type == PT_BLANK)
		{
			square = true;
		}
	}

	if (checkingPiece.type != PT_BLANK)
	{
		if (takingEnemy)
		{
			if (checkingPiece.colour != myPiece.colour && pieceTypeProperties.canBeTaken[checkingPiece.type])
			{
				square = true;
			}
		}
		if (takingFriendly)
		{
			if (checkingPiece.colour == myPiece.colour)
			{
				square = true;
			}
		}
		if (!jumping) //doesn't really feel like it belongs here
		{
			return 1;
		}
	}
	return 0;
}

void extendMove(Piece myPiece, Vec2 start, Vec2 dir, int amount, bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties, MoveRuleFlags flags)
{
	Vec2 checkingPiecePos = start;

	if (amount > 0)
	{
		for (int i = 0; i < amount; i++)
		{
			Vec2 pos = checkingPiecePos + dir;
			if (pos.x >= 0 && pos.x < BOARD_SQUARES_ACROSS && pos.y >= 0 && pos.y < BOARD_SQUARES_DOWN)
			{
				checkingPiecePos += dir;
				ASSERT(checkingPiecePos.x < BOARD_SQUARES_ACROSS && checkingPiecePos.y < BOARD_SQUARES_DOWN);
				if (changeSquare(myPiece, checkingPiecePos, boardMovesAllowed, boardState, pieceTypeProperties, flags)) { break; }
			}
		}
	}
	else
	{
		Vec2 pos = checkingPiecePos + dir;
		while (pos.x >= 0 && pos.x < BOARD_SQUARES_ACROSS && pos.y >= 0 && pos.y < BOARD_SQUARES_DOWN)
		{
			checkingPiecePos += dir;
			ASSERT(checkingPiecePos.x < BOARD_SQUARES_ACROSS && checkingPiecePos.y < BOARD_SQUARES_DOWN);
			if (changeSquare(myPiece, checkingPiecePos, boardMovesAllowed, boardState, pieceTypeProperties, flags)) { break; }
			pos = checkingPiecePos + dir;
		}
	}
}

//this could be cut down more (i think, although maybe i'm just thinking about metaprogramming too much)
void rookMoves(bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties)
{
	extendMove(myPiece, piecePos, Vec2{ 1, 0 }, -1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ -1, 0 }, -1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ 0, 1 }, -1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ 0, -1 }, -1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
}

void knightMoves(bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties)
{
	extendMove(myPiece, piecePos, Vec2{ 2, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ 2, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ 1, 2 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ 1, -2 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));

	extendMove(myPiece, piecePos, Vec2{ -2, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ -2, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ -1, 2 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ -1, -2 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
}

void bishopMoves(bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties)
{
	extendMove(myPiece, piecePos, Vec2{ 1, 1 }, -1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ -1, 1 }, -1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ 1, -1 }, -1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ -1, -1 }, -1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
}

void queenMoves(bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties)
{
	rookMoves(boardMovesAllowed, myPiece, piecePos, boardState, pieceTypeProperties);
	bishopMoves(boardMovesAllowed, myPiece, piecePos, boardState, pieceTypeProperties);
}

void kingMoves(bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties)
{
	extendMove(myPiece, piecePos, Vec2{ 1, 0 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ -1, 0 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ 0, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ 0, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));

	extendMove(myPiece, piecePos, Vec2{ 1, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ -1, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ 1, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
	extendMove(myPiece, piecePos, Vec2{ -1, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_TAKING_ENEMY | MRF_MOVING_ON_BLANKS));
}

void pawnMoves(bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties)
{
	float dir;
	int amount = 1;
	if (myPiece.colour == PC_WHITE) 
	{ 
		dir = -1; 
		if (piecePos.y == 6)
		{
			amount = 2;
		}
	} 
	else 
	{ 
		dir = 1; 
		if (piecePos.y == 1)
		{
			amount = 2;
		}
	}
	
	extendMove(myPiece, piecePos, Vec2{ 0, dir }, amount, boardMovesAllowed, boardState, pieceTypeProperties, MRF_MOVING_ON_BLANKS);

	extendMove(myPiece, piecePos, Vec2{ 1, dir }, 1, boardMovesAllowed, boardState, pieceTypeProperties, MRF_TAKING_ENEMY);
	extendMove(myPiece, piecePos, Vec2{ -1, dir }, 1, boardMovesAllowed, boardState, pieceTypeProperties, MRF_TAKING_ENEMY);
}

void spiritMoves(bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties)
{
	//can move diagonally one square
	extendMove(myPiece, piecePos, Vec2{ 1, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
	extendMove(myPiece, piecePos, Vec2{ -1, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
	extendMove(myPiece, piecePos, Vec2{ 1, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
	extendMove(myPiece, piecePos, Vec2{ -1, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
}

void rockMoves(bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties)
{
	//moves 1 space in any direction
	extendMove(myPiece, piecePos, Vec2{ 1, 0 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
	extendMove(myPiece, piecePos, Vec2{ -1, 0 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
	extendMove(myPiece, piecePos, Vec2{ 0, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
	extendMove(myPiece, piecePos, Vec2{ 0, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));

	extendMove(myPiece, piecePos, Vec2{ 1, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
	extendMove(myPiece, piecePos, Vec2{ -1, 1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
	extendMove(myPiece, piecePos, Vec2{ 1, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
	extendMove(myPiece, piecePos, Vec2{ -1, -1 }, 1, boardMovesAllowed, boardState, pieceTypeProperties, (MoveRuleFlags)(MRF_MOVING_ON_BLANKS | MRF_TAKING_ENEMY));
}






























//*

//update based on phase 
//SHOULD GET BACK TO THIS WHEN I FEEL READY, I HAVE MADE CHANGES SINCE, SO I SHOULD REDO THIS BUT KEEP THE SAME IDEA
#if 0
//should i have "record what the player is doing and then act on it (in terms of actions in the game)"
//or should it be "record what the player is clicking on and then calculate what they are doing based on that (in terms of if they are clicking on the board or in the hand)"

//i think it should just be "clicking on square" and "clicking in hand" as this gives the best readability without having to redefine local variables twice, maybe i'll try both and see

//the more i think about it the more i want something that says "overHand" or "overBoard" and then something that says "handPieceUnderMouse" and swap "mouseOverPiece" to "pieceOverMouse" (or "boardpieceOverMouse")
//and then after i have all of this data about the inputs, i can work with the stuff that has to check the state of the game, decide what it is doing based on the state, and then act on that

//get all of the things the player can do
enum UserAction
{
	UA_BLANK,
	UA_PICKING_UP,
	UA_PUTTING_DOWN,
	UA_PUTTING_BACK
};

UserAction userAction = UA_BLANK;

switch (turnPhase)
{
case (HAND_PHASE) :
{
	if (gameInput.mouseButton.leftClicked && heldPiece.type != PT_BLANK && isMouseInsideBoard) //should check to make sure pices can't be placed in direct attack zone
	{
		if (mouseOverPiece.type == PT_BLANK)
		{
			if (pieceTypeProperties.canBePlacedFromHandToBlank[heldPiece.type])
			{
				userAction = UA_PUTTING_DOWN;
			}
		}
		else if (mouseOverPiece.colour == heldPiece.colour && pieceTypeProperties.canSquashFriendlyFromHand[heldPiece.type])
		{
			userAction = UA_PUTTING_DOWN;
		}
		else if (mouseOverPiece.colour != heldPiece.colour && pieceTypeProperties.canSquashEnemyFromHand[heldPiece.type])
		{
			userAction = UA_PUTTING_DOWN;
		}
	}
	break;
}
case (MOVE_PHASE) :
{
	if (gameInput.mouseButton.leftClicked)
	{
		if (heldPiece.type != PT_BLANK)
		{
			if (pointInsideRect(gameInput.mousePos, boardStart, boardSquares * squareSize))
			{
				if (shadowPos == mouseSquare)
				{
					userAction = UA_PUTTING_BACK;
				}
				else if (boardMovesAllowed[(int)mouseSquare.y][(int)mouseSquare.x] == true)
				{
					userAction = UA_PUTTING_DOWN;
				}
			}
		}
		else
		{
			if ((mouseOverPiece.type != PT_BLANK) && (mouseOverPiece.colour == gameTurn) && (mouseOverPiece.cooldown == 0))
			{
				userAction = UA_PICKING_UP;
			}
		}
	}
	break;
}
}

//act based on all the things the player can do
switch (turnPhase)
{
case (DRAW_PHASE) :
{
	break;
}
case (HAND_PHASE) :
{
	switch (userAction)
	{
	case (UA_PICKING_UP) :
	{

		break;
	}
	case (UA_PUTTING_DOWN) :
	{
		mouseOverPiece = heldPiece;
		heldPiece = {};
		mouseOverPiece.cooldown = pieceTypeProperties.maxCooldown[mouseOverPiece.type];
		turnPhase = MOVE_PHASE;
		break;
	}
	case (UA_PUTTING_BACK) :
	{

		break;
	}
	}
	break;
}
case (MOVE_PHASE) :
{
	switch (userAction)
	{
	case (UA_PUTTING_BACK) :
	{
		mouseOverPiece = heldPiece;
		heldPiece = {};
		break;
	}
	case (UA_PICKING_UP) :
	{
		heldPiece = mouseOverPiece;
		shadowPos = mouseSquare;
		mouseOverPiece = {};
		break;
	}
	case (UA_PUTTING_DOWN) :
	{
		if (((gameTurn == GT_WHITE && mouseSquare.y == 0) || (gameTurn == GT_BLACK && mouseSquare.y == (BOARD_SQUARES_DOWN - 1))) && pieceTypeProperties.canDirectAttack[heldPiece.type])
		{ //direct attack
			heldPiece = {};
			mainState.health[!gameTurn] -= 1;
		}
		else
		{
			mouseOverPiece = heldPiece;
			heldPiece = {};
		}

		gameTurn = (GameTurn)(!gameTurn);
		turnPhase = DRAW_PHASE;
		break;
	}
	}
	break;
}
}
#endif
