MONARCHLOC := /home/laroque/Repos/monarch/cbuild
#MONARCHLOC := /home/grybka/projects/p8/monarch/cbuild
CFLAGS := -Wall -I $(MONARCHLOC)/include -g
#CFLAGS := -Wall -I $(MONARCHLOC)/include -O3
LIBS := -L$(MONARCHLOC)/lib -lMonarchCore -lfftw3f -lfftw3f_threads -lpthread -lm -lprotobuf -lyajl
LDFLAGS = -Wl,-rpath,$(MONARCHLOC)/lib

all: generate_data check_egg_digitization
.PHONY : all

generate_data: generate_data.cc ReceiverTransferFunctions.o
	g++ $(CFLAGS) -c generate_data.cc -o generate_data.o
	g++ $(CFLAGS) $(LDFLAGS) -o generate_data $(LIBS) generate_data.o ReceiverTransferFunctions.o

ReceiverTransferFunctions.o: ReceiverTransferFunctions.cc
	g++ $(CFLAGS) -c -o ReceiverTransferFunctions.o ReceiverTransferFunctions.cc

check_egg_digitization: check_egg_digitization.cc
	g++ $(CFLAGS) -c check_egg_digitization.cc -o check_egg_digitization.o
	g++ $(CFLAGS) $(LDFLAGS) -o check_egg_digitization $(LIBS) check_egg_digitization.o

.PHONY : clean
clean:
	rm -f ReceiverTransferFunctions.o
	rm -f generate_data
	rm -f generate_data.o
	rm -f check_egg_digitization.o
	rm -f check_egg_digitization

