.PHONY: imagem shell

shell: imagem src
	docker run -it --rm --name os -v `pwd`/src:/src os

imagem: 
	docker build -qt os:latest .
