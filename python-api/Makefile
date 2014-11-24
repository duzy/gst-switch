.PHONY: lint unittests pytest integration test clean all

lint: PYTHONWARNINGS := ignore
lint: PYTHONPATH := ${PYTHONPATH}:${PWD}
lint:
	@rm -rf tests/unittests/__pycache__
	@rm -rf tests/integrationtests/__pycache__
	@rm -rf reports
	pylint --rcfile=.pylintrc_gstswitch gstswitch
	pylint --rcfile=.pylintrc_unittests tests/unittests/*
	pylint --rcfile=.pylintrc_integrationtests tests/integrationtests/*.py
	@echo "Code style is okay!!"

unittests:
	@rm -rf reports
	@mkdir -p reports/coverage/unittests
	py.test --cov gstswitch tests/unittests/ --pep8 -v -s
	@make clean

pep8:
	pep8 gstswitch
	@echo "gstswitch is pep8 clean!!"
	pep8 tests/unittests
	@echo "unittests are pep8 clean!!"
	pep8 tests/integrationtests
	@echo "integration tests are pep8 clean!!"

integration:
	@rm -f ../tools/*.c.gcov
	@rm -f ../tools/*.gcda
	-wget http://imgur.com/tools/imgurbash.sh
	-chmod +x imgurbash.sh
	@rm -rf reports
	@mkdir -p reports/coverage/integration
	py.test --cov gstswitch tests/integrationtests/ --pep8 -s -v 
	-@mv htmlcov/*.* reports/coverage/integration
	-rm imgurbash.sh*

performance:
	py.test tests/performancetests/*.py -v -s
	make clean

clean:
	rm -rf .cache
	rm -rf reports
	rm -f *.data
	rm -f tests/integrationtests/*.data
	rm -f *.log
	rm -f *.html
	rm -f *.sh.*
	rm -f *.sh
	rm -f .coverage


test: lint unittests integration clean
