SUBDIRS = plugins tools tests

EXTRA_DIST = autogen.sh

ACLOCAL_AMFLAGS = -I m4

#TESTS = test-controller
# TESTS += test-video
# TESTS += test-ui-integration
# TESTS += test-fuzz
# TESTS += test-random-connection
# TESTS += test-audio
# TESTS += test-switching
# TESTS += test-checking-timestamps
TESTS = test-fuzz

test: $(TESTS)
	@ls $(TESTS:%=%.log)

#test-%:
#	./tests/test-switch-server --enable-$@ #> $@.log
$(TESTS):
	./tests/test-switch-server --enable-$@ #> $@.log

perf:
	echo "TODO: program profiling"

coverage:
	echo "TODO: code coverage"
