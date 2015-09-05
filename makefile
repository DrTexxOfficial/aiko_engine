CFLAGS += -DMMEM_CONF_SIZE=256
CFLAGS += -Iinclude

OBJECTS  = src/common/aiko_engine.o
OBJECTS += src/unix/engine.o
OBJECTS += src/unix/network.o
OBJECTS += src/unix/serial.o
OBJECTS += src/unix/store.o
OBJECTS += src/unix/timer.o
OBJECTS += src/unix/wifi.o

OBJECTS += src/common/lisp/expression.o
OBJECTS += src/common/lisp/interface.o
OBJECTS += src/common/lisp/parser.o
OBJECTS += src/common/lisp/primitives.o
OBJECTS += src/common/lisp/utility.o
OBJECTS += src/common/memory/list.o
OBJECTS += src/common/memory/mmem.o 

AIKO_CONFIGURE_OBJECTS  = examples/unix/aiko_configure.o

AIKO_SERVER_OBJECTS  = examples/unix/aiko_server.o
AIKO_SERVER_OBJECTS += examples/common/aiko_server/lisp_extend.o

AIKO_TIMER_OBJECTS = examples/unix/aiko_timer.o

AIKO_UDP_OBJECTS = examples/unix/aiko_udp.o

all:	version aiko_configure aiko_server aiko_timer aiko_udp

GIT_VERSION := $(shell git describe --abbrev=8 --dirty --always --tags)

version:
	@echo '#define AIKO_VERSION  "$(GIT_VERSION)"' >include/aiko_version.h

aiko_configure:	$(AIKO_CONFIGURE_OBJECTS) $(OBJECTS)
	gcc $^ -o $@

aiko_server:	$(AIKO_SERVER_OBJECTS) $(OBJECTS)
	gcc $^ -o $@

aiko_timer:	$(AIKO_TIMER_OBJECTS) $(OBJECTS)
	gcc $^ -o $@

aiko_udp:	$(AIKO_UDP_OBJECTS) $(OBJECTS)
	gcc $^ -o $@

$(OBJECTS):	\
	include/aiko_engine.h        \
	include/aiko_compatibility.h \
	include/aiko_network.h       \
	include/aiko_serial.h        \
	include/aiko_store.h         \
	include/aiko_wifi.h          \
	include/lisp.h               \
	include/memory/list.h        \
	include/memory/mmem.h        \
	examples/common/aiko_server/lisp_extend.h

clean:
	-rm -f $(OBJECTS) include/aiko_version.h
	-rm -f $(AIKO_CONFIGURE_OBJECTS)
	-rm -f $(AIKO_SERVER_OBJECTS)
	-rm -f $(AIKO_TIMER_OBJECTS)
	-rm -f $(AIKO_UDP_OBJECTS)

clobber:	clean
	-rm -f aiko_configure aiko_server aiko_timer aiko_udp

firmware:
	-@mkdir firmware
	@(cd examples/esp8266/aiko_server; $(MAKE))
	@(cp examples/esp8266/aiko_server/firmware/* firmware)

flash:
	@vendor/esptool/esptool.py -p $(SERIAL_PORT) write_flash \
	  0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin

help :
	@echo ""
	@echo "make         - Build Unix examples
	@echo "make clean   - Remove binaries"
	@echo "make clobber - Remove all generated files"
	@echo "make help    - Display usage"
	@echo ""
