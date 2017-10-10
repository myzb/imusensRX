# Static library makefile
# Created on : Oct 10, 2017
#     Author : may

MINGW_INSTALL_DIR  := /usr/i686-w64-mingw32
DEVEL_DIR          := /local/work.may/Development_Linux
CM_ROOT_DIR        := /local/work.may/CarMaker
OBJ_DIR            := out

TARGET             := libdtrack

LD_FLAGS     :=
LIBS         :=
LIB_DIRS     :=
INCLUDE_DIRS :=
OUT_DIR      :=
EXT          :=

# Local cpp source files
CPP_SRCS := \
	DTrackNet.cpp \
	DTrackParse.cpp \
	DTrackParser.cpp \
	DTrackSDK.cpp

# Target OS
ifeq ($(OS), win)
# Build for Windows

AR       := i686-w64-mingw32-ar
CC       := i686-w64-mingw32-gcc
CXX      := i686-w64-mingw32-g++
OUT_DIR  := $(OBJ_DIR)
EXT      := a

CXXFLAGS := \
	-D_WIN32 \
	-O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP

ARFLAGS  := rcs

else
# Build for Linux

AR       := ar
CC       := gcc
CXX      := g++
OUT_DIR  := $(OBJ_DIR)
EXT      := a

CXXFLAGS := \
	-DOS_LINUX \
	-O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP

ARFLAGS  := rcs

endif

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

# Prepend -l to library list
LIBS := $(addprefix -l,$(LIBS))

# Grab obj names from cpp and prepend obj dir
# obj1.cpp obj2.cpp ... --> $(OBJS) := out/obj1.o out/obj2.o ...
OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o, $(CPP_SRCS))

# All Target
all: directories $(TARGET).$(EXT)

directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OUT_DIR)

# Link
# Passes each object in $(OBJS) to the compiler and links to $(TARGET).$(EXT) afterwards
$(TARGET).$(EXT): $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	$(AR) $(ARFLAGS) $(OUT_DIR)/$(TARGET).$(EXT) $(OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Compile
# Called recursively for each object in $(OBJS) by the linking process
$(OBJ_DIR)/%.o: ./%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(CXX) -m32 $(CXXFLAGS) -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

# Clean build
clean:
	-rm -rf $(OBJ_DIR) $(OUT_DIR)/$(TARGET).*
	@echo ' '

.PHONY: all clean
	