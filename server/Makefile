CC = gcc
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter#-Wmissing-declarations

PTHREAD = -pthread

LIBS = libs/
INCLUDES = -I$(LIBS)

OBJS_SERVER_CLIENT = peer.c
OBJS_INCLUDES = $(LIBS)p_server.c $(LIBS)utils.c $(LIBS)client.c $(LIBS)packet.c
#OBJS_O = server.o utils.o client.o file_transfer.o

TRACKER_OBJS_SERVER_CLIENT = tracker.c
TRACKER_OBJS_INCLUDES = $(LIBS)t_server.c $(LIBS)utils.c $(LIBS)client.c $(LIBS)packet.c

EXE_NAME = peer
TRACKER_EXE_NAME = tracker

server_client:
	$(CC) $(INCLUDES) $(WARNINGS) -o $(EXE_NAME) $(OBJS_INCLUDES) $(OBJS_SERVER_CLIENT) $(PTHREAD)

tracker:
	$(CC) $(INCLUDES) $(WARNINGS) -o $(TRACKER_EXE_NAME) $(TRACKER_OBJS_INCLUDES) $(TRACKER_OBJS_SERVER_CLIENT) $(PTHREAD)

clean:
	rm -rf $(EXE_NAME)
