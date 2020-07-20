ifndef PLATFORM
PLATFORM = linux
endif

ifndef DOCKER
DOCKER = 0
endif

ifeq ($(PLATFORM),linux)
LINUX = 1
else ifeq ($(PLATFORM),w32)
WINDOWS = 1
CXX = i686-w64-mingw32-g++
else ifeq ($(PLATFORM),w64)
WINDOWS = 1
CXX = x86_64-w64-mingw32-g++
else
$(error Unknown platform '$(PLATFORM)')
endif

ifdef LINUX
LDLIBS_THREAD += -lpthread
LDLIBS_GRAPHICS += -lX11
endif

ifdef WINDOWS
CXXFLAGS += -D_WIN32_WINNT=0x0600
LDFLAGS += -static
LDLIBS_GRAPHICS += -lgdi32
LDLIBS_NET += -lws2_32
EXE = .exe
endif

# C++ standard
CXXFLAGS += -std=gnu++17

# Warnings
CXXFLAGS += -Wall
ifdef WINDOWS
CXXFLAGS += -Wno-format
endif

# Optimize
CXXFLAGS += -O3

# Debug
CXXFLAGS += -g

# Profile
#CXXFLAGS += -pg
#LDFLAGS += -pg

# Dependencies
CPPFLAGS += -MMD -MF $(patsubst %.o,%.d,$@) -MT $@ -MP

# Parallel make
MAKEFLAGS += $(MAKEFLAGS_PARALLEL)

# Architecture
ifdef LINUX
ifneq ($(DOCKER),1)
CXXFLAGS += -march=native -mtune=native
endif
endif
ifdef WINDOWS
#CXXFLAGS += -march=znver1 -mtune=znver1 # Desktop (should be znver2)
#CXXFLAGS += -march=sandybridge -mtune=sandybridge # Laptop
endif

%$(EXE): %.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

LDLIBS = -lgmpxx -lgmp -lcurl
LDLIBS += $(LDLIBS_$(PLATFORM))
LDLIBS += $(LDLIBS_$(patsubst %$(EXE),%,$@))
LDLIBS += $(LDLIBS_$(PLATFORM)_$(patsubst %$(EXE),%,$@))
LDLIBS_linux +=
LDLIBS_test +=
LDLIBS_linux_test +=
LDLIBS_interact += $(LDLIBS_GRAPHICS)
UTILOBJS = StringUtils.o FileUtils.o TimeUtils.o ParseUtils.o
STDOBJS = TokenText.o ParseValue.o Bindings.o Eval.o Modem.o Heap.o PrintValue.o FormatValue.o Protocol.o
BOTOBJS = Bot.o BotFactory.o PassBot.o OrbitBot.o ShootBot.o Gravity.o
ALLPROGS = send run interact test create bot tutorial
ALLPROGS += $(ALLPROGS_$(PLATFORM))
ALLPROGS_linux +=

ALLPROGS_EXE = $(patsubst %,%$(EXE),$(ALLPROGS))

.PHONY: all
all: $(ALLPROGS_EXE)

send$(EXE): send.o $(UTILOBJS) $(STDOBJS)
run$(EXE): run.o $(UTILOBJS) $(STDOBJS)
interact$(EXE): interact.o $(UTILOBJS) $(STDOBJS) Graphics.o
test$(EXE): test.o $(UTILOBJS) $(STDOBJS)
create$(EXE): create.o $(UTILOBJS) $(STDOBJS)
bot$(EXE): bot.o $(UTILOBJS) $(STDOBJS) $(BOTOBJS)
tutorial$(EXE): tutorial.o $(UTILOBJS) $(STDOBJS) $(BOTOBJS)

.PHONY: clean
clean:
	rm -f *.o *.d $(ALLPROGS_EXE)

.PHONY: almostclean
almostclean:
	rm -f *.o *.d

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),almostclean)
include $(patsubst %.o,%.d,$(wildcard *.o))
endif
endif
