#! /bin/bash -ex

cd python-api

case $TEST in

	python-api )
		case $TYPE in
			unittest )
				make unittests || {
					printf "Unittests failed!\n"
					exit -1
				}
				coveralls || {
					printf "Coveralls failed!\n"
					exit -1
				}
				;;
			integration )
				make integration || {
					printf "Integration tests failed!\n"
					exit -1
				}
				coveralls || {
					printf "Coveralls failed!\n"
					exit -1
				}
				;;
		esac
		;;
	module )
		case $TYPE in
			python )
				make test || {
					printf "Tests failed!\n"
					exit -1
				}
				coveralls || {
					printf "Coveralls failed!\n"
					exit -1
				}
				;;
			c )
				make test || {
					printf "Tests failed!\n"
					exit -1
				}
				coveralls -n -r ../tools
		esac
esac || {
	printf "Failed!\n!\n"
	exit -1
}
