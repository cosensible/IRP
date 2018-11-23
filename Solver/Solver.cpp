#include "Solver.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <mutex>

#include <cmath>


using namespace std;


namespace szx {

#pragma region Solver::Cli
int Solver::Cli::run(int argc, char * argv[]) {
    Log(LogSwitch::Szx::Cli) << "parse command line arguments." << endl;
    Set<String> switchSet;
    Map<String, char*> optionMap({ // use string as key to compare string contents instead of pointers.
        { InstancePathOption(), nullptr },
        { SolutionPathOption(), nullptr },
        { RandSeedOption(), nullptr },
        { TimeoutOption(), nullptr },
        { MaxIterOption(), nullptr },
        { JobNumOption(), nullptr },
        { RunIdOption(), nullptr },
        { EnvironmentPathOption(), nullptr },
        { ConfigPathOption(), nullptr },
        { LogPathOption(), nullptr }
    });

    for (int i = 1; i < argc; ++i) { // skip executable name.
        auto mapIter = optionMap.find(argv[i]);
        if (mapIter != optionMap.end()) { // option argument.
            mapIter->second = argv[++i];
        } else { // switch argument.
            switchSet.insert(argv[i]);
        }
    }

    Log(LogSwitch::Szx::Cli) << "execute commands." << endl;
    if (switchSet.find(HelpSwitch()) != switchSet.end()) {
        cout << HelpInfo() << endl;
    }

    if (switchSet.find(AuthorNameSwitch()) != switchSet.end()) {
        cout << AuthorName() << endl;
    }

    Solver::Environment env;
    env.load(optionMap);
    if (env.instPath.empty() || env.slnPath.empty()) { return -1; }

    Solver::Configuration cfg;
    cfg.load(env.cfgPath);

    Log(LogSwitch::Szx::Input) << "load instance " << env.instPath << " (seed=" << env.randSeed << ")." << endl;
    Problem::Input input;
    if (!input.load(env.instPath)) { return -1; }

    Solver solver(input, env, cfg);
    solver.solve();

    pb::Submission submission;
    submission.set_thread(to_string(env.jobNum));
    submission.set_instance(env.friendlyInstName());
    submission.set_duration(to_string(solver.timer.elapsedSeconds()) + "s");
    submission.set_obj(solver.output.totalCost);

    solver.output.save(env.slnPath, submission);
    #if QYM_DEBUG
    solver.output.save(env.solutionPathWithTime(), submission);
    solver.record();
    #endif // QYM_DEBUG

    return 0;
}
#pragma endregion Solver::Cli

#pragma region Solver::Environment
void Solver::Environment::load(const Map<String, char*> &optionMap) {
    char *str;

    str = optionMap.at(Cli::EnvironmentPathOption());
    if (str != nullptr) { loadWithoutCalibrate(str); }

    str = optionMap.at(Cli::InstancePathOption());
    if (str != nullptr) { instPath = str; }

    str = optionMap.at(Cli::SolutionPathOption());
    if (str != nullptr) { slnPath = str; }

    str = optionMap.at(Cli::RandSeedOption());
    if (str != nullptr) { randSeed = atoi(str); }

    str = optionMap.at(Cli::TimeoutOption());
    if (str != nullptr) { msTimeout = static_cast<Duration>(atof(str) * Timer::MillisecondsPerSecond); }

    str = optionMap.at(Cli::MaxIterOption());
    if (str != nullptr) { maxIter = atoi(str); }

    str = optionMap.at(Cli::JobNumOption());
    if (str != nullptr) { jobNum = atoi(str); }

    str = optionMap.at(Cli::RunIdOption());
    if (str != nullptr) { rid = str; }

    str = optionMap.at(Cli::ConfigPathOption());
    if (str != nullptr) { cfgPath = str; }

    str = optionMap.at(Cli::LogPathOption());
    if (str != nullptr) { logPath = str; }

    calibrate();
}

void Solver::Environment::load(const String &filePath) {
    loadWithoutCalibrate(filePath);
    calibrate();
}

void Solver::Environment::loadWithoutCalibrate(const String &filePath) {
    // EXTEND[qym][8]: load environment from file.
    // EXTEND[qym][8]: check file existence first.
}

void Solver::Environment::save(const String &filePath) const {
    // EXTEND[qym][8]: save environment to file.
}
void Solver::Environment::calibrate() {
    // adjust thread number.
    int threadNum = thread::hardware_concurrency();
    if ((jobNum <= 0) || (jobNum > threadNum)) { jobNum = threadNum; }

    // adjust timeout.
    msTimeout -= Environment::SaveSolutionTimeInMillisecond;
}
#pragma endregion Solver::Environment

#pragma region Solver::Configuration
void Solver::Configuration::load(const String &filePath) {
    // EXTEND[szx][5]: load configuration from file.
    // EXTEND[szx][8]: check file existence first.
}

void Solver::Configuration::save(const String &filePath) const {
    // EXTEND[szx][5]: save configuration to file.
}
#pragma endregion Solver::Configuration

#pragma region Solver
bool Solver::solve() {
    init();

    int workerNum = (max)(1, env.jobNum / cfg.threadNumPerWorker);
    cfg.threadNumPerWorker = env.jobNum / workerNum;
    List<Solution> solutions(workerNum, Solution(this));
    List<bool> success(workerNum);

    Log(LogSwitch::Szx::Framework) << "launch " << workerNum << " workers." << endl;
    List<thread> threadList;
    threadList.reserve(workerNum);
    for (int i = 0; i < workerNum; ++i) {
        // TODO[szx][2]: as *this is captured by ref, the solver should support concurrency itself, i.e., data members should be read-only or independent for each worker.
        // OPTIMIZE[szx][3]: add a list to specify a series of algorithm to be used by each threads in sequence.
        threadList.emplace_back([&, i]() { success[i] = optimize(solutions[i], i); });
    }
    for (int i = 0; i < workerNum; ++i) { threadList.at(i).join(); }

    Log(LogSwitch::Szx::Framework) << "collect best result among all workers." << endl;
    int bestIndex = -1;
    double bestValue = 0;
    for (int i = 0; i < workerNum; ++i) {
        if (!success[i]) { continue; }
        Log(LogSwitch::Szx::Framework) << "worker " << i << " got " << solutions[i].totalCost << endl;
        if (solutions[i].totalCost <= bestValue) { continue; }
        bestIndex = i;
        bestValue = solutions[i].totalCost;
    }

    env.rid = to_string(bestIndex);
    if (bestIndex < 0) { return false; }
    output = solutions[bestIndex];
    return true;
}

void Solver::record() const {
    #if QYM_DEBUG
    int generation = 0;

    ostringstream log;

    System::MemoryUsage mu = System::peakMemoryUsage();

    double obj = output.totalCost;
    double checkerObj = -1;
    bool feasible = check(checkerObj);

    // record basic information.
    log << env.friendlyLocalTime() << ","
        << env.rid << ","
        << env.instPath << ","
        << feasible << "," << (obj - checkerObj) << ","
        << output.totalCost << ","
        << input.bestobj() << ","
        << input.referenceobj() << ","
        << timer.elapsedSeconds() << ","
        << input.referencetime() << ","
        << mu.physicalMemory << "," << mu.virtualMemory << ","
        << env.randSeed << ","
        << cfg.toBriefStr() << ","
        << generation << "," << iteration;

    // record solution vector.
    // EXTEND[qym][2]: save solution in log.
    log << endl;

    // append all text atomically.
    static mutex logFileMutex;
    lock_guard<mutex> logFileGuard(logFileMutex);

    ofstream logFile(env.logPath, ios::app);
    logFile.seekp(0, ios::end);
    if (logFile.tellp() <= 0) {
        logFile << "Time,ID,Instance,Feasible,ObjMatch,Cost,MinCost,RefCost,Duration,RefDuration,PhysMem,VirtMem,RandSeed,Config,Generation,Iteration,Solution" << endl;
    }
    logFile << log.str();
    logFile.close();
    #endif // QYM_DEBUG
}

bool Solver::check(double &checkerObj) const {
    #if QYM_DEBUG
    enum CheckerFlag {
        IoError = 0x0,
        FormatError = 0x1,
        SubtourExistenceError = 0x2,
        MultipleVisitError = 0x4,
        LoadDeliveryError = 0x8,
        QuantityReasonabilityError = 0x16
    };

    checkerObj = System::exec("Checker.exe " + env.instPath + " " + env.solutionPathWithTime());
    if (checkerObj > 0) {
        checkerObj = (double)checkerObj / 1000;
        return true;
    }
    int errorCode = (int)checkerObj / 1000;
    errorCode = ~errorCode;
    if (errorCode == CheckerFlag::IoError) { Log(LogSwitch::Checker) << "IoError." << endl; }
    if (errorCode & CheckerFlag::FormatError) { Log(LogSwitch::Checker) << "FormatError." << endl; }
    if (errorCode & CheckerFlag::SubtourExistenceError) { Log(LogSwitch::Checker) << "SubtourExistenceError." << endl; }
    if (errorCode & CheckerFlag::MultipleVisitError) { Log(LogSwitch::Checker) << "MultipleVisitError." << endl; }
    if (errorCode & CheckerFlag::LoadDeliveryError) { Log(LogSwitch::Checker) << "LoadDeliveryError." << endl; }
    if (errorCode & CheckerFlag::QuantityReasonabilityError) { Log(LogSwitch::Checker) << "QuantityReasonabilityError." << endl; }
    checkerObj = errorCode;
    return false;
    #else
    checkerObj = 0;
    return true;
    #endif // QYM_DEBUG
}



void Solver::init() {
	ID nodeNum = input.nodes_size();
	aux.routingCost.init(nodeNum, nodeNum);
	fill(aux.routingCost.begin(), aux.routingCost.end(), 0.0);
	for (ID n = 0; n < nodeNum; ++n) {
		double nx = input.nodes(n).x();
		double ny = input.nodes(n).y();
		for (ID m = 0; m < nodeNum; ++m) {
			if (n == m) { continue; }
			aux.routingCost.at(n, m) = round(hypot(
				nx - input.nodes(m).x(), ny - input.nodes(m).y()));
		}
	}
	//cout << "Lij : " << endl;
	//for (int i = 0; i < nodeNum; ++i) {
	//	for (int j = 0; j < nodeNum; ++j) {
	//		cout << aux.routingCost.at(i, j) << "\t";
	//	}
	//	cout << endl;
	//}
}

static string itos(int i) { stringstream s; s << i; return s.str(); }

bool Solver::optimize(Solution &sln, ID workerId) {
    Log(LogSwitch::Szx::Framework) << "worker " << workerId << " starts." << endl;

	const Length periodNum = input.periodnum() + 1;
	const Length nodeNum = input.nodes_size();

	Arr2D<GRBVar> inventory(periodNum, nodeNum);		// period in {0,1,2,3}
	Arr2D<GRBVar> isNodeVisited(periodNum, nodeNum);	// period in {1,2,3}
	Arr2D<GRBVar> order(periodNum, nodeNum);			// period in {1,2,3} node without {0}
	Arr2D<GRBVar> delivery(periodNum, nodeNum);			// delivery[0]代表从仓库装的货物量
	GRBVar ***isArcVisited;

	// reset solution state.	
	bool status = true;
	auto &allRoutes(*sln.mutable_allroutes());
	const auto &vehicles(*input.mutable_vehicles());
	const auto &nodes(*input.mutable_nodes());

	try {
		GRBEnv env = GRBEnv();
		GRBModel model = GRBModel(env);
		model.set(GRB_StringAttr_ModelName, "IRP");

		// Set variables
		for (int t = 0; t < periodNum; ++t) {
			for (int i = 0; i < nodeNum; ++i) {
				string s = "I_" + itos(i) + "^" + itos(t);
				inventory.at(t, i) = model.addVar(nodes[i].minlevel(), nodes[i].capacity(), 0.0, GRB_CONTINUOUS, s);
			}
		}
		for (int t = 1; t < periodNum; ++t) {
			for (int i = 0; i < nodeNum; ++i) {
				string s = "Z_" + itos(i) + "^" + itos(t);
				isNodeVisited.at(t, i) = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, s);
				s = "W_" + itos(i) + "^" + itos(t);
				delivery.at(t, i) = model.addVar(0.0, vehicles[0].capacity(), 0.0, GRB_CONTINUOUS, s);
				if (i == 0) { continue; }
				s = "U_" + itos(i) + "^" + itos(t);
				order.at(t, i) = model.addVar(1.0, nodeNum, 0.0, GRB_INTEGER, s);
			}
		}
		isArcVisited = new GRBVar **[periodNum];
		for (int t = 1; t < periodNum; ++t) {
			isArcVisited[t] = new GRBVar *[nodeNum];
			for (int i = 0; i < nodeNum; ++i) {
				isArcVisited[t][i] = new GRBVar [nodeNum];
				for (int j = 0; j < nodeNum; ++j) {
					if (j == i) { continue; }
					string s = "X_" + itos(i) + itos(j) + "^" + itos(t);
					isArcVisited[t][i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, s);
				}
			}
		}
		model.update();

		GRBLinExpr obj = 0.0;
		for (int t = 0; t < periodNum; ++t) {
			for (int i = 0; i < nodeNum; ++i) {
				obj += nodes[i].holidingcost()*inventory[t][i];
				if (t > 0) {
					for (int j = 0; j < nodeNum; ++j) {
						if (i == j) { continue; }
						obj += aux.routingCost[i][j] * isArcVisited[t][i][j];
					}
				}

			}
		}
		model.setObjective(obj, GRB_MINIMIZE);

		for (int i = 0; i < nodeNum; ++i) {	//初始库存
			model.addConstr(inventory.at(0, i) == nodes[i].initquantity());
		}
		for (int t = 1; t < periodNum; ++t) {
			for (int i = 1; i < nodeNum; ++i) {
				model.addConstr(isNodeVisited[t][i] <= isNodeVisited[t][0]);
				model.addConstr(delivery[t][i] <= nodes[i].capacity() * isNodeVisited[t][i]);
				model.addConstr(inventory[t - 1][i] + delivery[t][i] <= nodes[i].capacity());
				//model.addConstr(inventory[t - 1][i] >= input.nodes(i).minlevel() + input.nodes(i).unitdemand());
				model.addConstr(inventory[t][i] >= nodes[i].minlevel());
				model.addConstr(inventory[t][i] == inventory[t - 1][i] - nodes[i].unitdemand() + delivery[t][i]);
			}
		}
		for (int t = 1; t < periodNum; ++t) {
			GRBLinExpr vehicleLoad = 0.0;
			for (int i = 1; i < nodeNum; ++i) {
				vehicleLoad += delivery[t][i];
			}
			model.addConstr(delivery[t][0] == vehicleLoad);
			model.addConstr(inventory[t][0] == inventory[t - 1][0] - nodes[0].unitdemand() - vehicleLoad);
			model.addConstr(vehicleLoad <= vehicles[0].capacity());
			model.addConstr(vehicleLoad <= inventory[t - 1][0]);
		}
		for (int t = 1; t < periodNum; ++t) {
			for (int i = 0; i < nodeNum; ++i) {
				GRBLinExpr outdegree = 0.0, indegree = 0.0;
				for (int j = 0; j < nodeNum; ++j) {
					if (j == i) { continue; }
					outdegree += isArcVisited[t][i][j];
					indegree += isArcVisited[t][j][i];
				}
				model.addConstr(outdegree == isNodeVisited[t][i]);
				model.addConstr(indegree == isNodeVisited[t][i]);
			}
		}
		for (int t = 1; t < periodNum; ++t) {
			for (int i = 1; i < nodeNum; ++i) {
				for (int j = 1; j < nodeNum; ++j) {
					if (j == i) { continue; }
					model.addConstr(order[t][i] - order[t][j] + nodeNum * isArcVisited[t][i][j] <= nodeNum - 1);
				}
			}
		}
		model.optimize();

		for (int t = 1; t < periodNum; ++t) {
			auto &routeInPeriod(*allRoutes.Add());
			auto &route(*routeInPeriod.add_routes());
			if (isNodeVisited[t][0].get(GRB_DoubleAttr_X)) {
				ID preNode = 0;
				L:
				for (int i = 1; i < nodeNum; ++i) {
					if (i != preNode && isArcVisited[t][preNode][i].get(GRB_DoubleAttr_X)) {
						auto &deliveries(*route.add_deliveries());
						deliveries.set_node(i);
						deliveries.set_quantity(round(delivery.at(t, i).get(GRB_DoubleAttr_X)));
						preNode = i;
						goto L;
					}
				}
				auto &deliveries(*route.add_deliveries());
				deliveries.set_node(0);
				deliveries.set_quantity(round(-delivery.at(t, 0).get(GRB_DoubleAttr_X)));
			}
		}

		sln.totalCost = model.get(GRB_DoubleAttr_ObjVal);

		//for (int t = 1; t < periodNum; ++t) {
		//	for (int i = 0; i < nodeNum; ++i) {
		//		printf("%s : %.20lf\n", delivery.at(t, i).get(GRB_StringAttr_VarName), delivery.at(t, i).get(GRB_DoubleAttr_X));
		//	}
		//}
		//cout << "是否访问节点：" << endl;
		//for (int i = 0; i < nodeNum; ++i) {
		//	cout << isNodeVisited[2][i].get(GRB_StringAttr_VarName) << ":"
		//		<< isNodeVisited[2][i].get(GRB_DoubleAttr_X) << "\t\t";

		//}

		//cout << "\n是否访问边：" << endl;
		//for (int i = 0; i < nodeNum; ++i) {
		//	for (int j = 0; j < nodeNum; ++j) {
		//		if (i == j) { continue; }
		//		cout << isArcVisited[2][i][j].get(GRB_StringAttr_VarName) << ":"
		//			<< isArcVisited[2][i][j].get(GRB_DoubleAttr_X) << "\t";
		//	}
		//	cout << endl;
		//}
	}
	catch (GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	catch (...) {
		cout << "Error during optimization" << endl;
	}

    Log(LogSwitch::Szx::Framework) << "worker " << workerId << " ends." << endl;
    return status;
}
#pragma endregion Solver

}
