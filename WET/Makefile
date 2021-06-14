#------------ EXECUTABLE FILES : MOSTLY EXPERIMENTS ---------------
pcm_location=/home/maalshbo/Intel_PCM
all :
	g++ -std=c++11 -DIntel_PCM=1 -I ${pcm_location}/pcm/ -L  ${pcm_location}/pcm/cpucounters.o ${pcm_location}/pcm/pci.o ${pcm_location}/pcm/msr.o ${pcm_location}/pcm/client_bw.o  ${pcm_location}/pcm/cpucounters.o -lpthread -o out test.cc
old:
	rm -f out *.o; /local/home/maalshbo/apps/bin/g++ -O3 -fno-tree-vectorize -static -o out test.cc m5op_x86.S /home/maalshbo//m5threads/libpthread.a -lrt
c11:
	g++ -std=c++11 -O3 -fno-tree-vectorize -lpthread  test.cc
gem5:
	rm -f out *.o; /local/home/maalshbo/apps/bin/g++ -DGEM5=1 -O3 -fno-tree-vectorize -static -o out test.cc m5op_x86.S /home/maalshbo//m5threads/libpthread.a -lrt
#	g++ -std=c++11  -DGEM5=1 -O3 -fno-tree-vectorize -lpthread  test.cc

pcm: 
	g++ -std=c++11 -DIntel_PCM=1 -I ${pcm_location}/pcm/ -L  ${pcm_location}/pcm/cpucounters.o ${pcm_location}/pcm/pci.o ${pcm_location}/pcm/msr.o ${pcm_location}/pcm/client_bw.o  ${pcm_location}/pcm/cpucounters.o -lpthread -o out  test.cc

pcmD: 
	g++ -std=c++11 -DIntel_PCM=1  -I ${pcm_location}/pcm/ -L  ${pcm_location}/pcm/cpucounters.o ${pcm_location}/pcm/pci.o ${pcm_location}/pcm/msr.o ${pcm_location}/pcm/client_bw.o ${pcm_location}/pcm/mmio.o ${pcm_location}/pcm/cpucounters.o -lpthread  test.cc
run:
	./out

debug:
	rm -f out *.o; /local/home/maalshbo/apps/bin/g++ -g -O3 -ffast-math -static -o out test.cc m5op_x86.S /home/maalshbo//m5threads/libpthread.a 

clean:
	rm -f out *.o


