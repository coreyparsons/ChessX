#ifndef MAIN_GAME_H
#define MAIN_GAME_H

#define BOARD_SQUARES_ACROSS 8
#define BOARD_SQUARES_DOWN 8

#define SMALL_TEXT 64
#define BIG_TEXT 512

enum TextureName;

enum PieceType
{
	PT_BLANK,

	PT_PAWN,
	PT_ROOK,
	PT_KNIGHT,
	PT_BISHOP,
	PT_QUEEN,
	PT_KING,
	PT_SPIRIT,
	PT_ROCK,	

	PT_SIZE
};

enum PieceColour //should merge this with game turn to make GameColour
{
	PC_BLACK,
	PC_WHITE,
	PC_SIZE
};

struct Piece
{
	PieceType type;
	PieceColour colour;
	int cooldown;
};

enum GameTurn
{
	GT_BLACK,
	GT_WHITE,
	GT_SIZE
};

struct PieceTypeProperties
{

	bool canBeTaken[PT_SIZE] = {};
	bool canDirectAttack[PT_SIZE] = {};
	bool canSquashFriendlyFromHand[PT_SIZE] = {};
	bool canSquashEnemyFromHand[PT_SIZE] = {}; //can squash any enimies from the hand that could usually be taken
	bool canBeSquashedByFriendly[PT_SIZE] = {};
	bool canBeSquashedByEnimy[PT_SIZE] = {};
	bool canBePlacedFromHandToBlank[PT_SIZE] = {}; //means that this piece has the normal rules for where it can be spawned

	bool canProduce[PT_SIZE] = {}; //producer means normal pieces have to be put around this piece (may change this to have a list of directions or something like that)
	bool canDisrupt[PT_SIZE] = {}; //means enimies have to kill this piece before they can attack the enimy
	int maxCooldown[PT_SIZE] = {};
};

enum TurnPhase
{
	DRAW_PHASE,
	HAND_PHASE,
	MOVE_PHASE
};

#define MAX_HAND_SIZE 20

struct MainState
{
	//things about the board
	Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS] = {};
	Piece heldPiece = {};
	Vec2 shadowPos = {};
	PieceTypeProperties pieceTypeProperties = {};

	//things about the hands
	Piece hands[PC_SIZE][MAX_HAND_SIZE] = {};

	//things about the game
	GameTurn gameTurn;
	TurnPhase turnPhase;
	int health[PC_SIZE];

	bool init = false;

	//debug things
	Piece selectedPiece = {};
	bool debugMenuShowing = false;
};

enum MoveRuleFlags
{
	MRF_BLANK = 0,
	MRF_JUMPING = 1, //make jumping_friendly and jumping_enemey
	MRF_TAKING_ENEMY = 1 << 1,
	MRF_MOVING_ON_BLANKS = 1 << 2,
	MRF_TAKING_FRIENDLY = 1 << 3,
};

struct GameInput; struct GameState; struct Assets; struct SDL_Texture;

void mainUpdateAndRender(GameInput gameInput, float fps, GameState &gameState);
void makeBoardHighlights(bool board[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Colour blankSquareColour, Colour pieceSquareColour, Piece heldPiece, TurnPhase turnPhase, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Vec2 boardStart, Vec2 squareSize);
float relative(PieceColour pieceColour, float valueIfWhite);
char toChar(int num);
void toCharList(char(&text)[SMALL_TEXT], int num);
void concat(char(&start)[SMALL_TEXT], char* end);
void concat(char(&start)[BIG_TEXT], char* end);
int getHandSize(Piece hand[MAX_HAND_SIZE]);
void bunchHand(Piece hand[MAX_HAND_SIZE]);
bool pointInsideRect(Vec2 point, Vec2 rectPos, Vec2 rectSize);
bool imgButton(GameInput gameInput, SDL_Texture* texture, Vec2 pos, Vec2 size);
void calculateSpawns(bool(&spawnsAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS]);
void checkMoves(Piece piece, Vec2 piecePos, bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties);
bool changeSquare(Piece myPiece, Vec2 &checkingSpace, bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties, MoveRuleFlags flags);
void extendMove(Piece myPiece, Vec2 start, Vec2 dir, int amount, bool(&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties, MoveRuleFlags flags);

void rookMoves(bool (&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceTypeProperties);
void knightMoves(bool (&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceProperites);
void bishopMoves(bool (&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceProperites);
void queenMoves(bool (&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceProperites);
void kingMoves(bool (&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceProperites);
void pawnMoves(bool (&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceProperites);
void spiritMoves(bool (&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceProperites);
void rockMoves(bool (&boardMovesAllowed)[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], Piece myPiece, Vec2 piecePos, Piece boardState[BOARD_SQUARES_DOWN][BOARD_SQUARES_ACROSS], PieceTypeProperties pieceProperites);

#endif