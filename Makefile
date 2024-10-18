# Define the compiler
CXX = g++

PROGRAM_NAME = mvccArchiveTool

# Define the flags
CXXFLAGS = -Os -std=c++17 -static-libstdc++ -static-libgcc

# Define the source files
SRCS = src/$(PROGRAM_NAME).cpp src/mvccFileLogic.cpp

# Define the include directories
INCLUDES = -Iinclude

# Define the output executable
TARGET = build\\$(PROGRAM_NAME).exe

# Define the destination directory
#DEST_DIR = ""

# Rule to build the target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS)
	strip $(TARGET)
#	@copy/y $(TARGET) "$(DEST_DIR)\\"

# Rule to copy the build to a new location
copy: $(TARGET)
#	@copy/y $(TARGET) "$(DEST_DIR)\\"

# Rule to run the executable
run: copy
#	$(DEST_DIR)/$(PROGRAM_NAME).exe

# Rule to clean the build
clean:
	rm -f $(TARGET)
