#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
#include <array>

constexpr int pieceSize = 80;

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

private:
    Bit*    PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void    FENtoBoard(const std::string& fen);
    char    pieceNotation(int x, int y) const;
    void    setPieceAt(const int playerNumber, ChessPiece piece, int x, int y);

    Grid*                    _grid;
    std::array<BitBoard, 64> _knightBitboards;
    std::array<BitBoard, 64> _kingBitboards;

    void generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t emptySquares);
    void generateKingMoves(std::vector<BitMove>& moves, BitBoard kingBoard, uint64_t emptySquares);
    void generatePawnMoves(std::vector<BitMove>& moves, BitBoard pawnBoard, uint64_t emptySquares, uint64_t enemySquares, int playerNumber);
    void addPawnMoves(std::vector<BitMove>& moves, BitBoard movesBoard, int shift);

    std::pair<BitBoardSet, BitBoardSet> calculateTurnBoards();
};
