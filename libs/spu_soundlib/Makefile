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
DEFINCS		:= -I$(BASEDIR) -I$(PSL1GHT)/ppu/include
INCLUDES	:= $(DEFINCS) 

MACHDEP		:= -DBIGENDIAN 
CFLAGS		+= -O2 -Wall $(MACHDEP) -fno-strict-aliasing $(INCLUDES)

.PHONY: $(BUILD) clean

SPUSOUNDLIB	:=  spu_soundlib.o

%.o: %.c
	@echo "[CC]  $(notdir $<)"
	@$(CC) $(DEPSOPTIONS) -DUSE_MP3 $(CFLAGS) $(INCLUDES) -c $< -o $@


#---------------------------------------------------------------------------------
libspu_sound.a: $(SPUSOUNDLIB)
#---------------------------------------------------------------------------------

	@echo "[AR]  $(notdir $@)"
	@$(AR) -rcs $@ $^

#---------------------------------------------------------------------------------
install: libspu_sound.a
#---------------------------------------------------------------------------------
	@[ -d $(PSL1GHT)/ppu/include/audio ] || mkdir -p $(PSL1GHT)/ppu/include/audio
	@echo "[CP]  $(notdir $@)"
	@cp -f  libspu_sound.a $(PSL1GHT)/ppu/lib/
	@cp -f  spu_soundlib.h $(PSL1GHT)/ppu/include/audio
	@rm -f *.d
	@rm -f $(SPUSOUNDLIB)
	@rm -f *.a

#---------------------------------------------------------------------------------
clean:
#---------------------------------------------------------------------------------
	
	rm -f *.d
	rm -f *.o
	rm -f *.a

