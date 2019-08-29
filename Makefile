#
# Makefile
# Raspberry Pi Beamertool
# Copyright (c) 2013-2014 Moritz Heinemann <mo@blog-srv.net>
#
# Makefile is based on /opt/vc/src/hello_pi/Makefile.include
#

VERSION_STRING=\"0.3.3\"

BIN=bin/rpi-beamertool

OBJS=rpi_beamertool.o
OBJS+=canvas_manager.o canvas.o renderer.o fps_counter.o
OBJS+=content_manager.o content_interface.o
OBJS+=content_example_plugin.o
OBJS+=content_local_file_texture.o texture.o stb_image.o GIFLoader.o video_player.o
OBJS+=artnet_receiver.o config_loader.o console_screen.o

VPATH=src/ src/canvas/ src/canvas/canvas_content/ src/canvas/canvas_content/content_example_plugin/ src/canvas/canvas_content/content_local_file_texture/

OBJS_PATH := $(addprefix build/,$(OBJS))

CC=g++

CFLAGS+=-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi -std=c++11 -DVERSION_STRING=$(VERSION_STRING)

LDFLAGS+=-lilclient
LDFLAGS+=-L$(SDKSTAGE)/opt/vc/lib/ -lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lncurses -L/opt/vc/src/hello_pi/libs/ilclient -L/opt/vc/src/hello_pi/libs/vgfont

INCLUDES+=-I$(SDKSTAGE)/opt/vc/include/ -I$(SDKSTAGE)/opt/vc/include/interface/vcos/pthreads -I$(SDKSTAGE)/opt/vc/include/interface/vmcs_host/linux -I./ -I/opt/vc/src/hello_pi/libs/ilclient -I/opt/vc/src/hello_pi/libs/vgfont -I./include
# -I./include/canvas -I./include/canvas/canvas_content -I./include/canvas/canvas_content/content_example_plugin -I./include/canvas/canvas_content/content_local_file_texture

all: $(BIN) $(LIB)

.SECONDARY: $(OBJS_PATH)

build/%.o: %.c
	@rm -f $@
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

build/%.o: %.cpp
	@rm -f $@
	$(CXX) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

$(BIN): $(OBJS_PATH)
	$(CC) -o $@ -Wl,--whole-archive $(OBJS_PATH) $(LDFLAGS) -Wl,--no-whole-archive -rdynamic

%.a: $(OBJS_PATH)
	$(AR) r $@ $^

clean:
	for i in $(OBJS_PATH); do (if test -e "$$i"; then ( rm $$i ); fi ); done
	@rm -f $(BIN) $(LIB)

