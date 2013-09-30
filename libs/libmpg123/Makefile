.SUFFIXES:
ifeq ($(strip $(PSL1GHT)),)
$(error "PSL1GHT must be set in the environment.")
endif

include $(PSL1GHT)/ppu_rules

#---------------------------------------------------------------------------------

BUILD		:=	build

#---------------------------------------------------------------------------------
ifeq ($(strip $(PLATFORM)),)
#---------------------------------------------------------------------------------
export BASEDIR		:= $(CURDIR)
export INCDIR		:= $(BASEDIR)

#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

DEPSDIR		 = .
DEFINCS		:= -I. -I$(BASEDIR) -I$(PSL1GHT)/ppu/include
INCLUDES	:= $(DEFINCS)

MACHDEP		:= -DBIGENDIAN
CFLAGS		:= -O2 -Wall $(MACHDEP) -fno-strict-aliasing $(INCLUDES) $(CFLAGS)

MPG123LIB	:= icy2utf8.o compat.o parse.o frame.o format.o decode_2to1.o decode_4to1.o decode_ntom.o equalizer.o icy.o id3.o layer1.o\
		layer2.o layer3.o getbits.h optimize.h optimize.o tabinit.o stringbuf.o libmpg123.o decode.o dct64.o parse.o readers.o

$(BUILD):  libmpg123.a

#---------------------------------------------------------------------------------
libmpg123.a: $(MPG123LIB)
#---------------------------------------------------------------------------------
	@echo "[AR]  $(notdir $@)"
	@$(AR) -rcs $@ $^

#---------------------------------------------------------------------------------
install: libmpg123.a
#---------------------------------------------------------------------------------
	@[ -d $(PORTLIBS)/include/soundlib ] || mkdir -p $(PORTLIBS)/include/soundlib
	@echo "[CP]  libmpg123.a"
	@cp -f libmpg123.a $(PORTLIBS)/lib/
	@echo "[CP]  include"
	@cp -f mpg123.h $(PORTLIBS)/include/soundlib
	@rm -f *.d
	@rm -f *.o
	@rm -f *.a

#---------------------------------------------------------------------------------
clean:
#---------------------------------------------------------------------------------
	rm -f *.d
	rm -f *.o
	rm -f *.a
