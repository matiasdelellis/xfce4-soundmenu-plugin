INCLUDES = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"xfce4-soundmenu-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# Soundmenu plugin
#
plugin_PROGRAMS = \
	xfce4-soundmenu-plugin

plugindir = \
	$(libexecdir)/xfce4/panel-plugins

xfce4_soundmenu_plugin_SOURCES = \
	soundmenu-plugin.c \
	soundmenu-plugin.h \
	soundmenu-mpris2.c \
	soundmenu-mpris2.h \
	soundmenu-dialogs.c \
	soundmenu-dialogs.h

if HAVE_LIBCLASTFM
xfce4_soundmenu_plugin_SOURCES += \
	soundmenu-lastfm.c \
	soundmenu-lastfm.h

endif

xfce4_soundmenu_plugin_CFLAGS = \
	$(DBUS_GLIB_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(PLATFORM_CFLAGS)
	
xfce4_soundmenu_plugin_LDADD = \
	$(DBUS_GLIB_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBXFCE4PANEL_LIBS)

if HAVE_LIBKEYBINDER
xfce4_soundmenu_plugin_CFLAGS += $(LIBKEYBINDER_CFLAGS)
xfce4_soundmenu_plugin_LDADD += $(LIBKEYBINDER_LIBS)
endif

if HAVE_LIBCLASTFM
xfce4_soundmenu_plugin_CFLAGS += $(LIBCLASTFM_CFLAGS)
xfce4_soundmenu_plugin_LDADD += $(LIBCLASTFM_LIBS)
endif

#
# Desktop file
#
desktopdir =								\
	$(datadir)/xfce4/panel-plugins

desktop_in_in_files =							\
	soundmenu.desktop.in.in

desktop_in_files =							\
	$(desktop_in_in_files:.desktop.in.in=.desktop.in)

%.desktop.in: %.desktop.in.in
	sed -e "s,\@libexecdir\@,$(libexecdir),g" < $< > $@

desktop_DATA =								\
	$(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	$(desktop_in_in_files)

CLEANFILES =								\
	$(desktop_in_files)						\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
