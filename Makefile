PYTHON_VERSION=2.7
PYTHON_INCLUDE=/usr/include/python${PYTHON_VERSION}

build: giljoy.c
	gcc -I${PYTHON_INCLUDE} -Wall -g -D_GNU_SOURCE giljoy.c -lc -lpython${PYTHON_VERSION} -ldl -lpthread -o giljoypy


clean:
	rm -f giljoypy
