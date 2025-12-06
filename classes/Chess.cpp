#include "Chess.h"
#include <limits>
#include <cmath>
#include <random>

#include "MagicBitboards.h"

static std::array<BitBoard, 64> generateBitboards(BitBoard (*f)(int)) {
    std::array<BitBoard, 64> boards;
    for (int i = 0; i < 64; i++) {
        boards[i] = f(i);
    }

    return boards;
}

Chess::Chess() {
    _grid            = new Grid(8, 8);
    _knightBitboards = generateBitboards(+[](int square) {
        BitBoard bitboard = 0ULL;
        int      rank     = square / 8;
        int      file     = square % 8;

        std::pair<int, int> offsets[8] = {
            {-2, -1}, {2, -1}, {-2, 1}, {2, 1},
            {-1, -2}, {1, -2}, {-1, 2}, {1, 2}
        };
        bitboard |= 1ULL << square;

        for (const auto& [ro, fo] : offsets) {
            int r = rank + ro;
            int f = file + fo;

            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                bitboard |= 1ULL << (r * 8 + f);
            }
        }

        return bitboard;
    });

    _kingBitboards = generateBitboards(+[](int square) {
        BitBoard            bitboard   = 0ULL;
        int                 rank       = square / 8;
        int                 file       = square % 8;
        std::pair<int, int> offsets[8] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            {0, -1}, {0, 1},
            {1, -1}, {1, 0}, {1, 1},
        };

        for (const auto& [ro, fo] : offsets) {
            int r = rank + ro;
            int f = file + fo;

            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                bitboard |= 1ULL << (r * 8 + f);
            }
        }

        return bitboard;
    });

    initMagicBitboards();
}

Chess::~Chess() {
    delete _grid;
    cleanupMagicBitboards();
}

char Chess::pieceNotation(int x, int y) const {
    const char* wpieces  = {"0PNBRQK"};
    const char* bpieces  = {"0pnbrqk"};
    Bit*        bit      = _grid->getSquare(x, y)->bit();
    char        notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag() - 128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece) {
    const char* pieces[] = {"pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png"};

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName  = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);
    bit->setGameTag((playerNumber << 7) | (piece & 0x7));

    return bit;
}

void Chess::setUpBoard() {
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    // FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    setAIPlayer(AI_PLAYER);

    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)

    const auto  end_of_placement = fen.find(' ');
    std::string placement;
    if (end_of_placement != std::string::npos) {
        placement = fen.substr(0, end_of_placement);
    }
    else {
        placement = fen;
    }

    int row     = 0;
    int row_end = -1;

    do {
        placement = placement.substr(row_end + 1);
        row_end   = placement.find('/');
        int col   = 0;
        for (const char c : placement.substr(0, row_end)) {
            switch (c) {
            case 'r': setPieceAt(0, Rook, col, row);
                ++col;
                break;
            case 'R': setPieceAt(1, Rook, col, row);
                ++col;
                break;
            case 'p': setPieceAt(0, Pawn, col, row);
                ++col;
                break;
            case 'P': setPieceAt(1, Pawn, col, row);
                ++col;
                break;
            case 'n': setPieceAt(0, Knight, col, row);
                ++col;
                break;
            case 'N': setPieceAt(1, Knight, col, row);
                ++col;
                break;
            case 'b': setPieceAt(0, Bishop, col, row);
                ++col;
                break;
            case 'B': setPieceAt(1, Bishop, col, row);
                ++col;
                break;
            case 'q': setPieceAt(0, Queen, col, row);
                ++col;
                break;
            case 'Q': setPieceAt(1, Queen, col, row);
                ++col;
                break;
            case 'k': setPieceAt(0, King, col, row);
                ++col;
                break;
            case 'K': setPieceAt(1, King, col, row);
                ++col;
                break;
            default: {
                if (c > '0' && c <= '8') {
                    col += c - '0';
                }
            }
            break;
            }
        }
        ++row;
    }
    while (row_end != std::string::npos);
}

bool Chess::actionForEmptyHolder(BitHolder& holder) {
    return false;
}

