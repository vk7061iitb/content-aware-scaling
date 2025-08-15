CXX = clang++
CXXFLAGS = -std=c++17 -O2 -Wall
TARGET = seam_carving
SOURCE = seam_carving.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: clean
