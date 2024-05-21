# Variables
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2
TARGET = vocoder
SRC_DIR = src
SRC = $(SRC_DIR)/vocoder.cc
PYTHON = python3
SCRIPT = audio_streamer.py
INPUT_FILE = test_data/Recording.wav
OUTPUT_FILE = output.wav

# Rules
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	$(PYTHON) $(SCRIPT) $(INPUT_FILE)

clean:
	rm -f $(TARGET) $(OUTPUT_FILE)

.PHONY: all run clean
