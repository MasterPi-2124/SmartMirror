CC = "gcc"
PROJECT = capturev4l2
SRC = capturev4l2.c
# LIBS = `pkg-config opencv4 --cflags --libs`

$(PROJECT) : $(SRC) client capture
	$(CC) $(SRC) -o $(PROJECT) $(LIBS)

client : client.c
	$(CC) client.c -o client

capture: capture.c
	$(CC) capture.c -lv4l2 -o capture
clean:
	rm -rf $(PROJECT)
	rm -rf hello.png client
	rm -rf /home/ngoquang/Desktop/workplaces/Learn_CV/demo/hello.png