SUBDIRS = plugins tools tests

EXTRA_DIST = autogen.sh

ACLOCAL_AMFLAGS = -I m4

TESTS = test-controller \
	test-video \
	test-ui \
	test-fuzz \
	test-random-connection \
	test-audio \
	test-switching \
	test-checking-timestamps

test: $(TESTS)
	@ls $(TESTS:%=%.log)

$(TESTS):
	./tests/test-switch-server --enable-$@ #> $@.log

if GPROF_ENABLED
perf:
	echo "TODO: program profiling"
endif

if GCOV_ENABLED
coverage:
	cd ./tools && gcov gst_switch_srv-*.o
#	cd ./tools && gcov *.c
endif
