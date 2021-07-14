AM_CPPFLAGS =						\
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\"	\
	-DDATADIR=\""$(pkgdatadir)"\"			\
	-DHAVE_LIBPWQUALITY				\
	$(CFLAGS)					\
	$(XINGS_USER_ACCOUNTS_CFLAGS)

if BUILD_CHEESE
AM_CPPFLAGS += $(CHEESE_CFLAGS)
endif

bin_PROGRAMS = xings-user-accounts

BUILT_SOURCES = \
	um-realm-generated.c \
	um-realm-generated.h

xings_user_accounts_SOURCES =		\
	cc-crop-area.h			\
	cc-crop-area.c			\
	cc-user-panel.h 		\
	cc-user-panel.c			\
	fingerprint-strings.h		\
	run-passwd.h			\
	run-passwd.c			\
	um-account-dialog.h		\
	um-account-dialog.c		\
	um-account-type.h		\
	um-account-type.c 		\
	um-cell-renderer-user-image.h	\
	um-cell-renderer-user-image.c	\
	um-fingerprint-dialog.h		\
	um-fingerprint-dialog.c		\
	um-history-dialog.h		\
	um-history-dialog.c		\
	um-password-dialog.h		\
	um-password-dialog.c		\
	um-photo-dialog.h		\
	um-photo-dialog.c		\
	um-realm-manager.c		\
	um-realm-manager.h		\
	um-utils.h			\
	um-utils.c			\
	um-user-image.h			\
	um-user-image.c			\
	pw-utils.h			\
	pw-utils.c			\
	xings-user-accounts.c		\
	$(BUILT_SOURCES)

xings_user_accounts_LDADD = 		\
	$(PANEL_LIBS)			\
	$(XINGS_USER_ACCOUNTS_LIBS)	\
	-lpwquality			\
	-lcrypt				\
	-lm				

if BUILD_CHEESE
xings_user_accounts_LDADD += $(CHEESE_LIBS)
endif

um-realm-generated.c: $(top_srcdir)/data/org.freedesktop.realmd.xml
	$(AM_V_GEN) gdbus-codegen --interface-prefix org.freedesktop.realmd. \
		--generate-c-code um-realm-generated --c-namespace UmRealm \
		--annotate "org.freedesktop.realmd.Realm" org.gtk.GDBus.C.Name Common \
		--c-generate-object-manager $<
um-realm-generated.h: um-realm-generated.c

noinst_PROGRAMS = frob-account-dialog

frob_account_dialog_SOURCES = \
	frob-account-dialog.c \
	um-account-dialog.h \
	um-account-dialog.c \
	um-realm-manager.c \
	um-realm-manager.h \
	um-utils.h \
	um-utils.c \
	pw-utils.h \
	pw-utils.c \
	$(BUILT_SOURCES)

frob_account_dialog_LDADD = \
	$(xings_user_accounts_LDADD)

frob_account_dialog_CFLAGS = \
	$(AM_CFLAGS)

CLEANFILES = \
	$(BUILT_SOURCES)
