CXXFLAGS += -ggdb -Wall -Wextra -Wno-unused-parameter
CXXFLAGS += $(shell llvm-config --cxxflags)
LDFLAGS += $(shell llvm-config --ldflags --system-libs --libs core)

SRCS=main.cpp token.cpp
DEPS=token.hpp

OUT=kopic

$(OUT): $(SRCS) $(DEPS)
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SRCS) $(LDFLAGS)

.PHONY: lint
lint:
	clang-format --dry-run -Werror $(SRCS) $(DEPS)
	cpplint $(SRCS) $(DEPS)
