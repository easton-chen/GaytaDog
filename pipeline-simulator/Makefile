RV64GCC = riscv64-unknown-elf-gcc
FLAG = -Wa,-march=rv64im
TESTPRO = fact matrix qsort Ackermann simple

all: Simulation

Simulation: simulation.cpp read_elf.cpp
	g++ -o simulation simulation.cpp read_elf.cpp

fact: ./testcode/fact.c
	$(RV64GCC) $(FLAG) -o fact ./testcode/fact.c

matrix: ./testcode/matrix.c
	$(RV64GCC) $(FLAG) -o matrix ./testcode/matrix.c

qsort: ./testcode/qsort.c
	$(RV64GCC) $(FLAG) -o qsort ./testcode/qsort.c

Ackermann: ./testcode/Ackermann.c
	$(RV64GCC) $(FLAG) -o Ackermann ./testcode/Ackermann.c

simple: ./testcode/simple.c
	$(RV64GCC) $(FLAG) -o simple ./testcode/simple.c

clean:
	rm -f Simulation $(TESTPRO)
