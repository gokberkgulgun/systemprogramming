CXX=gcc
CXXFLAGS=-D_FILE_OFFSET_BITS=64
LDFLAGS=-pthread -lfuse -ldl

htmldir= ./html
fusedir= ./html_con
fuse : 
	$(CXX) fusehtmlconverter.c converter.c -o htmlconverter $(CXXFLAGS) $(LDFLAGS)

run:
	./htmlconverter -f $(htmldir) $(fusedir)

clean:
	rm -f htmlconverter
	fusermount -u $(fusedir) 
