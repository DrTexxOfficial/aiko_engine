SERIAL_PORT = /dev/ttyUSB0

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
OBJECTS += src/common/state/machine.o

AIKO_CLIENT_OBJECTS  = examples/unix/aiko_client.o

AIKO_SERVER_OBJECTS  = examples/unix/aiko_server.o
AIKO_SERVER_OBJECTS += examples/common/aiko_server/lisp_extend.o

AIKO_STATE_MACHINE_OBJECTS = examples/unix/aiko_state_machine.o

AIKO_TCP_OBJECTS = examples/unix/aiko_tcp.o

AIKO_TIMER_OBJECTS = examples/unix/aiko_timer.o

AIKO_UDP_OBJECTS = examples/unix/aiko_udp.o

CONFIGURE_WIFI_OBJECTS  = examples/unix/configure_wifi.o

SERIAL_SERIAL_BRIDGE_OBJECTS  = examples/unix/serial_serial_bridge.o
SERIAL_SERIAL_BRIDGE_OBJECTS += examples/common/serial_serial/serial_serial.o
SERIAL_SERIAL_BRIDGE_OBJECTS += examples/common/aiko_server/lisp_extend.o

SERIAL_UDP_BRIDGE_OBJECTS  = examples/unix/serial_udp_bridge.o
SERIAL_UDP_BRIDGE_OBJECTS += examples/common/serial_udp/serial_udp.o
SERIAL_UDP_BRIDGE_OBJECTS += examples/common/aiko_server/lisp_extend.o

all: aiko_client aiko_server aiko_state_machine aiko_tcp aiko_timer aiko_udp configure_wifi serial_serial_bridge serial_udp_bridge

GIT_VERSION := $(shell git describe --abbrev=8 --dirty --always --tags)

version:
	@echo '#define AIKO_VERSION  "$(GIT_VERSION)"' >include/aiko_version.h

aiko_client:	version $(AIKO_CLIENT_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

aiko_server:	version $(AIKO_SERVER_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

aiko_state_machine:	version $(AIKO_STATE_MACHINE_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

aiko_tcp:	version $(AIKO_TCP_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

aiko_timer:	version $(AIKO_TIMER_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

aiko_udp:	version $(AIKO_UDP_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

configure_wifi:	version $(CONFIGURE_WIFI_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

serial_serial_bridge:	version $(SERIAL_SERIAL_BRIDGE_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

serial_udp_bridge:	version $(SERIAL_UDP_BRIDGE_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

$(OBJECTS):	\
	include/aiko_engine.h        \
	include/aiko_compatibility.h \
	include/aiko_network.h       \
	include/aiko_serial.h        \
	include/aiko_state.h         \
	include/aiko_store.h         \
	include/aiko_wifi.h          \
	include/lisp.h               \
	include/memory/list.h        \
	include/memory/mmem.h        \
	examples/common/aiko_server/lisp_extend.h     \
	examples/common/serial_serial/serial_serial.h \
	examples/common/serial_udp/serial_udp.h

clean:
	-rm -f $(OBJECTS) include/aiko_version.h
	-rm -f $(AIKO_CLIENT_OBJECTS)
	-rm -f $(AIKO_SERVER_OBJECTS)
	-rm -f $(AIKO_STATE_MACHINE_OBJECTS)
	-rm -f $(AIKO_TCP_OBJECTS)
	-rm -f $(AIKO_TIMER_OBJECTS)
	-rm -f $(AIKO_UDP_OBJECTS)
	-rm -f $(CONFIGURE_WIFI_OBJECTS)
	-rm -f $(SERIAL_SERIAL_BRIDGE_OBJECTS)
	-rm -f $(SERIAL_UDP_BRIDGE_OBJECTS)
	-rm -f vendor/contiki-mqtt/mqtt-msg.o

clobber:	clean
	-rm -f aiko_client aiko_server aiko_state_machine aiko_tcp aiko_timer aiko_udp configure_wifi serial_serial_bridge serial_udp_bridge

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

.PHONY: version
