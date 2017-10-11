# Shared library makefile
# Created on : Jun 12, 2017
#     Author : may

TARGET             := libimusensor

# Paths
MKFILE_PATH        := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT_DIR           := $(patsubst %/,%,$(dir $(MKFILE_PATH)))
MINGW_INSTALL_DIR  := /usr/i686-w64-mingw32
DEVEL_DIR          := /local/work.may/Development_Linux
CM_ROOT_DIR        := /local/work.may/CarMaker
OBJ_DIR            := out
RM                 := rm -rf

# Global flags
LD_FLAGS     :=
LIBS         :=
LIB_DIRS     := $(OBJ_DIR)
INCLUDE_DIRS :=
OUT_DIR      :=
EXT          :=

# Sub-projects ----------------------------------------------------------------
SUB_DIRS := \
	DTrackSDK

LIBS += \
	dtrack

#------------------------------------------------------------------------------
# OS independent cpp's
CPP_SRCS := \
	ImuAPIWrapper.cpp \
	quaternionFilters.cpp \
	usbInterface.cpp \
	utils.cpp \
	device_interface.cpp \
	DeviceBase.cpp \
	smarttrack.cpp

# Windows Buids ---------------------------------------------------------------
ifeq ($(OS), win)

# Specific Windows cpp's
CPP_SRCS += \
	hid_windows.cpp
	
# Shared libraries
LIBS += \
	boost_system \
	boost_chrono \
	boost_thread_win32 \
	hid \
	setupapi \
	tcl86 \
	ws2_32

INCLUDE_DIRS += \
	$(DEVEL_DIR)/boost_1_65_1 \
	$(DEVEL_DIR)/tcl8.6.7/generic \
	$(SUB_DIRS)
	
LIB_DIRS += \
	$(DEVEL_DIR)/boost_1_65_1/stage/lib \
	$(DEVEL_DIR)/tcl8.6.7/build/tcl

# Prepend include/lib dir prefix
INCLUDE_DIRS := $(addprefix -I,$(INCLUDE_DIRS))
LIB_DIRS     := $(addprefix -L,$(LIB_DIRS))

CC       := i686-w64-mingw32-gcc
CXX      := i686-w64-mingw32-g++
OUT_DIR  := $(OBJ_DIR)
EXT      := dll

CXXFLAGS := \
	-DOS_WINDOWS -D_WIN32 -DBUILD_DLL \
	-O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP \
	$(INCLUDE_DIRS)

LDFLAGS  := \
	-Wl,--out-implib,$(OBJ_DIR)/$(TARGET).a \
	$(LIB_DIRS)

else
# Linux builds ----------------------------------------------------------------

# Specific Linux cpp's
CPP_SRCS += \
	hid_linux.cpp

# Shared libraries
LIBS += \
	boost_system \
	boost_thread \
	usb \
	tcl8.6
	
INCLUDE_DIRS += \
	$(SUB_DIRS)

CC       := gcc
CXX      := g++
OUT_DIR  := $(CM_ROOT_DIR)/GUI/lib
EXT      := so

# Prepend include/lib dir prefix
INCLUDE_DIRS := $(addprefix -I,$(INCLUDE_DIRS))
LIB_DIRS     := $(addprefix -L,$(LIB_DIRS))

CXXFLAGS := \
	-DOS_LINUX \
	-O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP \
	$(INCLUDE_DIRS)

LDFLAGS  := \
	$(LIB_DIRS)

endif

# Export variables for sub-makefiles ------------------------------------------

export ROOT_DIR
export OBJ_DIR
export OS

#------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#------------------------------------------------------------------------------

# Prepend -l to library list
LIBS := $(addprefix -l,$(LIBS))

# Grab obj names from cpp and prepend obj dir
# obj1.cpp obj2.cpp ... --> $(OBJS) := out/obj1.o out/obj2.o ...
OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o, $(CPP_SRCS))

# All Target
all: \
	directories \
	submodules \
	$(TARGET).$(EXT)

module: \
	directories \
	$(TARGET).$(EXT)

directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OUT_DIR)

# Link
# Passes each object in $(OBJS) to the compiler and links to $(TARGET).$(EXT) afterwards
$(TARGET).$(EXT): $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	$(CXX) -m32 -shared $(LDFLAGS) -o $(OUT_DIR)/$(TARGET).$(EXT) $(OBJS) $(LIBS)
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

# Recursive call subdirs
submodules:
	$(MAKE) -C $(SUB_DIRS)

# Clean build
clean:
	$(RM) $(OBJ_DIR) $(OUT_DIR)/$(TARGET).*
	@echo ' '

.PHONY: all submodules module clean
	