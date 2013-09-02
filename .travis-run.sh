#! /bin/bash

cd python-api

case $TEST in

	python-api )
		case $TYPE in
			unittest )
				make unittests
				;;
			integration )
				make integration
				;;
		esac
		;;
	module )
		make test
esac