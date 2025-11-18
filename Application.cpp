#include "Application.h"
#include "imgui/imgui.h"
#include "classes/TicTacToe.h"
#include "classes/Checkers.h"
#include "classes/Othello.h"
#include "classes/Connect4.h"
#include "classes/Chess.h"
#include <random>

namespace ClassGame {
    //
    // our global variables
    //
    Game* game       = nullptr;
    bool  gameOver   = false;
    int   gameWinner = -1;

    //
    // game starting point
    // this is called by the main render loop in main.cpp
    //
    void GameStartUp() {
        game = nullptr;
    }

    //
    // game render loop
    // this is called by the main render loop in main.cpp
    //
    void RenderGame() {
        ImGui::DockSpaceOverViewport();

        //ImGui::ShowDemoWindow();

        ImGui::Begin("Settings");

        if (gameOver) {
            ImGui::Text("Game Over!");
            ImGui::Text("Winner: %d", gameWinner);
            if (ImGui::Button("Reset Game")) {
                game->stopGame();
                game->setUpBoard();
                gameOver   = false;
                gameWinner = -1;
            }
        }
        if (!game) {
            if (ImGui::Button("Start Tic-Tac-Toe")) {
                game = new TicTacToe();
                game->setUpBoard();
            }
            if (ImGui::Button("Start Checkers")) {
                game = new Checkers();
                game->setUpBoard();
            }
            if (ImGui::Button("Start Othello")) {
                game = new Othello();
                game->setUpBoard();
            }
            if (ImGui::Button("Start Connect 4")) {
                game = new Connect4();
                game->setUpBoard();
            }
            if (ImGui::Button("Start Chess")) {
                game = new Chess();
                game->setUpBoard();
            }
        }
        else {
            ImGui::Text("Current Player Number: %d", game->getCurrentPlayer()->playerNumber());
            std::string stateString = game->stateString();
            int         stride      = game->_gameOptions.rowX;
            int         height      = game->_gameOptions.rowY;

            for (int y = 0; y < height; y++) {
                ImGui::Text("%s", stateString.substr(y * stride, stride).c_str());
            }
            ImGui::Text("Current Board State: %s", game->stateString().c_str());
        }
        ImGui::End();

        ImGui::Begin("GameWindow");
        if (game) {
            if (game->gameHasAI() && (game->getCurrentPlayer()->isAIPlayer() || game->_gameOptions.AIvsAI)) {
                game->updateAI();
            }
            game->drawFrame();
        }
        ImGui::End();

        ImGui::Begin("Possible Moves");
        if (auto* chess = dynamic_cast<Chess*>(game)) {
            auto moves = chess->generateAllMoves();
            if (moves.empty()) ImGui::BeginDisabled(true);
            if (ImGui::Button("Play Random Move")) {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, moves.size() - 1);

                chess->makeMove(moves[dis(gen)]);
            }
            if (moves.empty()) ImGui::EndDisabled();
            ImGui::Text("There are %d possible moves", (uint32_t)(moves.size()));
            ImGui::Separator();
            for (const auto& move : moves) {
                int fx = move.from % 8, fy = move.from / 8, tx = move.to % 8, ty = move.to / 8;
                constexpr const char* pieceNames[] = {"", "pawn", "knight", "bishop", "rook", "queen", "king"};
                constexpr char r[] = {'1','2','3','4','5','6','7','8'};
                constexpr char f[] = {'a','b','c','d','e','f','g','h'};
                ImGui::Text("%c%c -> %c%c ; %s", f[fx], r[fy], f[tx], r[ty], pieceNames[move.piece]);
            }
        }
        ImGui::End();
    }

    //
    // end turn is called by the game code at the end of each turn
    // this is where we check for a winner
    //
    void EndOfTurn() {
        Player* winner = game->checkForWinner();
        if (winner) {
            gameOver   = true;
            gameWinner = winner->playerNumber();
        }
        if (game->checkForDraw()) {
            gameOver   = true;
            gameWinner = -1;
        }
    }
}
