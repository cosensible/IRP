#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "Visualizer.h"

#include "../Solver/PbReader.h"
#include "../Solver/InventoryRouting.pb.h"


using namespace std;
using namespace szx;
using namespace pb;

int main(int argc, char *argv[]) {
	static constexpr double DefaultDoubleGap = 0.001;

	enum CheckerFlag {
		IoError = 0x0,
		FormatError = 0x1,
		SubtourExistenceError = 0x2,
		MultipleVisitError = 0x4,
		LoadDeliveryError = 0x8,
		QuantityReasonabilityError = 0x16
	};

	string inputPath = "..\\Deploy\\Instance\\Instances_lowcost_H3\\abs4n10.json";
	string outputPath = "C:\\Users\\gen\\Desktop\\abs4n10.json";

	if (argc > 1) {
		inputPath = argv[1];
	}
	else {
		//cerr << "input path: " << flush;
		//cin >> inputPath;
	}

	if (argc > 2) {
		outputPath = argv[2];
	}
	else {
		//cerr << "output path: " << flush;
		//cin >> outputPath;
	}

	pb::InventoryRouting::Input input;
	if (!load(inputPath, input)) { return ~CheckerFlag::IoError; }

	pb::InventoryRouting::Output output;
	ifstream ifs(outputPath);
	if (!ifs.is_open()) { return ~CheckerFlag::IoError; }
	string submission;
	getline(ifs, submission); // skip the first line.
	ostringstream oss;
	oss << ifs.rdbuf();
	jsonToProtobuf(oss.str(), output);

	// check solution.
	int error = 0;

	int nodeNum = input.nodes_size();
	const auto &allRoutes(*output.mutable_allroutes());
	const auto &nodes(*input.mutable_nodes());

	// check multiple visit.
	for (auto pr = allRoutes.begin(); pr != allRoutes.end(); ++pr) {
		vector<int> visitedTimes(nodeNum, 0);
		for (auto vr = pr->routes().begin(); vr != pr->routes().end(); ++vr) {
			for (auto dl = vr->deliveries().begin(); dl != vr->deliveries().end(); ++dl) {
				++visitedTimes[dl->node()];
			}
		}
		for (int i = 1; i < nodeNum; ++i) {
			if (visitedTimes[i] > 1) {
				error |= CheckerFlag::MultipleVisitError;
			}
		}
	}

	// check load-deliver equality.
	for (auto pr = allRoutes.begin(); pr != allRoutes.end(); ++pr) {
		for (auto vr = pr->routes().begin(); vr != pr->routes().end(); ++vr) {
			int totalQuantity = 0;
			for (auto dl = vr->deliveries().begin(); dl != vr->deliveries().end(); ++dl) {
				totalQuantity += dl->quantity();
			}
			if (abs(totalQuantity) > DefaultDoubleGap) {
				error |= CheckerFlag::LoadDeliveryError;
			}
		}
	}

	// check rest quantity
	double holdingCost = 0;
	vector<int> restQuntity(nodeNum, 0);
	for (auto i = nodes.begin(); i != nodes.end(); ++i) {
		restQuntity[i->id()] = i->initquantity();
		holdingCost += i->holidingcost() * i->initquantity();
	}
	for (auto pr = allRoutes.begin(); pr != allRoutes.end(); ++pr) {
		for (auto vr = pr->routes().begin(); vr != pr->routes().end(); ++vr) {
			for (auto dl = vr->deliveries().begin(); dl != vr->deliveries().end(); ++dl) {
				restQuntity[dl->node()] += dl->quantity();
			}
		}
		for (auto i = nodes.begin(); i != nodes.end(); ++i) {
			int id = i->id();
			if (id > 0) {
				if (lround(restQuntity[id]) > i->capacity()) { error |= CheckerFlag::QuantityReasonabilityError; }
				restQuntity[id] -= i->unitdemand();
				if (lround(restQuntity[id]) < i->minlevel()) { error |= CheckerFlag::QuantityReasonabilityError; }
			}
			else if (id == 0) {
				if (lround(restQuntity[id]) < i->minlevel()) { error |= CheckerFlag::QuantityReasonabilityError; }
				restQuntity[id] -= i->unitdemand();
				if (lround(restQuntity[id]) > i->capacity()) { error |= CheckerFlag::QuantityReasonabilityError; }
			}
			holdingCost += i->holidingcost() * restQuntity[id];
		}
	}

	// check objective.
	auto costOfEdge = [](const pb::Node &i, const pb::Node &j)->double {
		return round(hypot(i.x() - j.x(), i.y() - j.y()));
	};
	double routingCost = 0;
	for (auto pr = allRoutes.begin(); pr != allRoutes.end(); ++pr) {
		vector<int> visitedTimes(nodeNum, 0);
		for (auto vr = pr->routes().begin(); vr != pr->routes().end(); ++vr) {
			if (vr->deliveries_size() <= 1) { continue; }
			int preNode = 0;
			for (auto dl = vr->deliveries().begin(); dl != vr->deliveries().end(); ++dl) {
				routingCost += costOfEdge(nodes[preNode], nodes[dl->node()]);
				preNode = dl->node();
			}
		}
	}

	// visualize solution.
	// TODO[qym][5]: 
	double returnCode = (error == 0) ? (routingCost + holdingCost) : ~error;
	cout << "Return Code: " << returnCode << "\n\n" << endl;
	return (int)(returnCode * 1000);
}
