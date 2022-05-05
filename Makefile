all: auth

clean:
	@rm -rf *.o
	@rm -rf auth

install: auth
	useradd -c "auth user" -r -s /sbin/nologin -d /var/www/auth auth
	install -o root -g root -m 0755 auth /usr/local/sbin/
	install -o root -g root -m 0644 auth.service /etc/systemd/system/
	systemctl daemon-reload
	systemctl restart auth.service
	mkdir -p /var/www/auth
	cp -r auth -t /var/www/auth/
	chown -R auth:auth /var/www/auth

uninstall:
	systemctl stop auth
	rm -rf /var/www/auth
	rm -f /usr/local/sbin/auth
	rm -f /etc/systemd/system/auth.service
	systemctl daemon-reload
	userdel -f auth

auth:
	gcc -o auth ./src/main.c ./src/auth.h ./src/auth.c ./src/base64.h ./src/base64.c -lpam -lpam_misc
