lint:
	mkdir -p reports
	mkdir -p reports/pylint
	-pylint --rcfile=.pylintrc gstswitch
	mv -f *.html reports/pylint
	@echo "Lint report generated in reports/lint_report.html"
