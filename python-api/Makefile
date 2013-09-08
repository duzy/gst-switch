.PHONY: lint unittests pytest integration test clean all

lint:
	rm -rf tests/unittests/__pycache__
	rm -rf tests/integrationtests/__pycache__
	rm -rf reports
	mkdir -p reports/pylint
	pylint --rcfile=.pylintrc_gstswitch gstswitch
	pylint --rcfile=.pylintrc_unittests tests/unittests/*
	pylint --rcfile=.pylintrc_integrationtests tests/integrationtests/*.py
	mv -f *.html reports/pylint
	@echo "Lint report generated in reports/pylint"

unittests:
	rm -rf reports
	mkdir -p reports/coverage/unittests
	py.test --cov-report html --cov gstswitch tests/unittests/  -v -s
	mv htmlcov/*.* reports/coverage/unittests
	rmdir htmlcov
	make clean

pep8:
	pep8 gstswitch
	@echo "gstswitch is pep8 clean!!"
	pep8 tests/unittests
	@echo "unittests are pep8 clean!!"
	pep8 tests/integrationtests
	@echo "integration tests are pep8 clean!!"

integration:
	rm -f ../tools/*.c.gcov
	rm -f ../tools/*.gcda
	-wget http://imgur.com/tools/imgurbash.sh
	-chmod +x imgurbash.sh
	rm -rf reports
	mkdir -p reports/coverage/integration
	py.test --cov-report html --cov gstswitch tests/integrationtests/ -s -v 2>/dev/null
	mv htmlcov/*.* reports/coverage/integration
	-rm imgurbash.sh*

clean:
	rm -f *.data
	rm -f tests/integrationtests/*.data
	rm -f *.log
	rm -f *.html


test: lint unittests integration clean

all: lint test

