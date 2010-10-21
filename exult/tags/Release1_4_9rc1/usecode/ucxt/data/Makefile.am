ucxtdir = $(datadir)/exult

if BUILD_TOOLS
u7bgintrinsics.data u7siintrinsics.data: ../head2data ../../bgintrinsics.h ../../siintrinsics.h
	../head2data u7bgintrinsics.data u7siintrinsics.data
endif

ucxt_DATA = \
	u7opcodes.data	\
	u7misc.data \
	u7bgintrinsics.data \
	u7siintrinsics.data

EXTRA_DIST = \
	events.data	\
	flags.data	\
	opcodes.txt	\
	u7opcodes.data	\
	u7misc.data
