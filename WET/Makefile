#------------ EXECUTABLE FILES : MOSTLY EXPERIMENTS ---------------
p=/home/maalshbo/Intel_PCM
all :
	g++ -std=c++11 -I $p/pcm/ -L  $p/pcm/cpucounters.o $p/pcm/pci.o $p/pcm/msr.o $p/pcm/client_bw.o $p/pcm/mmio.o $p/pcm/cpucounters.o -lpthread -o out test.cc
old:
	rm -f out *.o; /local/home/maalshbo/apps/bin/g++ -O3 -fno-tree-vectorize -static -o out test.cc m5op_x86.S /home/maalshbo//m5threads/libpthread.a -lrt
c11:
	g++ -std=c++11 -O3 -fno-tree-vectorize -lpthread  test.cc

pcm: 
	g++ -std=c++11 -I $p/pcm/ -L  $p/pcm/cpucounters.o $p/pcm/pci.o $p/pcm/msr.o $p/pcm/client_bw.o $p/pcm/mmio.o $p/pcm/cpucounters.o -lpthread -o out  test.cc

pcmD: 
	g++ -std=c++11 -I $p/pcm/ -L  $p/pcm/cpucounters.o $p/pcm/pci.o $p/pcm/msr.o $p/pcm/client_bw.o $p/pcm/mmio.o $p/pcm/cpucounters.o -lpthread  test.cc
run:
	./out

debug:
	rm -f out *.o; /local/home/maalshbo/apps/bin/g++ -g -O3 -ffast-math -static -o out test.cc m5op_x86.S /home/maalshbo//m5threads/libpthread.a 

clean:
	rm -f out *.o




