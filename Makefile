# ========================
# Configuration
# ========================
PROJECT_NAME   := liburja.so
SRC_DIR        := src
OBJ_DIR        := build/obj
BIN_DIR        := build/bin
INCLUDE_DIR    := include

# ========================
# PAPI Configuration
# ========================
ifeq ($(filter clean,$(MAKECMDGOALS)),)
ifndef PAPI_DIR
$(error PAPI_DIR environment variable is not set. Please run 'make PAPI_DIR=/path/to/papi')
endif
PAPI_INCLUDE   := -I$(PAPI_DIR)/include
PAPI_LIB       := -L$(PAPI_DIR)/lib -lpapi -ldl -lpthread
endif
# ========================
# Compiler and Flags
# ========================
CXX            := g++

CXXFLAGS := -Wall -O3 -flto=auto -fPIC -std=c++17 -march=native -ffast-math -pthread \
            -I$(INCLUDE_DIR) $(PAPI_INCLUDE)

LDFLAGS  := -shared -flto -Wl,-rpath,$(PAPI_DIR)/lib $(PAPI_LIB) -pthread

# ========================
# Files
# ========================
SRCS           := $(wildcard $(SRC_DIR)/*.cpp)
OBJS           := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET         := $(BIN_DIR)/$(PROJECT_NAME)

# ========================
# Build Rules
# ========================
all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ========================
# Clean
# ========================
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
