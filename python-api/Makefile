.PHONY: lint unittests pytest integration test clean all

lint:
	rm -rf tests/unittests/__pycache__
	rm -rf tests/integrationtests/__pycache__
	rm -rf reports
	mkdir -p reports/pylint
	-pylint --rcfile=.pylintrc_gstswitch gstswitch
	-pylint --rcfile=.pylintrc_unittests tests/unittests/*
	-pylint --rcfile=.pylintrc_integrationtests tests/integrationtests/*.py
	mv -f *.html reports/pylint
	@echo "Lint report generated in reports/pylint"

unittests:
	rm -rf reports
	mkdir -p reports/coverage/unittests
	py.test --cov-report html --cov gstswitch tests/unittests/ -v -s
	mv htmlcov/*.* reports/coverage/unittests
	rmdir htmlcov
	make clean

pytest:
	py.test tests/unittests -v

integration:
	wget http://imgur.com/tools/imgurbash.sh
	chmod +x imgurbash.sh
	rm -rf reports
	mkdir -p reports/coverage/integration
	py.test --cov-report html --cov gstswitch tests/integrationtests/ -s -v 2>/dev/null
	mv htmlcov/*.* reports/coverage/integration
	make clean
	rm imgurbash.sh

clean:
	rm -f *.data
	rm -f tests/integrationtests/*.data


test: unittests integration

all: lint test

