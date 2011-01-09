LIBS_OBJS = src/lib/ipkg/Ipkg.o \
            src/lib/ipkg/IpkgCategory.o \
            src/lib/ipkg/IpkgPackage.o \
            src/lib/emu/ConfigFile.o \
            src/lib/emu/EmuServer.o \
            src/lib/emu/EmuClient.o \
            src/lib/emu/CardReader.o \
            src/lib/emu/Emu.o \
            src/lib/emu/EmuManager.o \
            src/lib/emu/EmuMessages.o

EMUD_OBJS = src/emud.o
EMU_OBJS = src/emu.o
CS_OBJS = src/cs.o

LIBS = libsif.so

SWIGS = src/libsif_wrap.o
SWIGS_LIBS = _libsif.so

EMUD_BIN = emud
EMU_BIN = emu
CS_BIN = cs

all: clean $(LIBS) $(SWIGS_LIBS) $(EMUD_BIN) $(EMU_BIN) $(CS_BIN)

$(SWIGS):
	swig -threads -c++ -python $(@:_wrap.o=.i)
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
	$(CXX) $(LDFLAGS) -shared -o $@ $(LIBS_OBJS) -lopkg -lcurl -lpthread -lSockets

$(SWIGS_LIBS): $(SWIGS)
	$(CXX) $(LDFLAGS) -shared -o $@ $(SWIGS) -L. -lopkg -lcurl -lpthread -lSockets -lsif

$(EMUD_BIN): $(EMUD_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(EMUD_OBJS) -L. -lsif -lpthread -lSockets

$(EMU_BIN): $(EMU_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(EMU_OBJS) -L. -lsif -lpthread -lSockets
	
$(CS_BIN): $(CS_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(CS_OBJS) -L. -lsif -lpthread -lSockets
	
clean:
	rm -f $(LIBS_OBJS) $(LIBS) $(EMUD_BIN) $(EMUD_OBJS) $(EMU_BIN) $(EMU_OBJS) $(CS_BIN) $(CS_OBJS) $(SWIGS) $(SWIGS_LIBS)

install:
	#ncftpput -u root -p sifteam 192.168.0.6 /root/ $(SWIGS_LIBS)
	#ncftpput -u root -p sifteam 192.168.0.6 /root/ src/libsif.py
	ncftpput -u root -p sifteam 192.168.0.6 /usr/lib/python2.5/lib-dynload $(SWIGS_LIBS)
	ncftpput -u root -p sifteam 192.168.0.6 /usr/lib/python2.5/ src/libsif.py
	ncftpput -u root -p sifteam 192.168.0.6 /usr/lib $(LIBS)
	ncftpput -u root -p sifteam 192.168.0.6 /usr/sbin $(EMUD_BIN)
	ncftpput -u root -p sifteam 192.168.0.6 /usr/bin $(EMU_BIN)
	ncftpput -u root -p sifteam 192.168.0.6 /usr/bin $(CS_BIN)
