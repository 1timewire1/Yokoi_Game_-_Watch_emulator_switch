#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# ROMFS is the directory containing data to be added to RomFS, relative to the Makefile (Optional)
#
# Switch port (see switch/README.md). Pulls in the shared emulator core
# (../source/SM5XX, ../source/std - NOT ../source/std/GW_ROM*, which is
# embedded-assets-only and unused by this rompack-only build) plus this
# folder's own platform layer (core/input/render/audio), mirroring how
# source/virtual_i_o backs the 3DS build and android/app/src/main/cpp
# backs the Android build.
#
# ../source/David_and_John (+ subdirs) is the "David and John" fake-CPU
# engine - 5 original/open-source games that ride the same SM5XX interface
# as real CPUs, always available regardless of what's in the loaded pack
# (see load_file.cpp). ../source/std/David_And_John_ROM_rgds holds their
# embedded game data in the same rgds-style format Android/Switch both use -
# NOT David_And_John_ROM (3DS-style), which would double-define the same
# symbols if both were compiled in.
#---------------------------------------------------------------------------------
TARGET		:=	yokoi_switch
BUILD		:=	build
SOURCES		:=	source source/core source/input source/render source/audio source/third_party \
			../source/std ../source/SM5XX ../source/SM5XX/SM5A ../source/SM5XX/SM510 ../source/SM5XX/SM511_SM512 \
			../source/std/David_And_John_ROM_rgds \
			../source/David_and_John ../source/David_and_John/common \
			../source/David_and_John/Avignon ../source/David_and_John/Climber \
			../source/David_and_John/Esquive ../source/David_and_John/Fabric ../source/David_and_John/Rythme
DATA		:=	data
# ../source/std is needed as its own include root (not just via ../source):
# some generated headers (e.g. David_And_John_ROM_rgds/*.h) use bare
# #include "GW_ROM.h" rather than "std/GW_ROM.h", matching the convention
# the root 3DS Makefile and Android's CMakeLists.txt both already use.
INCLUDES	:=	source ../source ../source/std
#ROMFS		:=	romfs

APP_TITLE		:=	Yokoi G&W Emulator
APP_AUTHOR		:=	SenseiFromClubPenguin
APP_VERSION		:=	1.0.1

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS	:=	-g -Wall -O2 -ffunction-sections \
			$(ARCH) $(DEFINES)

# Matches android/app/src/main/cpp/CMakeLists.txt's YOKOI_ROMPACK_REQUIRED_CONTENT_VERSION
# default: this build targets the same rgds pack format/version Android does.
# ENABLE_LOGGING=1: writes to sdmc:/switch/yokoi/yokoi_log.txt (see debug_log.h).
# Bring-up build - turn off once the render pipeline is confirmed working.
#
# freetype2's headers live one level under $(PORTLIBS)/include (not picked up
# by the generic -I$(PORTLIBS)/include below), and pkg-config isn't usable
# from this shell (PKG_CONFIG_PATH isn't set outside devkitPro's own bundled
# MSYS2 - see switch/README.md's Building section), so both are hardcoded here.
CFLAGS	+=	$(INCLUDE) -I$(PORTLIBS)/include/freetype2 -D__SWITCH__ -DYOKOI_ROMPACK_REQUIRED_CONTENT_VERSION=3 -DENABLE_LOGGING=1

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++20

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

# freetype/harfbuzz have a circular dependency (freetype's autohinter can use
# harfbuzz; harfbuzz uses freetype) - wrapped in --start-group/--end-group so
# the static linker can resolve both directions regardless of link order.
LIBS	:= -lglad -lEGL -lglapi -ldrm_nouveau \
			-Wl,--start-group -lfreetype -lharfbuzz -lpng16 -lz -lbz2 -Wl,--end-group \
			-lnx

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS) $(LIBNX)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.jpg)
	ifneq (,$(findstring $(TARGET).jpg,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).jpg
	else
		ifneq (,$(findstring icon.jpg,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.jpg
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
	export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
	export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp
endif

ifneq ($(APP_TITLEID),)
	export NACPFLAGS += --titleid=$(APP_TITLEID)
endif

ifneq ($(ROMFS),)
	export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).nro $(TARGET).nacp $(TARGET).elf

#---------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all	:	$(OUTPUT).nro

ifeq ($(strip $(NO_NACP)),)
$(OUTPUT).nro	:	$(OUTPUT).elf $(OUTPUT).nacp
else
$(OUTPUT).nro	:	$(OUTPUT).elf
endif

$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES_BIN)

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
