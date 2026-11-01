# bmp2msx-cli — Linux build for BMP2MSX.
#
# Builds the platform-independent conversion core from ../source
# (Convert.cpp, bsave.cpp, utils.cpp, calc.cpp — used unmodified) with
# the Linux support code in src/ and the Win32 shim in winshim/.
# The Windows solution in ../source is not affected by this Makefile.
#
# Objects live under obj/ with -MMD dependency files, so incremental
# and parallel builds (make -j) work as expected.
#
#   make            build ./bmp2msx-cli
#   make gui        build ./bmp2msx-wx (wxWidgets GUI; needs wx-config,
#                   i.e. the wxWidgets development package -- the CLI
#                   above does not)
#   make clean      remove obj/ and the binaries

CXX      ?= g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -D_ENGLISH

SRCROOT  := ../source
OBJDIR   := obj
INCDIR   := $(OBJDIR)/inc

# The Windows sources include some headers with different letter case
# than the file names ("config.h" for Config.h, "convert.h" for
# Convert.h).  Windows does not care; Linux does.  The build therefore
# prepares a directory of lower-case alias symlinks to the real headers.
CORE_SRCS := $(SRCROOT)/Convert.cpp $(SRCROOT)/bsave.cpp \
             $(SRCROOT)/utils.cpp $(SRCROOT)/calc.cpp
CLI_SRCS  := src/main_cli.cpp src/convert_run.cpp src/script.cpp \
             src/image_load.cpp src/fileuty_stdio.cpp src/platform_linux.cpp \
             third_party/stb_image_impl.cpp

CORE_OBJS := $(addprefix $(OBJDIR)/core_, $(notdir $(CORE_SRCS:.cpp=.o)))
CLI_OBJS  := $(addprefix $(OBJDIR)/, $(notdir $(CLI_SRCS:.cpp=.o)))
OBJS      := $(CORE_OBJS) $(CLI_OBJS)

INCLUDES  := -Iwinshim -I$(INCDIR) -I$(SRCROOT) -Isrc -Ithird_party

TARGET := bmp2msx-cli

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Case-alias include farm (generated; part of the build, not the repo)
$(INCDIR)/.stamp:
	@mkdir -p $(INCDIR)
	@for h in $(SRCROOT)/*.h; do \
		name=$$(basename $$h); \
		lower=$$(echo $$name | tr 'A-Z' 'a-z'); \
		ln -sf $$(readlink -f $$h) $(INCDIR)/$$lower; \
	done
	@touch $@

$(OBJDIR)/core_%.o: $(SRCROOT)/%.cpp $(INCDIR)/.stamp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(OBJDIR)/%.o: src/%.cpp $(INCDIR)/.stamp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(OBJDIR)/stb_image_impl.o: third_party/stb_image_impl.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -w $(INCLUDES) -MMD -MP -c $< -o $@

# ---------------------------------------------------------------------------
# wxWidgets GUI (bmp2msx-wx).  Shares the conversion core and the Linux
# support objects with the CLI; only the front end in wx/ is new, and
# only this target needs the wxWidgets development files.
# ---------------------------------------------------------------------------
WX_CONFIG   ?= wx-config
WX_CXXFLAGS := $(shell $(WX_CONFIG) --cxxflags 2>/dev/null)
WX_LIBS     := $(shell $(WX_CONFIG) --libs 2>/dev/null)

WX_SRCS := wx/main_wx.cpp wx/settings_dlg.cpp wx/script_dlg.cpp \
           wx/original_wnd.cpp wx/gui_convert.cpp wx/preview_render.cpp \
           wx/capture_log.cpp
WX_OBJS := $(addprefix $(OBJDIR)/wx_, $(notdir $(WX_SRCS:.cpp=.o)))
# Linux support objects shared with the CLI (everything but main_cli.o)
GUI_SUPPORT_OBJS := $(filter-out $(OBJDIR)/main_cli.o,$(CLI_OBJS))

TARGET_WX := bmp2msx-wx

gui: $(TARGET_WX)

$(TARGET_WX): $(CORE_OBJS) $(GUI_SUPPORT_OBJS) $(WX_OBJS)
	@if [ -z "$(WX_CXXFLAGS)" ]; then \
		echo "wx-config not found: install the wxWidgets development package (see README.md)"; \
		exit 1; \
	fi
	$(CXX) $(CXXFLAGS) -o $@ $(CORE_OBJS) $(GUI_SUPPORT_OBJS) $(WX_OBJS) $(WX_LIBS)

$(OBJDIR)/wx_%.o: wx/%.cpp $(INCDIR)/.stamp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(WX_CXXFLAGS) $(INCLUDES) -Iwx -MMD -MP -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) $(TARGET_WX)

-include $(OBJS:.o=.d) $(WX_OBJS:.o=.d)

.PHONY: all clean gui
