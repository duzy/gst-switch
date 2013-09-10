#! /bin/bash

cd python-api

case $TEST in

	python-api )
		case $TYPE in
			unittest )
				make unittests || {
					printf "Python-API unittests failed!"
					exit -1
				}
				coveralls || {
					printf "Coveralls Errored"
					exit -1
				}
				;;
			integration )
				make integration || {
					printf "Python-API integration tests failed!!"
					exit -1
				}
				coveralls || {
					printf "Coveralls failed"
					exit -1
				}
				;;
		esac
		;;
	module )
		case $TYPE in
			python )
				make test || {
					printf "Tests for Python-API failed!"
				}
				coveralls || {
					printf "Coveralls failed"
					exit -1
				}
				;;
			c )
				make test || {
					printf "Tests for gst-switch failed!"
				}
				cd ../tools
				coveralls -r . || {
					printf "Coveralls failed"
					exit -1
				}
		esac
esac