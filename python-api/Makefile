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
	-py.test --cov-report html --cov gstswitch -v
	mv htmlcov/*.* reports/coverage
	rmdir htmlcov
	rm *.data

pytest:
	-py.test tests/unittests -v