bool Chess::canBitMoveFrom(Bit& bit, BitHolder& src) {
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor    = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) {
    if (dst.bit() && !((dst.bit()->gameTag() ^ bit.gameTag()) >> 7)) {
        return false;
    }

    const auto movingPiece = static_cast<ChessPiece>(bit.gameTag() & 0x7);
    auto*      csquare     = static_cast<ChessSquare*>(&src);
    auto*      dsquare     = static_cast<ChessSquare*>(&dst);

    const int square = csquare->getSquareIndex();
    const int dsqr   = dsquare->getSquareIndex();

    auto allowedMoves = generateAllMoves();
    for (const auto& move : allowedMoves) {
        if (move.from == square && move.to == dsqr) {
            return true;
        }
    }

    // switch (movingPiece) {
    // case NoPiece:
    //     return false;
    // case King:
    //     return _kingBitboards[square].testSquare(dsqr);
    // case Pawn:
    //     if (bit.gameTag() & 128) {
    //         // black
    //         return dsqr == (square - 8) || (square >= 48 && square < 56 && dsqr == (square - 16));
    //     }
    //     return dsqr == (square + 8) || (square >= 8 && square < 16 && dsqr == (square + 16));
    // case Knight:
    //     return _knightBitboards[square].testSquare(dsqr);
    // case Bishop:
    //     return false;
    // case Rook:
    //     return false;
    // case Queen:
    //     return false;
    // }
    return false;
}

void Chess::stopGame() {
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner() {
    return nullptr;
}

bool Chess::checkForDraw() {
    return false;
}

std::string Chess::initialStateString() {
    return stateString();
}

std::string Chess::stateString() {
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation(x, y);
        }
    );
    return s;
}

void Chess::setStateString(const std::string& s) {
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int  index        = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        }
        else {
            square->setBit(nullptr);
        }
    });
}

void Chess::setPieceAt(const int playerNumber, const ChessPiece piece, const int x, const int y) {
    ChessSquare* holder = _grid->getSquare(x, y);
    if (!holder) {
        return;
    }

    Bit* bit = PieceForPlayer(playerNumber, piece);
    bit->setPosition(holder->getPosition());
    holder->setBit(bit);
}

void Chess::generateKnightMoves(std::vector<BitMove>& moves, const BitBoard knightBoard,
                                const uint64_t        emptySquares) const {
    knightBoard.forEachBit([&](int square) {
        const BitBoard moveBitboard = _knightBitboards[square].getData() & emptySquares;
        moveBitboard.forEachBit([&](int to) {
            moves.emplace_back(square, to, Knight);
        });
    });
}

void Chess::generateKingMoves(std::vector<BitMove>& moves, const BitBoard kingBoard,
                              const uint64_t        emptySquares) const {
    kingBoard.forEachBit([&](int square) {
        const BitBoard moveBitboard = _kingBitboards[square].getData() & emptySquares;
        moveBitboard.forEachBit([&](int to) {
            moves.emplace_back(square, to, King);
        });
    });
}

void Chess::generatePawnMoves(std::vector<BitMove>& moves, const BitBoard   pawnBoard, const uint64_t emptySquares,
                              const uint64_t        enemySquares, const int playerNumber) {
    BitBoard           singleMoves, doubleMoves, capturesLeft, capturesRight;
    int                singleShift, doubleShift, leftShift,    rightShift;
    constexpr uint64_t FL = 0xfefefefefefefeULL;
    constexpr uint64_t FR = 0x7f7f7f7f7f7f7fULL;

    if (playerNumber == 0) {
        constexpr uint64_t R3 = 0xff0000;
        singleShift           = 8;
        doubleShift           = 16;
        leftShift             = 7;
        rightShift            = 9;

        singleMoves   = (pawnBoard << 8) & emptySquares;
        doubleMoves   = ((singleMoves & R3) << 8) & emptySquares;
        capturesLeft  = ((pawnBoard & FL) << 7) & enemySquares;
        capturesRight = ((pawnBoard & FR) << 9) & enemySquares;
    }
    else {
        constexpr uint64_t R6 = 0x0000ff00'00000000;
        singleShift           = -8;
        doubleShift           = -16;
        leftShift             = -9;
        rightShift            = -7;

        singleMoves   = (pawnBoard >> 8) & emptySquares;
        doubleMoves   = ((singleMoves & R6) >> 8) & emptySquares;
        capturesLeft  = ((pawnBoard & FL) >> 9) & enemySquares;
        capturesRight = ((pawnBoard & FR) >> 7) & enemySquares;
    }

    addPawnMoves(moves, singleMoves, singleShift);
    addPawnMoves(moves, doubleMoves, doubleShift);
    addPawnMoves(moves, capturesLeft, leftShift);
    addPawnMoves(moves, capturesRight, rightShift);
}

void Chess::addPawnMoves(std::vector<BitMove>& moves, const BitBoard movesBoard, const int shift) {
    movesBoard.forEachBit([&](int square) {
        int from = square - shift;
        moves.emplace_back(from, square, Pawn);
    });
}

