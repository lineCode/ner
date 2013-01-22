# ner: notmuch/Makefile.am

noinst_LTLIBRARIES = libnotmuch-util.la

AM_CXXFLAGS = $(glib_CFLAGS)
libnotmuch_util_la_LIBADD = $(glib_LIBS) $(notmuch_LIBS)

libnotmuch_util_la_SOURCES = \
	config.cc config.hh \
	database.cc database.hh \
	exception.cc exception.hh \
	iterator.cc iterator.hh \
	message.cc message.hh \
	message_tree.cc message_tree.hh \
	query.cc query.hh \
	tag_operations.cc tag_operations.hh \
	thread.cc thread.hh \
	tree.hh \
	util.cc util.hh

