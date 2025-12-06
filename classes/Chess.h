#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
#include <array>

constexpr int pieceSize = 80;

namespace BitBoardIndex {
    enum Index_ : uint8_t {
        White = 0b0000,
        Black = 0b1000,

        Combined = 0b111,

        WhiteUnused = White,
        WhiteRook   = White | Rook,
        WhiteKnight = White | Knight,
        WhiteBishop = White | Bishop,
        WhiteQueen  = White | Queen,
        WhitePawn   = White | Pawn,
        WhiteKing   = White | King,
        WhiteAll    = White | Combined,

        BlackRook   = Black | Rook,
        BlackKnight = Black | Knight,
        BlackBishop = Black | Bishop,
        BlackQueen  = Black | Queen,
        BlackPawn   = Black | Pawn,
        BlackKing   = Black | King,
        BlackAll    = Black | Combined,

        Occupied = 0b10000,
    };

    static constexpr Index_ FlipIndex(const uint8_t index) {
        return static_cast<Index_>(index ^ 0b1000);
    };
}

struct BitBoards {
    BitBoard bitboards[17];

    inline constexpr BitBoard& operator[](const uint8_t index) {
        return bitboards[index];
    }
};

struct BitBoardSet {
    BitBoard knight, king, queen, pawn, rook, bishop;
};

class Chess : public Game {
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    bool actionForEmptyHolder(BitHolder& holder) override;

    void stopGame() override;

    Player* checkForWinner() override;
    bool    checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string& s) override;

    Grid* getGrid() override { return _grid; }

    std::vector<BitMove> generateAllMoves();

    void makeMove(const BitMove& move);

    void updateAI() override;
    bool gameHasAI() override;

private:
    Bit*    PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void    FENtoBoard(const std::string& fen);
    char    pieceNotation(int x, int y) const;
    void    setPieceAt(const int playerNumber, ChessPiece piece, int x, int y);

    Grid*                    _grid;
    std::array<BitBoard, 64> _knightBitboards;
    std::array<BitBoard, 64> _kingBitboards;

    void        generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t emptySquares) const;
    void        generateKingMoves(std::vector<BitMove>& moves, BitBoard kingBoard, uint64_t emptySquares) const;

    static void generatePawnMoves(std::vector<BitMove>& moves, BitBoard   pawnBoard, uint64_t emptySquares,
                                  uint64_t              enemySquares, int playerNumber);
    static void addPawnMoves(std::vector<BitMove>& moves, BitBoard movesBoard, int shift);

    BitBoards calculateTurnBoards() const;
};
