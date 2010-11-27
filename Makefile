include comp356.mk

BINS=show_maze2d

show_maze2d : show_maze2d.o maze.o
	$(CC) -o $@ $(CFLAGES) $(CPPFLAGS) $^ $(LDFLAGS) -l356

clean :
	rm -f *.o $(BINS)
