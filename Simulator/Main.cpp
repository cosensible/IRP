#include "Simulator.h"


using namespace std;
using namespace szx;


int main() {
    //Simulator::initDefaultEnvironment();

    Simulator sim;
    //sim.debug();
    sim.benchmark(1);
    //sim.parallelBenchmark(1);
    //sim.generateInstance();
	system("pause");
    return 0;
}