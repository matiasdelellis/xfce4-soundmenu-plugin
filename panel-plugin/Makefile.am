INCLUDES = \
	-I$(top_srcdir) \
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
	soundmenu-plugin.c \
	soundmenu-plugin.h \
	soundmenu-album-art.c \
	soundmenu-album-art.h \
	soundmenu-dbus.c \
	soundmenu-dbus.h \
	soundmenu-dialog-hig.c \
	soundmenu-dialog-hig.h \
	soundmenu-dialogs.c \
	soundmenu-dialogs.h \
	soundmenu-metadata.c \
	soundmenu-metadata.h \
	soundmenu-mpris2.c \
	soundmenu-mpris2.h \
	soundmenu-simple-async.c \
	soundmenu-simple-async.h \
	soundmenu-utils.c \
	soundmenu-utils.h

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

libsoundmenu_la_CFLAGS = \
	$(DBUS_GLIB_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
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
	$(LIBXFCE4PANEL_LIBS)

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

%.desktop.in: %.desktop.in.in
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
