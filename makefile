# Shared library makefile
# Created on : Jun 12, 2017
#     Author : may

TARGET   := libimusensor
OUTDIR   := $(HOME)/CarMaker/GUI/lib
OBJDIR   := obj

# Local cpp source files
CPP_SRCS := \
	ImuAPIWrapper.cpp \
	quaternionFilters.cpp \
	usbInterface.cpp \
	utils.cpp \
	hid_linux.cpp

#C_SRCS += \
#./hid_linux.c 

# Shared libraries
LIBS := \
	boost_system \
	boost_thread \
	usb \
	tcl8.6

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

# Prepend -l to library list
LIBS := $(addprefix -l,$(LIBS))

# Copy object names from CPP files and prepend object output dir
# obj1.cpp obj2.cpp ... --> $(OBJS) := out/obj1.o out/obj2.o ...
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o, $(CPP_SRCS))

# Same for C files
#OBJS += $(patsubst %.cpp,$(OBJDIR)/%.o, $(C_SRCS))

# All Target
all: directories $(TARGET).so

directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(OUTDIR)

# Link
# Passes each object in $(OBJS) to the compiler and links to $(TARGET).so afterwards
$(TARGET).so: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -m32 -shared -o $(OUTDIR)/$(TARGET).so $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Compile
# Called recursively for each object in $(OBJS) by the linking process
$(OBJDIR)/%.o: ./%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -m32 -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

#$(OUTDIR)/%.o: ./%.c
#	@echo 'Building file: $<'
#	@echo 'Invoking: GCC C Compiler'
#	gcc -m32 -I"/home/may/Projects_Eclipse/imu_rx" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
#	@echo 'Finished building: $<'
#	@echo ' '

# Clean build
clean:
	-rm -rf $(OBJDIR) $(OUTDIR)/$(TARGET).so
	-@echo ' '

.PHONY: all clean
	
