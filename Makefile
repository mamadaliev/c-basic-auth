all: build
	./app
deps: clean
	sudo apt install libpam-modules
	sudo apt remove libpam-dev
build: deps
	gcc -o app ./src/main.c ./src/auth.h ./src/auth.c ./src/base64.h ./src/base64.c -lpam -lpam_misc
clean:
	rm app
