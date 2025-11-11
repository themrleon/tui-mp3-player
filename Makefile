# Compiler and flags
CC = gcc
CFLAGS = 
LDFLAGS = -lncurses

# Target executable name
TARGET = tuimp3

# Source files
SRC = tuimp3.c

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Clean up build files
clean:
	rm -f $(TARGET)

# Install (if needed)
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin (may require sudo)"
	sudo cp $(TARGET) /usr/local/bin/

# Phony targets (not actual files)
.PHONY: all clean install
