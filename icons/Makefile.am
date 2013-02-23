# $Id$

SUBDIRS = 128x128

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then					\
                echo "Updating Gtk icon cache.";			\
                $(gtk_update_icon_cache);				\
        else								\
                echo "*** Icon cache not updated. Remember to run:";	\
		echo "***";						\
                echo "***   $(gtk_update_icon_cache)";			\
		echo "***";						\
        fi

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
