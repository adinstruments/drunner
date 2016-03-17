APP=output/drunner-install
CC=gcc
#CXX=g++
CXX=bin/colorgcc.pl
RM=rm -f
INC=-Isource -Ibuildnum -Isource/settings -Isource/generators -Isource/tests

BOOSTSTATIC=-static -pthread
CPPFLAGS=-Wall -Wno-unknown-pragmas -std=c++11 $(BOOSTSTATIC) $(BUILD_NUMBER_LDFLAGS) $(INC)
LDFLAGS=-static
LDLIBS=-lboost_filesystem -lboost_system

OBJECTS_DIR=objs
SRCS=$(shell find source -maxdepth 2 -name "*.cpp")
HDRS=$(shell find source -maxdepth 2 -name "*.h")
OBJS=$(patsubst source/%,$(OBJECTS_DIR)/%,$(SRCS:.cpp=.o))

all: $(APP)

$(APP): permissions buildnum/build_number.h $(OBJS)
	$(CXX) $(LDFLAGS) -o $(APP) $(OBJS) $(LDLIBS)

depend: .depend

.depend: buildnum/build_number.h $(SRCS) $(HDRS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^ | sed 's#^\(.*:\)#$(OBJECTS_DIR)/\1#' >>./.depend;

include .depend

$(OBJECTS_DIR)/%.o: source/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .depend

buildnum/build_number.h: $(SRCS) $(HDRS) buildnum/major_version
	@echo
	@echo Bumping build number..
	cd buildnum ; ./make_buildnum.sh

permissions: 
	mkdir -p objs/settings objs/generators objs/tests output
	find . -type d -exec chmod 0755 {} \;
	find . -type f -exec chmod 0644 {} \;
	chmod 0755 buildnum/make_buildnum.sh pullall bin/*

upload: $(APP)
	upload output/drunner-install

install: $(APP)
	$(APP) -v ~/temp
