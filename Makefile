# Copyright (C) 2026 samuelmarc
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

MKFILE := $(abspath $(lastword $(MAKEFILE_LIST)))

#---------------------------------------------------------------------------------
# TITLE, APPID, CONTENTID, ICON0, SFOXML must be set before ppu_rules
#---------------------------------------------------------------------------------
TITLE     := PSFlix
APPID     := PS1lix01
CONTENTID := UP0001-$(APPID)_00-0000000000000000
ICON0     := pkgfiles/ICON0.PNG
SFOXML    := sfo.xml

include $(PSL1GHT)/ppu_rules

#---------------------------------------------------------------------------------
# TARGET   : name of the output
# BUILD    : directory for object files & intermediate files
# SOURCES  : directories containing source code (.c .cpp .s .S)
# DATA     : directories containing binary files to embed (*.bin)
# INCLUDES : directories containing headers (.h .hpp)
# PKGFILES : directory copied as-is into the package
#---------------------------------------------------------------------------------
TARGET   := $(notdir $(CURDIR))
BUILD    := build
SOURCES  := source
DATA     :=
INCLUDES := include
PKGFILES := pkgfiles

#---------------------------------------------------------------------------------
# libraries (order matters: dependents first)
#---------------------------------------------------------------------------------
LIBS := -ltiny3d -lsimdmath -lpngdec -lsysmodule -lsysutil -lrsx -lgcm_sys \
        -lm -lrt -llv2

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
CFLAGS   = -O2 -Wall -mcpu=cell -std=gnu99 $(MACHDEP) $(INCLUDE)
CXXFLAGS = -O2 -Wall -Wextra -mcpu=cell -std=gnu++14 $(MACHDEP) $(INCLUDE) \
           -DPSFlix_APP_ID=\"$(APPID)\"
LDFLAGS  = $(MACHDEP) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# list of directories containing libraries (top level with include/ and lib/)
#---------------------------------------------------------------------------------
LIBDIRS := $(PORTLIBS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

.PHONY: all $(BUILD) clean run pkg repkg compdb

all: $(BUILD)

export OUTPUT   := $(CURDIR)/$(TARGET)

export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR  := $(CURDIR)/$(BUILD)
export BUILDDIR := $(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
    export LD := $(CC)
else
    export LD := $(CXX)
endif

export OFILES := $(addsuffix .o,$(BINFILES)) \
                 $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
                 $(sFILES:.s=.o) $(SFILES:.S=.o)

export MKFILE

#---------------------------------------------------------------------------------
# include and library paths
#---------------------------------------------------------------------------------
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  $(LIBPSL1GHT_INC) \
                  -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
                   $(LIBPSL1GHT_LIB)

#---------------------------------------------------------------------------------
# mkdir and $(MAKE) share one line so both also run under `make --just-print`
# (CLion dry-run), where only lines containing $(MAKE) are executed.
#---------------------------------------------------------------------------------
$(BUILD):
	@mkdir -p $(BUILD) && $(MAKE) --no-print-directory -C $(BUILD) -f $(MKFILE)

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).self $(OUTPUT).pkg $(OUTPUT).gnpdrm.pkg

#---------------------------------------------------------------------------------
run: $(BUILD)
	ps3load $(OUTPUT).self

#---------------------------------------------------------------------------------
pkg: $(BUILD) $(OUTPUT).pkg

# Rebuild the .pkg from scratch (use after adding/removing files in pkgfiles/)
repkg:
	@rm -rf $(BUILD)/pkg $(OUTPUT).pkg $(OUTPUT).gnpdrm.pkg
	@$(MAKE) --no-print-directory pkg

#---------------------------------------------------------------------------------
# Generates compile_commands.json for CLion/clangd (requires `bear`)
#---------------------------------------------------------------------------------
compdb: clean
	@bear -- $(MAKE) --no-print-directory

#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).self: $(OUTPUT).elf
$(OUTPUT).elf:  $(OFILES)
$(OFILES):      $(MKFILE)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .bin extension
#---------------------------------------------------------------------------------
%.bin.o: %.bin
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------