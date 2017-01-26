# Directories
OUT_DIR := build
SRC_DIR := src
LIB_DIR := lib
INCLUDE_DIR := include

SRCS := \
	main.cpp \
	hardware/smartscopeusb.cpp
OBJS := $(SRCS:.cpp=.cpp.o)
OBJS := $(addprefix $(OUT_DIR)/,$(OBJS))

GCC ?= g++

CCFLAGS += -Wall -Wextra -std=c++11 -g -Os
LDFLAGS += -Wall -Wextra
LIBS := 

ifdef DEBUG
CCFLAGS += -DDEBUG
endif

.PHONY: all clean clear

all: server

clean:
	-rm -rf $(OUT_DIR)

server: $(OBJS)
	@printf " Making %s %s\n" $@ $^
	$(GCC) -o $@ $^ $(CCFLAGS) $(LIBS)

$(OUT_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	@printf "  CC      %s\n" $@
	@mkdir -p $(@D)
	$(GCC) $(CCFLAGS) -o $@ -c $< -I$(INCLUDE_DIR)