ACLOCAL_AMFLAGS = -I m4

SUBDIRS = data man po src
DIST_SUBDIRS = data man po src

MAINTAINERCLEANFILES = \
	$(srcdir)/INSTALL \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/compile \
	$(srcdir)/config.guess \
	$(srcdir)/config.h.in \
	$(srcdir)/config.sub \
	$(srcdir)/configure \
	$(srcdir)/depcomp \
	$(srcdir)/install-sh \
	$(srcdir)/ltmain.sh \
	$(srcdir)/missing \
	`find "$(srcdir)" -type f -name Makefile.in -print`

CLEANFILES =				\
	README				\
	NEWS				\
	ChangeLog			\
	*~

README:
	cp -f README.md README

NEWS:
	cp -f NEWS.md NEWS

ChangeLog:
	cp -f NEWS.md ChangeLog
