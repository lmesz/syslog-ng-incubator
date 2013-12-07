module_LTLIBRARIES					+= modules/journal/libjournal.la

modules_journal_libjournal_la_CFLAGS	      = \
	$(SYSLOG_NG_CFLAGS)			\
	$(EVENTLOG_CFLAGS)			\
	$(IVYKIS_CFLAGS)			\
	-I$(top_srcdir)/modules/journal		\
	-I$(top_builddir)/modules/journal

modules_journal_libjournal_la_SOURCES	      = \
	modules/journal/journal-source-grammar.y\
	modules/journal/journal-source.c	\
	modules/journal/journal-source.h	\
	modules/journal/journal-source-parser.c	\
	modules/journal/journal-source-parser.h	\
	modules/journal/journal-source-plugin.c

journal_libs= -lsystemd-journal -lsystemd-id128  
 #$(shell pkg-config --libs libsystemd-journal)

modules_journal_libjournal_la_LIBADD	      = \
	$(EVENTLOG_LIBS)			\
	$(SYSLOG_NG_LIBS)			\
	$(IVYKIS_LIBS) $(journal_libs)

modules_journal_libjournal_la_LDFLAGS	      = \
	-avoid-version -module -no-undefined

modules/journal mod-journal: modules/journal/libjournal.la

BUILT_SOURCES						+= \
	modules/journal/journal-source-grammar.y	   \
	modules/journal/journal-source-grammar.c	   \
	modules/journal/journal-source-grammar.h

EXTRA_DIST						+= \
	modules/journal/journal-source-grammar.ym	   \
	modules/journal/journal-source-example.conf

.PHONY: modules/journal mod-journal
