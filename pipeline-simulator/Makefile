RV64GCC = riscv64-unknown-elf-gcc
FLAG = -Wa,-march=rv64im
TESTPRO = fact matrix qsort Ackermann simple

all: Simulation

Simulation: simulation.cpp read_elf.cpp
	g++ -o simulation simulation.cpp read_elf.cpp


clean:
	rm -f Simulation $(TESTPRO)
