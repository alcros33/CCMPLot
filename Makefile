TARGET_EXEC := plot

BUILD_DIR := ./build
SRC_DIRS := ./src
INC_DIRS := ./include ./include/plot
CXXFLAGS := -std=c++17 -Wall -Wextra
LIBS := cairomm-1.0 glfw3 muparser

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

INC_DIRS := $(shell find $(INC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) $(shell pkg-config --cflags $(LIBS))

LDFLAGS := $(shell pkg-config --libs $(LIBS))

CXXFLAGS += $(INC_FLAGS) $(LDFLAGS)

.PHONY: debug release clean

debug: CXXFLAGS += -DDEBUG -g -O0
debug: CFLAGS += -DDEBUG -g -O0
debug: $(BUILD_DIR)/$(TARGET_EXEC)

release: CXXFLAGS += -DNDEBUG -O2 -s
release: CFLAGS += -DNDEBUG -O2 -s
release: $(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -r $(BUILD_DIR)