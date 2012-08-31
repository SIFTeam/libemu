LIBS_OBJS = src/lib/3rd/ConfigFile.o \
            src/lib/emu/EmuServer.o \
            src/lib/emu/EmuClient.o \
            src/lib/emu/CardReader.o \
            src/lib/emu/Emu.o \
            src/lib/emu/EmuManager.o \
            src/lib/emu/EmuMessages.o

EMUD_OBJS = src/emud.o
EMU_OBJS = src/emu.o
CS_OBJS = src/cs.o

LIBS = libemu.so

SWIGS = src/libemu_wrap.o
SWIGS_LIBS = _libemu.so

EMUD_BIN = emud
EMU_BIN = emu
CS_BIN = cs

#CXXFLAGS = -I/usr/local/include/Sockets -I/usr/include/python2.6 -I/usr/include/libxml2

all: clean $(LIBS) $(SWIGS_LIBS) $(EMUD_BIN) $(EMU_BIN) $(CS_BIN)

$(SWIGS):
	swig -O -threads -c++ -python $(@:_wrap.o=.i)
	$(CXX) $(CXXFLAGS) -c -fpic -o $@ $(@:.o=.cxx)

$(LIBS_OBJS):
	$(CXX) $(CXXFLAGS) -c -fpic -o $@ $(@:.o=.cpp)

$(EMUD_OBJS):
	$(CXX) $(CXXFLAGS) -c -o $@ $(@:.o=.cpp)

$(EMU_OBJS):
	$(CXX) $(CXXFLAGS) -c -o $@ $(@:.o=.cpp)

$(CS_OBJS):
	$(CXX) $(CXXFLAGS) -c -o $@ $(@:.o=.cpp)

$(LIBS): $(LIBS_OBJS)
	$(CXX) $(LDFLAGS) -shared -o $@ $(LIBS_OBJS) -lopkg -lcurl -lpthread -lSockets -lxml2 -lpcre

$(SWIGS_LIBS): $(SWIGS) $(LIBS)
	$(CXX) $(LDFLAGS) -shared -o $@ $(SWIGS) -L. -lopkg -lcurl -lpthread -lSockets -lxml2 -lpcre -lemu

$(EMUD_BIN): $(EMUD_OBJS) $(LIBS)
	$(CXX) $(LDFLAGS) -o $@ $(EMUD_OBJS) -L. -lemu -lpthread -lSockets -lxml2 -lpcre

$(EMU_BIN): $(EMU_OBJS) $(LIBS)
	$(CXX) $(LDFLAGS) -o $@ $(EMU_OBJS) -L. -lemu -lpthread -lSockets -lxml2 -lpcre
	
$(CS_BIN): $(CS_OBJS) $(LIBS)
	$(CXX) $(LDFLAGS) -o $@ $(CS_OBJS) -L. -lemu -lpthread -lSockets -lxml2 -lpcre

clean:
	rm -f $(LIBS_OBJS) $(LIBS) $(EMUD_BIN) $(EMUD_OBJS) $(EMU_BIN) $(EMU_OBJS) $(CS_BIN) $(CS_OBJS) $(SWIGS) $(SWIGS_LIBS)
