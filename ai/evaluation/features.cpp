#include "features.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace puyo {

namespace {

bool isColor(Cell c) {
    return c != Cell::Empty && c != Cell::Garbage;
}

int sameNeighborCount(const Board& board, int x, int y) {
    Cell c = board.get(x, y);
    if (!isColor(c)) return 0;

    int n = 0;
    if (x > 0 && board.get(x - 1, y) == c) ++n;
    if (x + 1 < BOARD_WIDTH && board.get(x + 1, y) == c) ++n;
    if (y > 0 && board.get(x, y - 1) == c) ++n;
    if (y + 1 < VISIBLE_HEIGHT && board.get(x, y + 1) == c) ++n;
    return n;
}

int countLink3(const Board& board) {
    // Conceptually mirrors ama's bit-field definition: a 3-link cell
    // has two or more same-colour connections in a straight or corner/T
    // configuration.
    int count = 0;

    for (int x = 0; x < BOARD_WIDTH; ++x) {
        for (int y = 0; y < VISIBLE_HEIGHT; ++y) {
            if (sameNeighborCount(board, x, y) < 2) continue;

            Cell c = board.get(x, y);
            bool vertical =
                (y > 0 && board.get(x, y - 1) == c) ||
                (y + 1 < VISIBLE_HEIGHT && board.get(x, y + 1) == c);
            bool horizontal =
                (x > 0 && board.get(x - 1, y) == c) ||
                (x + 1 < BOARD_WIDTH && board.get(x + 1, y) == c);

            bool verticalPair =
                y > 0 && y + 1 < VISIBLE_HEIGHT &&
                board.get(x, y - 1) == c &&
                board.get(x, y + 1) == c;

            bool horizontalPair =
                x > 0 && x + 1 < BOARD_WIDTH &&
                board.get(x - 1, y) == c &&
                board.get(x + 1, y) == c;

            if ((vertical && horizontal) || verticalPair || horizontalPair) {
                ++count;
            }
        }
    }

    return count;
}

int countLink2(const Board& board) {
    int count = 0;

    for (int x = 0; x < BOARD_WIDTH; ++x) {
        for (int y = 0; y < VISIBLE_HEIGHT; ++y) {
            if (!isColor(board.get(x, y))) continue;

            const int neighbors = sameNeighborCount(board, x, y);

            // Endpoints of exactly one connection are the 2-connected
            // component represented by ama's l2 mask after l3 expansion.
            if (neighbors == 1) ++count;
        }
    }

    return count;
}

double getShape(const std::array<int, BOARD_WIDTH>& h) {
    int sum = 0;
    for (int v : h) sum += v;
    int avg = sum / BOARD_WIDTH;

    static constexpr int coef[BOARD_WIDTH] = {1, 1, 1, -1, -1, -1};

    double shape = 0.0;
    for (int x = 0; x < BOARD_WIDTH; ++x) {
        shape += std::abs(h[x] - avg - coef[x]);
    }
    return shape;
}

double getWell(const std::array<int, BOARD_WIDTH>& h) {
    double well = 0.0;

    if (h[0] < h[1]) {
        well += h[1] - h[0];
    }

    if (h[5] < h[4]) {
        well += h[4] - h[5];
    }

    for (int i = 1; i < 5; ++i) {
        if (h[i] < h[i - 1] && h[i] < h[i + 1]) {
            well += std::min(h[i - 1], h[i + 1]) - h[i];
        }
    }

    return well;
}

double getBump(const std::array<int, BOARD_WIDTH>& h) {
    double bump = 0.0;

    for (int i = 1; i < 5; ++i) {
        if (h[i] > h[i - 1] && h[i] > h[i + 1]) {
            bump += h[i] - std::max(h[i - 1], h[i + 1]);
        }
    }

    return bump;
}

double getGtrForm(const std::array<int, BOARD_WIDTH>& h) {
    // A deliberately simple GTR-like shape detector.
    // It is not a copy of ama's private form table; it rewards
    // a low right side and a 2-3 step staircase on the left.
    const double targetLeft[4] = {2, 3, 4, 3};
    const double targetRight[2] = {2, 1};

    double err = 0.0;
    for (int x = 0; x < 4; ++x) {
        err += std::abs(h[x] - targetLeft[x]);
    }
    err += std::abs(h[4] - targetRight[0]);
    err += std::abs(h[5] - targetRight[1]);

    return std::max(0.0, 30.0 - err);
}

} // namespace

Features extractStaticFeatures(const Board& board) {
    Features f;
    const auto h = board.heights();

    f.shape = getShape(h);
    f.well = getWell(h);
    f.bump = getBump(h);

    f.link2 = countLink2(board);
    f.link3 = countLink3(board);

    int garbage = 0;
    for (int x = 0; x < BOARD_WIDTH; ++x) {
        for (int y = 0; y < VISIBLE_HEIGHT; ++y) {
            if (board.get(x, y) == Cell::Garbage) ++garbage;
        }
    }
    f.nuisance = garbage;

    // ama evaluates the reachable cells on row 14. The current simulator
    // uses y=13 as the 14th row.
    int row14Mask = 0;
    for (int x = 0; x < BOARD_WIDTH; ++x) {
        if (board.get(x, BOARD_HEIGHT - 1) != Cell::Empty) {
            row14Mask |= (1 << x);
        }
    }

    int space = 1;
    for (int x = 3; x < 6; ++x) {
        if ((row14Mask >> x) & 1) break;
        ++space;
    }
    for (int x = 1; x >= 0; --x) {
        if ((row14Mask >> x) & 1) break;
        ++space;
    }
    f.waste14 = 6 - space;

    const double left = h[0] + h[1];
    const double right = h[3] + h[4] + h[5];
    f.side = std::max(left, right) - h[2];

    f.form = getGtrForm(h);

    return f;
}

} // namespace puyo
