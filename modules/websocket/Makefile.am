if ENABLE_WEBSOCKET
module_LTLIBRARIES			+=	\
	modules/websocket/libwebsocket.la

modules_websocket_libwebsocket_la_CFLAGS	=	\
	$(WEBSOCKET_CFLAGS)			\
	$(INCUBATOR_CFLAGS)			\
	-I$(top_srcdir)/modules/websocket		\
	-I$(top_builddir)/modules/websocket
modules_websocket_libwebsocket_la_SOURCES	=	\
	modules/websocket/websocket-grammar.y 		\
	modules/websocket/websocket.c			        \
	modules/websocket/websocket.h						  \
	modules/websocket/websocket-parser.h      \
	modules/websocket/websocket-parser.c

modules_websocket_libwebsocket_la_LIBADD	=	\
	$(WEBSOCKET_LIBS) $(INCUBATOR_LIBS)

modules/websocket modules/websocket/ mod-websocket: \
	modules/websocket/libwebsocket.la
else
modules/websocket modules/websocket/ mod-websocket:
endif

BUILT_SOURCES				+=	\
	modules/websocket/websocket-grammar.y		\
	modules/websocket/websocket-grammar.c		\
	modules/websocket/websocket-grammar.h
EXTRA_DIST				+=	\
	modules/websocket/websocket-grammar.ym

.PHONY: modules/websocket/ mod-websocket
