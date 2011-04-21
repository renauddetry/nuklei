.PHONY: all build install doc

all: build

build:
	./scons.py

install:
	./scons.py install

push:
	git push origin master
doc:
	doxygen doc_src/Doxyfile
	cp LICENSE.txt doc_generated/html
	mkdir -p doc_generated/html/doc_src
	cp doc_src/nuklei_logo.jpg doc_generated/html/doc_src
	cp doc_src/images/kde1d.png doc_generated/html/

test:
	cp nuklei-darwin.conf /tmp/
	./scons.py CXX=g++ gpl=yes qpl=yes
	./scons.py CXX=clang++ gpl=yes qpl=no
	cp /tmp/nuklei-darwin.conf .

publish: doc
	ssh renauddetry,nuklei@shell.sourceforge.net create
	rsync -rl --delete --delete-excluded doc_generated/html/ renauddetry@shell.sourceforge.net:/home/project-web/nuklei/htdocs/doxygen
