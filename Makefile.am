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
	test-composite-mode \
	$(null)

test: $(TESTS)
	@ls $(TESTS:%=%.log)

show-tests:
	@printf "Available tests:\n\t$(TESTS)\n"
	@printf "You may want to do 'make test-*' for specific test."

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
