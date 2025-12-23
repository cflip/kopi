CXXFLAGS += -ggdb -Wall -Wextra -Wno-unused-parameter
CXXFLAGS += $(shell llvm-config --cxxflags)
LDFLAGS += $(shell llvm-config --ldflags --system-libs --libs core)

SRCS=src/main.cpp src/ast.cpp src/ast_codegen.cpp src/parser.cpp src/token.cpp
DEPS=src/ast.hpp src/parser.hpp src/token.hpp

OUT=kopic

$(OUT): $(SRCS) $(DEPS)
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SRCS) $(LDFLAGS)

.PHONY: lint
lint:
	clang-format --dry-run -Werror $(SRCS) $(DEPS)
	cpplint $(SRCS) $(DEPS)
