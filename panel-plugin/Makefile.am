AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I $(top_srcdir)/gvc \
	-DG_LOG_DOMAIN=\"xfce4-soundmenu-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DBASEICONDIR=\"$(datadir)/icons/hicolor\" \
	$(PLATFORM_CPPFLAGS)

#
# Soundmenu plugin
#
plugin_LTLIBRARIES = \
	libsoundmenu.la

plugindir = \
	$(libdir)/xfce4/panel/plugins

libsoundmenu_la_SOURCES = \
	soundmenu-panel-plugin.c \
	soundmenu-panel-plugin.h \
	soundmenu-album-art.c \
	soundmenu-album-art.h \
	soundmenu-controls.c \
	soundmenu-controls.h \
	soundmenu-dialog-hig.c \
	soundmenu-dialog-hig.h \
	soundmenu-dialogs.c \
	soundmenu-dialogs.h \
	soundmenu-utils.c \
	soundmenu-utils.h \
	soundmenu-simple-async.c \
	soundmenu-simple-async.h \
	scalemenuitem.c \
	scalemenuitem.h

if FEAT_MIXER
libsoundmenu_la_SOURCES += \
	soundmenu-pulseaudio.c \
	soundmenu-pulseaudio.h
endif

if HAVE_LIBKEYBINDER
libsoundmenu_la_SOURCES += \
	soundmenu-keybinder.c \
	soundmenu-keybinder.h
endif

if HAVE_LIBCLASTFM
libsoundmenu_la_SOURCES += \
	soundmenu-lastfm.c \
	soundmenu-lastfm.h
endif

if HAVE_LIBGLYR
libsoundmenu_la_SOURCES += \
	soundmenu-related.c \
	soundmenu-related.h
endif

if HAVE_LIBNOTIFY
libsoundmenu_la_SOURCES += \
	soundmenu-notify.c \
	soundmenu-notify.h
endif

libsoundmenu_la_CFLAGS = \
	$(DBUS_GLIB_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(LIBMPRIS2CLIENT_CFLAG) \
	$(PLATFORM_CFLAGS)

libsoundmenu_la_LDFLAGS = 					\
	-avoid-version 						\
	-module 						\
	-no-undefined 						\
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)'

libsoundmenu_la_LIBADD = \
	$(DBUS_GLIB_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBXFCE4PANEL_LIBS) \
	$(LIBMPRIS2CLIENT_LIBS) \
	-lm

if FEAT_MIXER
libsoundmenu_la_LIBADD += ../gvc/libgvc.la
endif

if HAVE_LIBKEYBINDER
libsoundmenu_la_CFLAGS += $(LIBKEYBINDER_CFLAGS)
libsoundmenu_la_LIBADD += $(LIBKEYBINDER_LIBS)
endif

if HAVE_LIBCLASTFM
libsoundmenu_la_CFLAGS += $(LIBCLASTFM_CFLAGS)
libsoundmenu_la_LIBADD += $(LIBCLASTFM_LIBS)
endif

if HAVE_LIBGLYR
libsoundmenu_la_CFLAGS += $(LIBGLYR_CFLAGS)
libsoundmenu_la_LIBADD += $(LIBGLYR_LIBS)
endif

if HAVE_LIBNOTIFY
libsoundmenu_la_CFLAGS += $(LIBNOTIFY_CFLAGS)
libsoundmenu_la_LIBADD += $(LIBNOTIFY_LIBS)
endif

#
# Desktop file
#
desktopdir =								\
	$(datadir)/xfce4/panel/plugins

desktop_in_in_files =							\
	soundmenu.desktop.in.in

desktop_in_files =							\
	$(desktop_in_in_files:.desktop.in.in=.desktop.in)

soundmenu.desktop.in: soundmenu.desktop.in.in
	sed -e "s,\@libdir\@,$(libdir),g" < $< > $@

desktop_DATA =								\
	$(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	$(desktop_in_in_files)

CLEANFILES =								\
	$(desktop_in_files)						\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
