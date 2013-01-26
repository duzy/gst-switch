SUBDIRS = plugins tools tests

EXTRA_DIST = autogen.sh

ACLOCAL_AMFLAGS = -I m4

TESTS = \
	test-video \
	test-audio \
	test-fuzz \
	test-random \
	test-ui \
	test-checking-timestamps \
	test-switching \
	test-controller \
	$(null)

test: $(TESTS)
	@ls $(TESTS:%=%.log)

$(TESTS):
	./tests/test-switch-server --enable-$@ || true

if GPROF_ENABLED
perf:
	echo "TODO: program profiling"
endif

if GCOV_ENABLED
coverage:
	$(MAKE) -C tools coverage
endif
