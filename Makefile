# basada en (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET_EXEC := plot

BUILD_DIR := ./build
SRC_DIRS := ./src
INC_DIRS := ./include/plot 

CPPFLAGS := -std=c++17 -Wall -Wextra $(shell pkg-config --cflags cairomm-1.0)
LDFLAGS := $(shell pkg-config --libs cairomm-1.0) $(shell pkg-config --libs glfw3)  $(shell pkg-config --libs muparser)

# Find all the C and C++ files we want to compile'
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c')

# String substitution for every C/C++ file.
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(INC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(CPPFLAGS) $(INC_FLAGS) -MMD -MP

#all : $(BUILD_DIR)/$(TARGET_EXEC)

debug: CXXFLAGS += -DDEBUG -g -O0
debug: CCFLAGS += -DDEBUG -g -O0
debug: $(BUILD_DIR)/$(TARGET_EXEC)

release: CXXFLAGS += -DNDEBUG -O2 -s
release: CCFLAGS += -DNDEBUG -O2 -s
release: $(BUILD_DIR)/$(TARGET_EXEC)

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)