# name of excutable
EXEC := lify

# Complier to use
CXX := clang++

LIBFLAG := -I/opt/homebrew/opt/openssl@3/include -I/opt/homebrew/opt/curl/include
LDFLAGS := -L/opt/homebrew/opt/openssl@3/lib -L/opt/homebrew/opt/curl/lib
LDLIBS := -lssl -lcrypto -lcurl

OPT := -O0
CXXFLAGS := -std=c++23 -g ${OPT} \
            $(LIBFLAG) -Iinclude  \
            -pedantic-errors -Wall -Weffc++ -Wextra \
            -Wconversion -Wsign-conversion -Werror


# Setting the SRC files to all source files in src
SRC := $(shell find src -name "*.cpp")

OBJ := $(patsubst src/%.cpp, build/%.o, $(SRC))

DEPENDS := $(OBJ:.o=.d)

# Commands
all: $(EXEC)

${EXEC}: $(OBJ)
	$(CXX) $(OBJ) $(LDLIBS) $(LDFLAGS) -o $@

build/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -MMD -MP $< -o $@

-include ${DEPENDS}

.PHONY: clean run test

test: $(EXEC)
	./tests/runSuite.sh tests/suite $(EXEC)

run: $(EXEC)
	./$(EXEC)

clean:
	rm -rf build $(EXEC)
