# NVIDIA ROM Parser Makefile
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -pedantic
DEBUG_FLAGS = -g -DDEBUG
RELEASE_FLAGS = -DNDEBUG

# Target executable
TARGET = nvidia_rom_parser

# Source files
SOURCES = nvidia_rom_parser.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: release

# Release build
release: CXXFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Install (copy to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod 755 /usr/local/bin/$(TARGET)

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

# Test with a sample file (if available)
test: $(TARGET)
	@if [ -f "sample.rom" ]; then \
		echo "Testing with sample.rom..."; \
		./$(TARGET) sample.rom test_output.txt; \
	else \
		echo "No sample.rom file found for testing"; \
	fi

# Help target
help:
	@echo "NVIDIA ROM Parser Makefile"
	@echo "=========================="
	@echo "Available targets:"
	@echo "  all        - Build release version (default)"
	@echo "  release    - Build optimized release version"
	@echo "  debug      - Build debug version with debugging symbols"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install to /usr/local/bin (requires sudo)"
	@echo "  uninstall  - Remove from /usr/local/bin (requires sudo)"
	@echo "  test       - Test with sample.rom if available"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make                    # Build release version"
	@echo "  make debug             # Build debug version"
	@echo "  make clean             # Clean build files"
	@echo "  make install           # Install system-wide"

# Mark targets that don't create files
.PHONY: all release debug clean install uninstall test help
