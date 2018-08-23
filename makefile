#
#Compiler choose for the process:
CC=gcc

LIBS=`pkg-config --libs --cflags dbus-1 glib-2.0 gio-2.0 gthread-2.0 gio-unix-2.0 gobject-2.0`

build:main.o alarmClockStubs.o
	$(CC) main.o alarmClockStubs.o -o AlarmClockServer $(LIBS)
main.o:alarmClockStubs.c main.c
	$(CC) -c -Wall -o main.o $(LIBS)
alarmClockStubs.o:alarmClockStubs.c
	$(CC) -c -Wall -o alarmClockStubs.o $(LIBS)
alarmClockStubs.c:
	gdbus-codegen --generate-c-code alarmClockStubs --c-namespace alarmClock --interface-prefix com.time.service. --c-generate-object-manager time_service_alarmClock.xml
