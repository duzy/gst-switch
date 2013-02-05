SUBDIRS = plugins tools tests

EXTRA_DIST = autogen.sh

ACLOCAL_AMFLAGS = -I m4

include tests/names.mk
test: clean-test-data $(TESTS)
demo: $(UI_TESTS)

show-tests:
	@printf "Available tests:\n\t$(TESTS)\n"
	@printf "And these tests demonstrate the UI:\n\t$(UI_TESTS)\n"
	@printf "You may want to do 'make test-*' for specific test.\n"

$(TESTS) $(UI_TESTS):
	@$(MAKE) -C tests $@

clean-test-data:
	rm -f "tests/test-recording "*" "*.data
	rm -f "tests/test-server-"*.log

if GPROF_ENABLED
perf:
	echo "TODO: program profiling"
endif

if GCOV_ENABLED
coverage:
	$(MAKE) -C tools coverage
endif
