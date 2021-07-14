SUBDIRS =					\
	faces					\
	icons

@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/applications
desktop_in_files = xings-user-accounts.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

uidir=$(pkgdatadir)
ui_DATA=					\
	account-dialog.ui			\
	account-fingerprint.ui			\
	history-dialog.ui			\
	password-dialog.ui			\
	user-accounts-dialog.ui

EXTRA_DIST =					\
	$(ui_DATA)				\
	xings-user-accounts.desktop.in		\
	org.freedesktop.realmd.xml

CLEANFILES =					\
	$(desktop_DATA)
