CXX=$(shell sst-config --CXX)
CXXFLAGS=$(shell sst-config --ELEMENT_CXXFLAGS)
LDFLAGS=$(shell sst-config --ELEMENT_LDFLAGS)

all: libspylaxgen.so install

libspylaxgen.so: src/spylax/spylaxgen.cc 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o lib/$@ $^

install: 
	sst-register spylaxgen spylaxgen_LIBDIR=$(CURDIR)/lib

clean:  
	rm -f *.o tests/*.dot tests/*.jpg lib/libspylaxgen.so




