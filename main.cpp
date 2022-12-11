#include <cassert>
#include <cmath>
#include <iostream>
#include <array>
#include <optional>
#include <string>
#include <sstream>
#include <concepts>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <memory>

enum class BoxContent {
    Empty,
    S,
    O
};

enum class Player {
    A,
    B
};

constexpr Player otherPlayer(Player player) {
    switch(player) {
        case Player::A:
            return Player::B;
        case Player::B:
            return Player::A;
        default:
            throw std::runtime_error("Illegal player.");
    }
}

constexpr Player firstPlayer = Player::A;

template <size_t N>
struct BoxLine {
    using const_iterator = typename std::array<BoxContent, N>::const_iterator;
    static constexpr size_t numBoxes = N;
    
    BoxLine();
    BoxLine(const BoxLine&) = default;
    BoxLine(BoxLine&&) = default;
    BoxLine& operator=(const BoxLine&) = default;
    BoxLine& operator=(BoxLine&&) = default;
    bool showsFinishedGame() const;
    bool showsDrawnGame() const;
    size_t determineMoveNumber() const;
    Player determineNextPlayer() const;
    std::optional<Player> determineWinner() const;
    BoxLine<N> writeAt(BoxContent content, size_t index) const;

    BoxContent operator[](size_t index);
    const BoxContent& operator[](size_t index) const;
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    
    private:
    std::array<BoxContent, N> boxContents;
    
    BoxLine(const std::array<BoxContent, N>&);
};

template <size_t N>
BoxLine<N>::BoxLine() {
    boxContents.fill(BoxContent::Empty);
}

template <size_t N>
BoxLine<N>::BoxLine(const std::array<BoxContent, N>& contents)
    : boxContents(contents) {}
    
template <size_t N>
BoxContent BoxLine<N>::operator[](size_t index) {
    return boxContents[index];
}

template <size_t N>
BoxLine<N> BoxLine<N>::writeAt(BoxContent content, size_t index) const {
    assert(content != BoxContent::Empty);
    BoxLine<N> result(boxContents);
    result.boxContents[index] = content;
    return result;
}

template <size_t N>
typename BoxLine<N>::const_iterator BoxLine<N>::begin() const {
    return boxContents.begin();
}

template <size_t N>
typename BoxLine<N>::const_iterator BoxLine<N>::end() const {
    return boxContents.end();
}

template <size_t N>
typename BoxLine<N>::const_iterator BoxLine<N>::cbegin() const {
    return boxContents.cbegin();
}

template <size_t N>
typename BoxLine<N>::const_iterator BoxLine<N>::cend() const {
    return boxContents.cend();
}

template <size_t N>
std::optional<Player> BoxLine<N>::determineWinner() const {
    std::vector<BoxContent> sosSequence {BoxContent::S, BoxContent::O, BoxContent::S};
    auto searchResult = std::ranges::search(boxContents, sosSequence);
    std::optional<Player> result;
    if(!searchResult.empty()) {
        return otherPlayer(determineNextPlayer());
    } else {
        return std::nullopt;
    }
}

template <size_t N>
bool BoxLine<N>::showsFinishedGame() const {
    return showsDrawnGame() || determineWinner().has_value();
        
}

template <size_t N>
bool BoxLine<N>::showsDrawnGame() const {
    return determineMoveNumber() == N;
}

template <size_t N>
size_t BoxLine<N>::determineMoveNumber() const {
    return N - std::ranges::count(boxContents, BoxContent::Empty);
}

template <size_t N>
Player BoxLine<N>::determineNextPlayer() const {
    return determineMoveNumber() % 2 == 0 ? firstPlayer : otherPlayer(firstPlayer);
}

template <size_t N>
std::string boxLineToString(const BoxLine<N>& boxLine) {
    std::stringstream stream;
    for(const auto& boxContent : boxLine) {
        switch(boxContent) {
            case BoxContent::Empty:
                stream.put('_');
                break;
            case BoxContent::S:
                stream.put('S');
                break;
            case BoxContent::O:
                stream.put('O');
                break;
        }
    }
    return stream.str();
}

template <std::unsigned_integral I, std::unsigned_integral J>
constexpr I pow(I base, J exp) {
    I result = 1;
    while(exp > 0) {
        result *= base;
        --exp;
    }
    return result;
}

struct Move {
    BoxContent letter;
    size_t index;
};

constexpr Move terminalMove = {BoxContent::Empty, std::numeric_limits<size_t>::max()};
constexpr bool isTerminal(const Move& move) {
    return move.letter == BoxContent::Empty;
}

struct EvaluatedMove {
    Move move;
    float evaluation;
};
 
auto operator==(const EvaluatedMove& lhs, const EvaluatedMove& rhs) {
    return lhs.evaluation == rhs.evaluation;
}

auto operator<=>(const EvaluatedMove& lhs, const EvaluatedMove& rhs) {
    return lhs.evaluation <=> rhs.evaluation;
}

