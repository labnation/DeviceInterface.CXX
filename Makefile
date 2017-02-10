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
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

HOST_OS := $(shell uname | tr a-z A-Z)
CCFLAGS += -Wall -g $(INCLUDE_DIR_PARAM) -I$(PREFIX)/include -MMD -MP -D$(HOST_OS) -std=c++11
LIBS := -lusb-1.0 -lstdc++ -lpthread -pthread
LIB_PATH := -L$(PREFIX)/lib
LDFLAGS += -Wall $(LIBS) $(LIB_PATH) 

ifdef STATIC
LDFLAGS += -static
endif

$(info $$HOST_OS is [${HOST_OS}])
ifeq (LINUX,$(HOST_OS))
LIBS += -lavahi-client -lavahi-common -ldbus-1
endif

ifdef DEBUG
CCFLAGS += -DDEBUG
else
CCFLAGS += -O3
endif

ifdef TARGET
CCFLAGS += -DTARGET_$(TARGET)
endif

.PHONY: all clean clear

all: server

clean:
	-rm -rf $(OUT_DIR)

-include $(DEPS)

server: $(OBJS)
	@printf " Making %s %s\n" $@ $^
	$(CC) -o $@ $^ $(LDFLAGS)

$(OUT_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	@printf "  CC      %s\n" $@
	@mkdir -p $(@D)
	$(CC) $(CCFLAGS) -o $@ -c $<
