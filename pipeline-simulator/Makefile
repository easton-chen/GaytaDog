RV64GCC = riscv64-unknown-elf-gcc
FLAG = -Wa,-march=rv64im
TESTPRO = p1 p2 p3 p4 p5 p6 p7 p8 p9 p10

all: Simulation $(TESTPRO)

Simulation: simulation.cpp read_elf.cpp
	g++ -o simulation simulation.cpp read_elf.cpp
p1: ./1/1.cpp
	$(RV64GCC) $(FLAG) -o p1 ./1/1.cpp

p2: ./1/2.cpp
	$(RV64GCC) $(FLAG) -o p2 ./1/2.cpp

p3: ./2/3.cpp
	$(RV64GCC) $(FLAG) -o p3 ./2/3.cpp

p4: ./2/4.cpp
	$(RV64GCC) $(FLAG) -o p4 ./2/4.cpp

p5: ./3/5.cpp
	$(RV64GCC) $(FLAG) -o p5 ./3/5.cpp

p6: ./3/6.cpp
	$(RV64GCC) $(FLAG) -o p6 ./3/6.cpp

p7: ./4/7.cpp
	$(RV64GCC) $(FLAG) -o p7 ./4/7.cpp

p8: ./4/8.cpp
	$(RV64GCC) $(FLAG) -o p8 ./4/8.cpp

p9: ./5/9.cpp
	$(RV64GCC) $(FLAG) -o p9 ./5/9.cpp

p10: ./5/10.cpp
	$(RV64GCC) $(FLAG) -o p10 ./5/10.cpp
clean:
	rm -f Simulation $(TESTPRO)
