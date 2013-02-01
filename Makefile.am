SUBDIRS = plugins tools tests

EXTRA_DIST = autogen.sh

ACLOCAL_AMFLAGS = -I m4

TESTS = \
	test-video \
	test-audio \
	test-fuzz \
	test-controller \
	test-composite-mode \
	$(null)

UI_TESTS = \
	test-ui \
	test-switching \
	test-random \
	test-checking-timestamps \
	$(null)

test: $(TESTS)

show-tests:
	@printf "Available tests:\n\t$(TESTS)\n"
	@printf "And these tests demonstrate the UI:\n\t$(UI_TESTS)\n"
	@printf "You may want to do 'make test-*' for specific test.\n"

$(TESTS) $(UI_TESTS):
	@./tests/test-switch-server $(TESTARGS) --enable-$@ || true

demo: $(UI_TESTS)

clean-test-data:
	rm -f "record "*" "*.dat
	rm -f "test-recording "*" "*.data
	rm -f test-server-*.log

if GPROF_ENABLED
perf:
	echo "TODO: program profiling"
endif

if GCOV_ENABLED
coverage:
	$(MAKE) -C tools coverage
endif
