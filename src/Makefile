# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I.
LDFLAGS = -lssl -lcrypto -lpqxx -lpq

# Source files
SOURCES = main.cpp telemetry.cpp encrypt_decrypt.cpp led_control.cpp utils.cpp ../logs/logger.cpp ../database/log_db_operations.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Target binary
TARGET = SCMonitoring

# Default target
all: $(TARGET)

# Rule to build the target binary
$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean