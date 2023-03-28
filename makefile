SHELL = /bin/sh

# Bundle
PREFIX ?= /usr/local
LV2DIR ?= $(PREFIX)/lib/lv2
BUNDLE = BChoppr.lv2
DSP = BChoppr
DSP_SRC = ./src/BChoppr.cpp
GUI = BChoppr_GUI
GUI_SRC = ./src/BChoppr_GUI.cpp
OBJ_EXT = .so
DSP_OBJ = $(DSP)$(OBJ_EXT)
GUI_OBJ = $(GUI)$(OBJ_EXT)
B_OBJECTS = $(addprefix $(BUNDLE)/, $(DSP_OBJ) $(GUI_OBJ))
FILES = *.ttl surface.png LICENSE
B_FILES = $(addprefix $(BUNDLE)/, $(FILES))
DSP_INCL = src/Message.cpp
GUI_CXX_INCL = src/BWidgets/BUtilities/vsystem.cpp 

# pkg-config
PKG_CONFIG ?= pkg-config
GUI_LIBS += lv2 x11 cairo
LV2_LIBS += lv2
ifneq ($(shell $(PKG_CONFIG) --exists fontconfig || echo no), no)
  override GUI_LIBS += fontconfig
  override GUIPPFLAGS += -DPKG_HAVE_FONTCONFIG
endif
DSPCFLAGS += `$(PKG_CONFIG) --cflags --static $(LV2_LIBS)`
GUICFLAGS += -I$(CURDIR)/src/BWidgets/include `$(PKG_CONFIG) --cflags --static $(GUI_LIBS)`
DSPLIBS += -Wl,-Bstatic -lm `$(PKG_CONFIG) --libs --static $(LV2_LIBS)` -Wl,-Bdynamic
GUILIBS += -Wl,-Bstatic -lbwidgetscore -lcairoplus -lpugl -lm -Wl,-Bdynamic `$(PKG_CONFIG) --libs $(GUI_LIBS)`

# complile
CC ?= gcc
CXX ?= g++
INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= $(INSTALL) -m644
STRIP ?= strip
OPTIMIZATIONS ?=-O3 -ffast-math
override CPPFLAGS += -DPIC
override GUIPPFLAGS += -DPUGL_HAVE_CAIRO
override CFLAGS +=-Wall -fvisibility=hidden -fPIC $(OPTIMIZATIONS)
override CXXFLAGS +=-Wall -std=c++17 -fvisibility=hidden -fPIC $(OPTIMIZATIONS)
override STRIPFLAGS +=-s
override LDFLAGS +=-shared -pthread -Lsrc/BWidgets/build

ifdef WWW_BROWSER_CMD
  override GUIPPFLAGS += -DWWW_BROWSER_CMD=\"$(WWW_BROWSER_CMD)\"
endif

# check lib versions
ifeq ($(shell $(PKG_CONFIG) --exists 'lv2 >= 1.12.4' || echo no), no)
  $(error lv2 >= 1.12.4 not found. Please install lv2 >= 1.12.4 first.)
endif
ifeq ($(shell $(PKG_CONFIG) --exists 'x11 >= 1.6.0' || echo no), no)
  $(error x11 >= 1.6.0 not found. Please install x11 >= 1.6.0 first.)
endif
ifeq ($(shell $(PKG_CONFIG) --exists 'cairo >= 1.12.0' || echo no), no)
  $(error cairo >= 1.12.0 not found. Please install cairo >= 1.12.0 first.)
endif

$(BUNDLE): clean $(DSP_OBJ) $(GUI_OBJ)
	@cp $(FILES) $(BUNDLE)

all: $(BUNDLE)

$(DSP_OBJ): $(DSP_SRC)
	@echo -n Build $(BUNDLE) DSP...
	@mkdir -p $(BUNDLE)
	@$(CXX) $(CPPFLAGS) $(OPTIMIZATIONS) $(CXXFLAGS) $(LDFLAGS) $(DSPCFLAGS) $< $(DSP_INCL) $(DSPLIBS) -o $(BUNDLE)/$@
ifeq (,$(filter -g,$(CXXFLAGS)))
	@$(STRIP) $(STRIPFLAGS) $(BUNDLE)/$@
endif
	@echo \ done.

$(GUI_OBJ): $(GUI_SRC) src/BWidgets/build
	@echo -n Build $(BUNDLE) GUI...
	@mkdir -p $(BUNDLE)
	@mkdir -p $(BUNDLE)/tmp
	@cd $(BUNDLE)/tmp; $(CXX) $(CPPFLAGS) $(GUIPPFLAGS) $(CXXFLAGS) $(GUICFLAGS) $(addprefix $(CURDIR)/, $< $(GUI_CXX_INCL)) -c
	@$(CXX) $(LDFLAGS) $(BUNDLE)/tmp/*.o $(GUILIBS) -o $(BUNDLE)/$@
ifeq (,$(filter -g,$(CXXFLAGS)))
	@$(STRIP) $(STRIPFLAGS) $(BUNDLE)/$@
endif
	@rm -rf $(BUNDLE)/tmp
	@echo \ done.

src/BWidgets/build:
	@echo Build Toolkit... 
	@cd src/BWidgets ; $(MAKE) -s cairoplus CFLAGS+=-fvisibility=hidden
	@cd src/BWidgets ; $(MAKE) -s pugl CPPFLAGS+="-DPIC -DPUGL_API=\"__attribute__((visibility(\\\"hidden\\\")))\"" CFLAGS+=-fvisibility=hidden
	@cd src/BWidgets ; $(MAKE) -s bwidgets CPPFLAGS+=-DBUTILITIES_DICTIONARY_DATAFILE=\"\\\"../../BChoppr_Dictionary.data\\\"\" CXXFLAGS+=-fvisibility=hidden 
	@echo ...done.

install:
	@echo -n Install $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_PROGRAM) -m755 $(B_OBJECTS) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_DATA) $(B_FILES) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@cp -R $(BUNDLE) $(DESTDIR)$(LV2DIR)
	@echo \ done.

uninstall:
	@echo -n Uninstall $(BUNDLE)...
	@rm -f $(addprefix $(DESTDIR)$(LV2DIR)/$(BUNDLE)/, $(FILES))
	@rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(GUI_OBJ)
	@rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(DSP_OBJ)
	-@rmdir $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@echo \ done.

clean:
	@echo -n Remove $(BUNDLE)...
	@rm -rf $(BUNDLE)
	@cd src/BWidgets ; $(MAKE) -s clean
	@echo \ done.

.PHONY: all install uninstall clean

.NOTPARALLEL:
