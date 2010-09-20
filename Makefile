all: nc10backlight

nc10backlight: nc10backlight.c
	gcc nc10backlight.c -lpciaccess -o nc10backlight

install: nc10backlight
	install -o root -g wheel -m 4755 nc10backlight /usr/local/bin/

