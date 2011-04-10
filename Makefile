.PHONY: all build install doc

all: build

build:
	./scons.py

install:
	./scons.py install

doc:
	doxygen doc_src/Doxyfile
	cp LICENSE.txt doc_generated/html
	mkdir -p doc_generated/html/doc_src
	cp doc_src/nuklei_logo.jpg doc_generated/html/doc_src
