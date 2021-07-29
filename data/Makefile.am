SUBDIRS =					\
	faces					\
	icons

@INTLTOOL_XML_NOMERGE_RULE@

@GSETTINGS_RULES@
gsettingsschema_in_files = org.xings.user-accounts.gschema.xml.in
gsettings_SCHEMAS = $(gsettingsschema_in_files:.xml.in=.xml)
%.gschema.xml: %.gschema.xml.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@datadir\@^$(datadir)^g' -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@INTLTOOL_DESKTOP_RULE@
desktopdir = $(datadir)/applications
desktop_in_files = xings-user-accounts.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

uidir=$(pkgdatadir)
ui_DATA =					\
	user-accounts.css			\
	account-dialog.ui			\
	account-fingerprint.ui			\
	history-dialog.ui			\
	password-dialog.ui			\
	user-accounts-dialog.ui

EXTRA_DIST =					\
	$(gsettingsschema_in_files)		\
	$(ui_DATA)				\
	xings-user-accounts.desktop.in		\
	org.freedesktop.realmd.xml

CLEANFILES =					\
	$(gsettings_SCHEMAS)			\
	$(desktop_DATA)
