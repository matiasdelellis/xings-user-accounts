EXTRA_DIST =						\
	xings-user-accounts.sgml

if HAVE_DOCBOOK2MAN
man_MANS =						\
	xings-user-accounts.1
endif

if HAVE_DOCBOOK2MAN
xings-user-accounts.1: xings-user-accounts.sgml
	$(AM_V_GEN) docbook2man $? > /dev/null
endif

clean-local :
	rm -f *~
	rm -f *.1
	rm -f manpage.*
