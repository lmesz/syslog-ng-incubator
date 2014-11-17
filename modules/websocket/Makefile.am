module_LTLIBRARIES			+= modules/websocket/libwebsocket.la

modules_websocket_libwebsocket_la_CFLAGS	=	\
	$(INCUBATOR_CFLAGS)			\
	-I$(top_srcdir)/modules/websocket		\
	-I$(top_builddir)/modules/websocket

modules_websocket_libwebsocket_la_LIBADD	=	\
	$(INCUBATOR_LIBS)

modules_websocket_libwebsocket_la_SOURCES	=	\
	modules/websocket/websocket-grammar.y 		\
	modules/websocket/websocket.c			        \
	modules/websocket/websocket.h						  \
	modules/websocket/websocket-parser.c      \
	modules/websocket/websocket-parser.h      \
	modules/websocket/websocket-plugin.c

modules/websocket modules/websocket/ mod-websocket: \
	modules/websocket/libwebsocket.la

BUILT_SOURCES				+=	\
	modules/websocket/websocket-grammar.y		\
	modules/websocket/websocket-grammar.c		\
	modules/websocket/websocket-grammar.h
EXTRA_DIST				+=	\
	modules/websocket/websocket-grammar.ym

.PHONY: modules/websocket/ mod-websocket
