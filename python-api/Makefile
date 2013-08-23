lint:
	mkdir -p reports
	mkdir -p reports/pylint
	-pylint --rcfile=.pylintrc gstswitch
	mv -f *.html reports/pylint
	@echo "Lint report generated in reports/pylint"

# runs py.test also
coverage:
	mkdir -p reports
	mkdir -p reports/coverage
	-py.test --cov-report html --cov gstswitch tests/unittests/ -v -s
	mv htmlcov/*.* reports/coverage
	rmdir htmlcov

pytest:
	-py.test tests/unittests -v

integration:
	-py.test tests/integrationtests/ -s -v 2>/dev/null
	rm *.data

