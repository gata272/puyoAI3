#include "evaluation.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace puyo {

namespace {

double keyValue(const Board& board, const Move& move, const PuyoPair& pair) {
    // Approximation of ama's "key": how much same-color material is already
    // connected to the pair's main/sub colors around the target column.
    double value = 0.0;
    for (int dx = -1; dx <= 1; ++dx) {
        int x = move.x + dx;
        if (x < 0 || x >= BOARD_WIDTH) continue;
        for (int y = 0; y < VISIBLE_HEIGHT; ++y) {
            int c = static_cast<int>(board.get(x, y));
            if (c == pair.main || c == pair.sub) value += 1.0;
        }
    }
    return std::max(0.0, 4.0 - value * 0.25);
}

double chiValue(const Board& board, int x) {
    const auto h = board.heights();
    if (x < 0 || x >= BOARD_WIDTH) return 0.0;

    double value = 0.0;
    int base = h[x];

    for (int d = 1; d < BOARD_WIDTH; ++d) {
        int lx = x - d;
        int rx = x + d;
        if (lx >= 0) value += std::max(0, base - h[lx]);
        if (rx < BOARD_WIDTH) value += std::max(0, base - h[rx]);
    }
    return value;
}

double quietScore(
    const Board& board,
    const Weights& w,
    const std::vector<PuyoPair>& pieces,
    int depth
) {
    Features f = extractStaticFeatures(board);
    double best = -std::numeric_limits<double>::infinity();

    if (depth <= 0 || pieces.empty()) {
        return f.chain * w.chain
             + f.y * w.y
             + f.key * w.key
             + f.chi * w.chi
             + f.link2 * w.link2
             + f.link3 * w.link3;
    }

    const PuyoPair& pair = pieces.front();

    for (int rot = 0; rot < 4; ++rot) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            int y = Simulator::findDropY(board, pair, x, rot);
            if (y < 0) continue;

            Move m{x, rot, true};
            SimulationResult sim = Simulator::drop(board, pair, m);
            if (sim.gameOver && !sim.allClear) continue;

            // Quiescence search follows tactical moves first: placements
            // which immediately produce a chain are the primary candidates.
            if (sim.chains == 0 && depth == 1) continue;

            const auto h = sim.board.heights();
            double chain = sim.chains;
            double yValue = h[x];
            double key = keyValue(board, m, pair);
            double chi = chiValue(sim.board, x);

            double q =
                chain * w.chain +
                yValue * w.y +
                key * w.key +
                chi * w.chi +
                extractStaticFeatures(sim.board).link2 * w.link2 +
                extractStaticFeatures(sim.board).link3 * w.link3;

            if (depth > 1 && !pieces.empty()) {
                std::vector<PuyoPair> rest(
                    pieces.begin() + 1, pieces.end()
                );
                q += 0.85 * quietScore(sim.board, w, rest, depth - 1);
            }

            best = std::max(best, q);
        }
    }

    if (!std::isfinite(best)) {
        return f.link2 * w.link2 + f.link3 * w.link3;
    }
    return best;
}

} // namespace

double evaluate(
    const Board& board,
    const Weights& weights,
    const EvaluationContext& context
) {
    const Features f = extractStaticFeatures(board);

    double score =
        f.shape * weights.shape +
        f.well * weights.well +
        f.bump * weights.bump +
        f.form * weights.form +
        f.link2 * weights.link2 +
        f.link3 * weights.link3 +
        f.waste14 * weights.waste14 +
        f.side * weights.side +
        f.nuisance * weights.nuisance;

    if (!context.lookahead.empty()) {
        score += quietScore(
            board,
            weights,
            context.lookahead,
            context.quiescenceDepth
        );
    }

    return score;
}

double actionPenalty(
    const Board& before,
    const SimulationResult& result,
    const Weights& weights
) {
    const Features a = extractStaticFeatures(before);
    const Features b = extractStaticFeatures(result.board);

    // "tear": loss of useful links. This is an explicit approximation of
    // ama's action-level tear feature and is kept separate from board eval.
    const double tear =
        std::max(0.0, (a.link2 + a.link3) - (b.link2 + b.link3));

    // "waste": material spent without producing a chain.
    const double waste =
        result.chains == 0 ? 1.0 : 0.0;

    return tear * weights.tear + waste * weights.waste;
}

} // namespace puyo
