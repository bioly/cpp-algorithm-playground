# cpp-algorithm-playground - GNU Make 3.81 compatible (the version macOS ships).
# Only 3.81 features are used: wildcard, patsubst, if, pattern rules, .PHONY.
#
#   make                      build
#   make run                  build and run every exercise in src/main.cpp
#   make run ONLY="lru_cache" run only the named exercises
#   make run V=1              also print the step-by-step traces
#   make debug                -O0 -g with AddressSanitizer + UBSan, then run
#   make new NAME=two_sum     scaffold a new exercise
#   make clean

CXX      ?= c++
CXXSTD   ?= c++17
BUILD    ?= build

# `?=` lets you override from outside (make CXXFLAGS=-O3); `override +=` then
# appends the flags we always need.
CXXFLAGS ?= -O1
override CXXFLAGS += -std=$(CXXSTD) -Wall -Wextra -Wpedantic -Wshadow -Isrc -Iexercises
ifdef WERROR
override CXXFLAGS += -Werror
endif

# main.cpp plus every exercises/<name>/<name>.cpp: new folders are found
# automatically, so this file never changes when you add an exercise.
SRCS := src/main.cpp $(wildcard exercises/*/*.cpp)
OBJS := $(patsubst %.cpp,$(BUILD)/%.o,$(SRCS))
BIN  := $(BUILD)/refresher

.PHONY: all run test list debug new clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# -MMD -MP also writes a .d file with each object's header dependencies, so
# editing a header rebuilds exactly what includes it.
$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(BIN)
	$(BIN) $(if $(V),-v) $(ONLY)

test: run

list: $(BIN)
	$(BIN) --list

debug:
	$(MAKE) BUILD=build/debug CXXFLAGS="-O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer" run

new:
	@test -n "$(NAME)" || { echo "usage: make new NAME=snake_case_name [TITLE=\"Human Title\"]"; exit 1; }
	@./scripts/new_exercise.sh "$(NAME)" "$(TITLE)"

clean:
	rm -rf build

-include $(OBJS:.o=.d)
