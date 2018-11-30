#include "Solver.h"


int main(int argc, char *argv[]) {
    return szx::Solver::Cli::run(argc, argv);
}


/* Copyright 2015, Gurobi Optimization, Inc. */

/* This example formulates and solves the following simple MIP model:

maximize    x + y  + 2z
subject to  x + 2y + 3z <= 4
x + y >= 1
x, y, z binary
*/

//#include "gurobi_c++.h"
//
//int main(int argc, char *argv[])
//{
//	try
//	{
//		GRBEnv env = GRBEnv();
//		GRBModel model = GRBModel(env);
//
//		// Set variables
//		GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
//		GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
//		GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");
//
//		model.update();
//
//		// Set objective function: maximize x + y + 2z
//		model.setObjective(x + y + 2 * z, GRB_MAXIMIZE);
//
//		// Set constraints: 
//		// x + 2y + 3z <= 4
//		// x + y >= 1
//		model.addConstr(x + 2 * y + 3 * z <= 4, "c0");
//		model.addConstr(x + y >= 1, "c1");
//
//		// Run the optimizion
//		model.optimize();
//
//		std::cout << x.get(GRB_StringAttr_VarName) << " "
//			<< x.get(GRB_DoubleAttr_X) << std::endl;
//		std::cout << y.get(GRB_StringAttr_VarName) << " "
//			<< y.get(GRB_DoubleAttr_X) << std::endl;
//		std::cout << z.get(GRB_StringAttr_VarName) << " "
//			<< z.get(GRB_DoubleAttr_X) << std::endl;
//
//		std::cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;
//
//	}
//	catch (GRBException e)
//	{
//		std::cout << "Error code = "
//			<< e.getErrorCode()
//			<< std::endl;
//		std::cout << e.getMessage() << std::endl;
//	}
//	catch (...)
//	{
//		std::cout << "Exception during optimization"
//			<< std::endl;
//	}
//	system("pause");
//	return 0;
//}