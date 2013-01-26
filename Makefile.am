SUBDIRS = plugins tools tests

EXTRA_DIST = autogen.sh

ACLOCAL_AMFLAGS = -I m4

TESTS = test-controller \
	test-video \
	test-ui-integration \
	test-fuzz \
	test-random-connection \
	test-audio \
	test-switching \
	test-checking-timestamps

test: $(TESTS)
	@ls $(TESTS:%=%.log)

$(TESTS):
	./tests/test-switch-server --enable-$@ #> $@.log

perf:
	echo "TODO: program profiling"

coverage:
	echo "TODO: code coverage"
