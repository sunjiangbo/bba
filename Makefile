CC = gcc
CFLAGS := -c -Wall
LDFLAGS :=-lm -lasound -ldl -lpthread -lavcodec
EXECUTABLE = bba
OBJDIR = objdir
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))
OBJECTS = $(addprefix $(OBJDIR)/, $(OBJS))

all: $(EXECUTABLE)

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
#	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
 
$(OBJECTS): | $(OBJDIR)
     
$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm $(OBJDIR) -rf $(EXECUTABLE) *.raw *.pcm