BitBoards Chess::calculateTurnBoards() const {
    BitBoards boards{};

    uint64_t nextbit = 1;
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const uint64_t setbit = nextbit;
            nextbit <<= 1;
            Bit* bit = _grid->getSquare(file, rank)->bit();
            if (!bit) {
                continue;
            }

            const int colorIndex = (bit->gameTag() & 128) >> 4;
            BitBoard& board      = boards.bitboards[colorIndex | (bit->gameTag() & 7)];

            board |= setbit;
        }
    }

    boards.bitboards[BitBoardIndex::WhiteAll] = boards.bitboards[BitBoardIndex::WhiteBishop] | boards.bitboards[
                                                    BitBoardIndex::WhiteKing] | boards.bitboards[
                                                    BitBoardIndex::WhiteKnight] | boards.bitboards[
                                                    BitBoardIndex::WhitePawn] | boards.bitboards[
                                                    BitBoardIndex::WhiteQueen] | boards.bitboards[
                                                    BitBoardIndex::WhiteRook];

    boards.bitboards[BitBoardIndex::BlackAll] = boards.bitboards[BitBoardIndex::BlackBishop] | boards.bitboards[
                                                    BitBoardIndex::BlackKing] | boards.bitboards[
                                                    BitBoardIndex::BlackKnight] | boards.bitboards[
                                                    BitBoardIndex::BlackPawn] | boards.bitboards[
                                                    BitBoardIndex::BlackQueen] | boards.bitboards[
                                                    BitBoardIndex::BlackRook];
    boards.bitboards[BitBoardIndex::Occupied] = boards.bitboards[BitBoardIndex::WhiteAll] | boards.bitboards[
                                                    BitBoardIndex::BlackAll];

    return boards;
}

std::vector<BitMove> Chess::generateAllMoves() {
    auto boards = calculateTurnBoards();

    std::vector<BitMove> moves;
    const auto           colorBit = (getCurrentPlayer()->playerNumber()) << 3; // calculate color bit from player number

    const BitBoard emptySquares = ~boards[BitBoardIndex::Occupied];
    const BitBoard enemySquares = boards[BitBoardIndex::FlipIndex(colorBit | BitBoardIndex::Combined)];
    const BitBoard targetableSquares = emptySquares | enemySquares;

    generateKingMoves(moves, boards[King | colorBit], emptySquares | enemySquares);
    generateKnightMoves(moves, boards[Knight | colorBit], emptySquares | enemySquares);
    generatePawnMoves(moves, boards[Pawn | colorBit], emptySquares, enemySquares, getCurrentPlayer()->playerNumber());

    boards[Queen | colorBit].forEachBit([&](const int square) {
        const BitBoard queenAttacks = getQueenAttacks(square, boards[BitBoardIndex::Occupied]) & targetableSquares;
        queenAttacks.forEachBit([&](const int toSquare) {
            moves.emplace_back(square, toSquare, Queen);
        });
    });

    boards[Rook | colorBit].forEachBit([&](const int square) {
        const BitBoard rookAttacks = getRookAttacks(square, boards[BitBoardIndex::Occupied]) & targetableSquares;
        rookAttacks.forEachBit([&](const int toSquare) {
            moves.emplace_back(square, toSquare, Rook);
        });
    });

    boards[Bishop | colorBit].forEachBit([&](const int square) {
        const BitBoard bishopAttacks = getBishopAttacks(square, boards[BitBoardIndex::Occupied]) & targetableSquares;
        bishopAttacks.forEachBit([&](const int toSquare) {
            moves.emplace_back(square, toSquare, Bishop);
        });
    });

    return moves;
}

void Chess::makeMove(const BitMove& move) {
    ChessSquare* from = _grid->getSquareByIndex(move.from);
    ChessSquare* to   = _grid->getSquareByIndex(move.to);
    if (!from || !to) {
        return;
    }
    Bit* bit = from->bit();
    if (!bit) { return; }

    if (Bit* take = to->bit()) {
        pieceTaken(take);
    }

    to->dropBitAtPoint(bit, bit->getPosition());
    from->draggedBitTo(bit, to);
    bitMovedFromTo(*bit, *from, *to);
}

void Chess::updateAI() {
    if (!gameHasAI()) return;

    const auto allowedMoves = generateAllMoves();
    if (allowedMoves.empty()) {
        // uh we lose?
        return;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, allowedMoves.size() - 1);
    makeMove(allowedMoves[dis(gen)]);
}

bool Chess::gameHasAI() {
    return true;
}
