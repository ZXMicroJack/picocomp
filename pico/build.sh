#!/bin/bash
if [ "$1" == "" ]; then
	docker run -ti --rm -v`pwd`/..:/work --name lpthdpico  picobuilda bash -c "cd /work/build; make"
else
	docker run -ti --rm -v`pwd`/..:/work --name lpthdpico  picobuilda bash -c "cd /work/build; $1 $2 $3 $4 $5 $6 $7"
fi
