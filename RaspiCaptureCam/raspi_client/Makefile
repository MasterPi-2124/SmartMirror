CC = "gcc"
PROJECT = raspi_cam_client
SRC = raspi_cam_client.c

# LIBS = `pkg-config opencv4 --cflags --libs`
$(PROJECT) : $(SRC) v4l2lib.o client.o
	$(CC) $(SRC) client.o v4l2lib.o -o $(PROJECT) $(LIBS)

client.o : client.c client.h
		$(CC) -c client.c client.h

v4l2lib.o : v4l2lib.c v4l2lib.h
		$(CC) -c v4l2lib.c v4l2lib.h
clean:
	rm -rf $(PROJECT) client.o v4l2lib.o