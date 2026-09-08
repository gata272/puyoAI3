CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -pedantic
INCLUDES := -I.

AI_SOURCES := \
	ai/simulation/board.cpp \
	ai/simulation/simulator.cpp \
	ai/evaluation/features.cpp \
	ai/evaluation/weights.cpp \
	ai/evaluation/evaluation.cpp \
	ai/search/move_generator.cpp \
	ai/search/beam_search.cpp \
	ai/gtr/gtr_ai.cpp \
	ai/ai.cpp

.PHONY: test clean

test:
	$(CXX) $(CXXFLAGS) $(INCLUDES) \
		tests/test_native.cpp $(AI_SOURCES) \
		-o /tmp/puyoai3_test
	/tmp/puyoai3_test

clean:
	rm -f /tmp/puyoai3_test
	rm -f ai/**/*.o
