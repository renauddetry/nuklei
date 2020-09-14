.PHONY: all build install examples push doc test publish

all: build

build:
	./scons.py

install:
	./scons.py install

examples:
	./scons.py install

doc:
	doxygen doc_src/Doxyfile
	cp LICENSE.txt doc_generated/html
	mkdir -p doc_generated/html/doc_src
	cp doc_src/nuklei_logo.jpg doc_generated/html/doc_src
	cp doc_src/images/kde1d.png doc_generated/html/

test:
	cp nuklei-darwin.conf /tmp/
	./scons.py CXX=g++ gpl=yes use_cgal=yes use_opencv=yes use_pcl=yes
	$(MAKE) check
	./scons.py CXX=g++ gpl=yes use_cgal=no use_opencv=no use_pcl=no
	$(MAKE) check
	cp /tmp/nuklei-darwin.conf .
	./scons.py

check:
	./scons.py check

ifeq "$(shell git rev-parse --abbrev-ref HEAD)" "master"
publish: doc
	rsync -rl --delete --delete-excluded doc_generated/html/ renauddetry@web.sourceforge.net:/home/project-web/nuklei/htdocs/doxygen
else
publish:
	$(error Switch to the master branch before publishing the doc.)
endif
