#pragma once

#include "features.h"
#include "weights.h"
#include "../search/move.h"
#include "../simulation/simulator.h"
#include <vector>

namespace puyo {

struct EvaluationContext {
    std::vector<PuyoPair> lookahead;
    int quiescenceDepth = 1;
};

double evaluate(
    const Board& board,
    const Weights& weights,
    const EvaluationContext& context
);

double actionPenalty(
    const Board& before,
    const SimulationResult& result,
    const Weights& weights
);

} // namespace puyo