template <size_t N>
BoxLine<N> applyMove(Move move, const BoxLine<N>& boxLine) {
    return boxLine.writeAt(move.letter, move.index);
}

template <size_t N>
struct Evaluator {
    Evaluator(Player player) : player(player) {
        evaluations.fill(std::numeric_limits<typename EvaluationTable::value_type>::quiet_NaN());
    }
    
    float evaluatePosition(const BoxLine<N>& boxLine) {
        return findNextMove(boxLine).evaluation;
    }
    
    EvaluatedMove findNextMove(const BoxLine<N>& boxLine) {
        auto index = calculateEvaluationTableIndex(boxLine);
        float tableValue = evaluations[index];
        if(!std::isnan(tableValue))
            return {bestMoves[index], tableValue};
        
        EvaluatedMove evaluatedMove = findNextMoveWithoutCache(boxLine);
        evaluations[index] = evaluatedMove.evaluation - 0.01f * evaluatedMove.evaluation;
        bestMoves[index] = evaluatedMove.move;
        return evaluatedMove;
    }

    private:
    EvaluatedMove findNextMoveWithoutCache(const BoxLine<N>& boxLine) {
        if(boxLine.showsFinishedGame()) {
            return {terminalMove, evaluateFinishedGame(boxLine)};
        } else {
            auto evaluations = evaluateAllMoves(boxLine);
            if(boxLine.determineNextPlayer() == player)
                return std::ranges::max(evaluations);
            else
                return std::ranges::min(evaluations);
        }
    }

    float evaluateFinishedGame(const BoxLine<N>& boxLine) const {
        auto winner = boxLine.determineWinner();
        if(winner)
            if(*winner == player)
                return 1.0f;
            else
                return -1.0f;
        else
            return 0.0f;
    }

    size_t calculateEvaluationTableIndex(const BoxLine<N>& line) const {
        size_t result = 0;
        size_t index = 0;
        for(const BoxContent& content : line) {
            size_t value;
            switch(content) {
                case BoxContent::Empty:
                    value = 0;
                    break;
                case BoxContent::S:
                    value = 1;
                    break;
                case BoxContent::O:
                    value = 2;
                    break;
                default:
                    throw std::runtime_error("Box line has illegal content.");
            }
            result += pow(3, index) * value;
            ++index;
        }
        return result;
    }
    
    std::vector<EvaluatedMove> evaluateAllMoves(const BoxLine<N>& boxLine) {
        auto moves = calculatePossibleMoves(boxLine);
        std::vector<EvaluatedMove> evaluatedMoves(moves.size());
        std::ranges::transform(moves, evaluatedMoves.begin(),
                               [&boxLine, this](const auto& move) -> EvaluatedMove {
                               return {move, evaluatePosition(applyMove(move, boxLine))};
                               });
        return evaluatedMoves;
    }
    
    std::vector<Move> calculatePossibleMoves(const BoxLine<N>& boxLine) {
        std::vector<Move> moves;
        for (auto it = boxLine.cbegin(); it != boxLine.cend(); ++it) {
            if(*it == BoxContent::Empty) {
                moves.push_back({BoxContent::S, (size_t)std::distance(boxLine.cbegin(), it)});
                moves.push_back({BoxContent::O, (size_t)std::distance(boxLine.cbegin(), it)});
            }
        }
        return moves;
    }

    using EvaluationTable = std::array<float, pow(3u, N)>;
    using BestMoveTable = std::array<Move, pow(3u, N)>;
    EvaluationTable evaluations;
    BestMoveTable bestMoves;
    Player player;
};

template <size_t N>
void printEvaluation(const BoxLine<N> boxLine, Evaluator<N>& e) {
    std::cout << boxLineToString(boxLine) << ": " << e.evaluatePosition(boxLine) << "\n";
}

template <size_t N>
void printNextMoveSequence(const BoxLine<N>& boxLine, Evaluator<N>& e,
                           size_t length = std::numeric_limits<size_t>::max()) {
    BoxLine<N> currentLine = boxLine;
    size_t moveNumber = currentLine.determineMoveNumber();
    for(size_t i = 0; i < length && !currentLine.showsFinishedGame(); ++i) {
        auto evaluatedMove = e.findNextMove(currentLine);
        currentLine = applyMove(evaluatedMove.move, currentLine);
        std::cout << ++moveNumber << ".\t" << boxLineToString(currentLine)
            << ": " << evaluatedMove.evaluation << "\n";
    }
}

int main() {
    constexpr size_t N = 16;

    std::unique_ptr<Evaluator<N>> e = std::make_unique<Evaluator<N>>(otherPlayer(firstPlayer));
    BoxLine<N> boxLine;
    std::cout << "\n"
        << "Evaluation for the second player\n"
        << "> 0 is winning, < 0 is losing\n"
        << "\n";
    printEvaluation(boxLine, *e);
    printNextMoveSequence(boxLine, *e);
}

