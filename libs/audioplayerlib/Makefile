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
DEFINCS		:= -I$(BASEDIR) -I$(PSL1GHT)/ppu/include -I../spu_soundlib -I../libmpg123 -I../libogg-1.2.1
INCLUDES	:= $(DEFINCS) 

MACHDEP		:= -DBIGENDIAN 
CFLAGS		+= -O2 -Wall $(MACHDEP) -fno-strict-aliasing $(INCLUDES)

.PHONY: $(BUILD) clean

AUDIOPLAYERLIB   := memfile.o audioplayer.o 
OGGPLAYERLIB	 := memfile.o oggplayer.o 

#---------------------------------------------------------------------------------
$(BUILD):  libaudioplayer.a liboggplayer.a
#---------------------------------------------------------------------------------	

audioplayer.o: audioplayer.c
	@echo "[CC]  $(notdir $<)"
	@$(CC) $(DEPSOPTIONS) -DUSE_MP3 $(CFLAGS) $(INCLUDES) -c $< -o $@
	@$(CC) $(DEPSOPTIONS) $(CFLAGS) $(INCLUDES) -c $< -o oggplayer.o
%.o: %.c
	@echo "[CC]  $(notdir $<)"
	@$(CC) $(DEPSOPTIONS) -DUSE_MP3 $(CFLAGS) $(INCLUDES) -c $< -o $@

#---------------------------------------------------------------------------------
libaudioplayer.a: $(AUDIOPLAYERLIB)
#---------------------------------------------------------------------------------	
	@echo "[AR]  $(notdir $@)"
	@$(AR) -rcs $@ $^

#---------------------------------------------------------------------------------
liboggplayer.a: $(OGGPLAYERLIB)
#---------------------------------------------------------------------------------	
	@echo "[AR]  $(notdir $@)"
	@$(AR) -rcs $@ $^

#---------------------------------------------------------------------------------
install: libaudioplayer.a liboggplayer.a
#---------------------------------------------------------------------------------
	@[ -d $(PORTLIBS)/include/soundlib ] || mkdir -p $(PORTLIBS)/include/soundlib
	@echo "[CP]  libaudioplayer.a"
	@cp -f libaudioplayer.a $(PORTLIBS)/lib/
	@cp -f audioplayer.h $(PORTLIBS)/include/soundlib
	@echo "[CP]  liboggplayer.a"
	@cp -f liboggplayer.a $(PORTLIBS)/lib/
	@cp -f audioplayer.h $(PORTLIBS)/include/soundlib
	@rm -f *.d .d
	@rm -f $(OGGPLAYERLIB)
	@rm -f audioplayer.o
	@rm -f *.a

#---------------------------------------------------------------------------------
clean:
#---------------------------------------------------------------------------------
	rm -f *.d
	rm -f *.o
	rm -f *.a
