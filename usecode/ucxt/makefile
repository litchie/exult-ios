#CXX = gpp
CXX = g++
SRC = src/
IDIR   = include

CXXFLAGS = -O -I$(IDIR)

PGM = ucxt
OBJS = $(SRC)opcodec_.o $(SRC)printucc.o $(SRC)stacki.o \
	$(SRC)ucdata.o $(SRC)ucdump.o $(SRC)ucfunc.o \
	$(SRC)opcodes.o
	 

$(PGM): $(OBJS)
	$(CXX) -o $(PGM) $(OBJS)

clean:
	rm src/*.o
