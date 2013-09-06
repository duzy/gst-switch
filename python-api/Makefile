.PHONY: lint unittests pytest integration test clean

lint:
	rm -rf tests/unittests/__pycache__
	rm -rf tests/integrationtests/__pycache__
	rm -rf reports
	mkdir -p reports/pylint
	-pylint --rcfile=.pylintrc gstswitch
	-pylint --rcfile=.pylintrc --disable=R0201 --disable=W0613 tests/unittests/*
	-pylint --rcfile=.pylintrc --disable=R0201 tests/integrationtests/*.py
	mv -f *.html reports/pylint
	@echo "Lint report generated in reports/pylint"

# runs py.test also
unittests:
	rm -rf reports
	mkdir -p reports/coverage/unittests
	py.test --cov-report html --cov gstswitch tests/unittests/ -v -s
	mv htmlcov/*.* reports/coverage/unittests
	rmdir htmlcov
	make clean
pytest:
	-py.test tests/unittests -v

integration:
	rm -rf reports
	mkdir -p reports/coverage/integration
	py.test --cov-report html --cov gstswitch tests/integrationtests/ -s -v 2>/dev/null
	mv htmlcov/*.* reports/coverage/integration
	make clean

clean:
	rm -f *.data
	rm -f tests/integrationtests/*.data


test: unittests integration

