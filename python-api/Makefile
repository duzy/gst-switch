lint:
	mkdir -p reports
	mkdir -p reports/pylint
	-pylint --rcfile=.pylintrc gstswitch
	mv -f *.html reports/pylint
	@echo "Lint report generated in reports/pylint"

# runs py.test also
unittests:
	mkdir -p reports
	mkdir -p reports/coverage/unittests
	-py.test --cov-report html --cov gstswitch tests/unittests/ -v -s
	mv htmlcov/*.* reports/coverage/unittests
	rmdir htmlcov

pytest:
	-py.test tests/unittests -v

integration:
	mkdir -p reports
	mkdir -p reports/coverage/integration
	-py.test --cov-report html --cov gstswitch tests/integrationtests/ -s -v 2>/dev/null
	mv htmlcov/*.* reports/coverage/integration
	rm -f *.data
	rm -f tests/integrationtests/*.data

test: unittests integration

