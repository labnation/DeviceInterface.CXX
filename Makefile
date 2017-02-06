# Directories
OUT_DIR := build
SRC_DIR := src
LIB_DIR := lib
INCLUDE_DIR := include /usr/include /usr/local/include
INCLUDE_DIR_PARAM :=$(foreach d, $(INCLUDE_DIR), -I$d)

SRCS := \
	main.cpp \
        hardware/smartscopeusb.cpp \
        net/interfaceserver.cpp \
        utils.cpp

OBJS := $(SRCS:.cpp=.cpp.o)
OBJS := $(addprefix $(OUT_DIR)/,$(OBJS))
DEPS := $(OBJS:.o=.d)

GCC ?= gcc


HOST_OS := $(shell uname | tr a-z A-Z)
CCFLAGS += -Wall -std=c++11 -g $(INCLUDE_DIR_PARAM) -MMD -MP -D$(HOST_OS)
LIBS := -lusb-1.0 -lstdc++ -lpthread
LIB_PATH := -L/usr/local/lib
LDFLAGS += -Wall $(LIBS) $(LIB_PATH)

$(info $$HOST_OS is [${HOST_OS}])
ifeq (LINUX,$(HOST_OS))
LIBS += -lavahi-client -lavahi-common
endif

#ifdef DEBUG
CCFLAGS += -DDEBUG
#endif

.PHONY: all clean clear

all: server

clean:
	-rm -rf $(OUT_DIR)

-include $(DEPS)

server: $(OBJS)
	@printf " Making %s %s\n" $@ $^
	$(GCC) -o $@ $^ $(LDFLAGS)

$(OUT_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	@printf "  CC      %s\n" $@
	@mkdir -p $(@D)
	$(GCC) $(CCFLAGS) -o $@ -c $<
