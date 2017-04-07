# Directories
OUT_DIR := build
SRC_DIR := src
LIB_DIR := lib
INCLUDE_DIR := include
INCLUDE_DIR_PARAM :=$(foreach d, $(INCLUDE_DIR), -I$d)

SRCS := \
	main.cpp \
        hardware/smartscopeusb.cpp \
        net/interfaceserver.cpp \
        utils.cpp

OBJS := $(SRCS:.cpp=.cpp.o)
OBJS := $(addprefix $(OUT_DIR)/,$(OBJS))
DEPS := $(OBJS:.o=.d)

PREFIX ?= /usr
CC = $(CROSS_COMPILE)c++
LD = $(CROSS_COMPILE)ld

TARGET ?= $(shell uname | tr a-z A-Z)
CCFLAGS += -Wall -g $(INCLUDE_DIR_PARAM) -I$(PREFIX)/include -MMD -DTARGET_${TARGET} -MP -std=c++11
LIBS := -lusb-1.0 -lpthread -lstdc++
LIB_PATH := -L$(PREFIX)/lib
LDFLAGS += -Wall $(LIBS) $(LIB_PATH)

$(info $$TARGET is [${TARGET}])
ifneq (DARWIN,$(TARGET))
LIBS += -lavahi-client -lavahi-common -ldbus-1
endif

ifdef DEBUG
CCFLAGS += -DDEBUG
else
CCFLAGS += -O3
endif

.PHONY: all clean clear

all: smartscopeserver

clean:
	-rm -rf $(OUT_DIR)

-include $(DEPS)

smartscopeserver: $(OBJS)
	@printf " Making %s %s\n" $@ $^
	$(CC) -o $@ $^ $(LDFLAGS)

$(OUT_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	@printf "  CC      %s\n" $@
	@mkdir -p $(@D)
	$(CC) $(CCFLAGS) -o $@ -c $<
