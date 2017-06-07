# Directories
OUT_DIR := build
SRC_DIR := src
LIB_DIR := lib $(PREFIX)/lib
LIB_DIR_PARAM :=$(foreach d, $(LIB_DIR), -L$d)
INCLUDE_DIR := include $(PREFIX)/include
INCLUDE_DIR_PARAM :=$(foreach d, $(INCLUDE_DIR), -I$d)

SRCS := \
	main.cpp \
        hardware/smartscopeusb.cpp \
        net/interfaceserver.cpp \
        utils.cpp

OBJS := $(SRCS:.cpp=.cpp.o)
OBJS := $(addprefix $(OUT_DIR)/,$(OBJS))
DEPS := $(OBJS:.o=.d)

CC = $(CROSS_COMPILE)c++
LD = $(CROSS_COMPILE)ld

CCFLAGS += -Wall -g $(INCLUDE_DIR_PARAM) -MMD -MP -std=c++11
LIBS := -lusb-1.0 -lpthread -lstdc++
LDFLAGS += -Wall $(LIBS) $(LIB_DIR_PARAM)


ifdef DNSSD
CCFLAGS += -DDNSSD
else
LIBS += -lavahi-client -lavahi-common -ldbus-1
endif

ifdef DEBUG
CCFLAGS += -DDEBUG
else
CCFLAGS += -O3
endif

.PHONY: all clean clear install

all: smartscopeserver

clean:
	-rm -rf $(OUT_DIR)

install: all
	install -m 755 -d $(DESTDIR)/usr/bin
	install -m 755 $(OUT_DIR)/smartscopeserver $(DESTDIR)/usr/bin

-include $(DEPS)

smartscopeserver: $(OBJS)
	@printf " Making %s %s\n" $@ $^
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

$(OUT_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	@printf "  CC      %s\n" $@
	@mkdir -p $(@D)
	$(CC) $(CCFLAGS) -o $@ -c $<
