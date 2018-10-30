/*
Name: Brendan Cassidy, Diego Alcantra, Evan Weiler, Logan Knott
Project: ECE 474/574 Project 2
Description:
This project is a parser for Behavioral netlists that will output
the top Module verilog file
*/
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "Variable.hpp"

using namespace std;

double critPathArray[12][6] = {
    {2.616, 2.644, 2.879, 3.061, 3.602, 3.966},
    {2.704, 3.713, 4.924, 5.638, 7.270, 9.566},
    {3.024, 3.412, 4.890, 5.569, 7.253, 9.566},
    {2.438, 2.651, 7.453, 7.811, 12.395, 15.354},
    {3.031, 3.934, 5.949, 6.256, 7.264, 8.416},
    {4.083, 4.115, 4.815, 5.623, 8.079, 8.766},
    {3.644, 4.007, 5.178, 6.460, 8.819, 11.095},
    {3.614, 3.980, 5.152, 6.549, 8.565, 11.220},
    {0.619, 2.144, 15.349, 33.093, 86.312, 243.233},
    {0.758, 2.149, 16.078, 35.563, 88.142, 250.583},
    {1.792, 2.218, 3.111, 3.471, 4.347, 6.200},
    {1.792, 2.218, 3.108, 3.701, 4.685, 6.503},
};
string callOperator(vector<Variable> variables, string operand, int num);
double calcOperationTime(vector<Variable> variables, string operand);

int main(int argc, char *argv[]) {
    
    if(argc != 3) {
        cout << "Usage: main inputFile outputFile" << endl;
        return EXIT_FAILURE;
    }
    
	int i, pos, operandCount = 0, count = 0;
	bool operation, validVar = false;

	//Step 1: Read file input line by line
	ifstream iFile;
    ofstream oFile;
	string line, operand, currType, bitWidth, varNames, currName;
	string delimiter = ", ";
    string modules = "";

	Variable tempVar;
	vector<Variable> allVariables;
	vector<Variable> currOperand;
    
    double critPath = 0.0;

	//iFile.open("C:/Users/cassi/Documents/School/UofA7th Sem/CSCV 352/ECE474Project2/ECE474Project2/474a_circuit1.txt");
	//iFile.open("C:/Users/evanj/OneDrive/SchoolWork/4Senior/574/ECE474Project2/assignment_2_circuits/474a_circuit1.txt");
	iFile.open(argv[1]); //THIS IS THE ONE BOYS
	//iFile.open("Test.txt");

	if (iFile.is_open()) {
		while (!iFile.eof()) {
			string val;
			count = 0;
			operation = false;
			validVar = false;
			currOperand.clear();
			operand="="; //Assume Reg
			
			getline(iFile, line); //Pull in line

			//Decompisition of Input, Output, Wire
			if (line.find("=") == string::npos && !line.empty()) {
				istringstream lineStream(line);
				lineStream >> currType >> bitWidth;
				getline(lineStream, varNames);
				varNames = varNames.substr(1, varNames.length() - 1);

				size_t begin = bitWidth.find_first_of("01234456789");

				while ((pos = varNames.find(delimiter)) != string::npos) {
					tempVar.setVarType(currType);
					tempVar.setBitWidth(stoi(bitWidth.substr(begin, bitWidth.length() - 1)));

					currName = varNames.substr(0, pos);
					varNames.erase(0, pos + delimiter.length());
					tempVar.setName(currName);

					allVariables.push_back(tempVar);
				}

				if (!varNames.empty()) {
					tempVar.setVarType(currType);
					tempVar.setBitWidth(stoi(bitWidth.substr(begin, bitWidth.length() - 1)));
					tempVar.setName(varNames);
					allVariables.push_back(tempVar);
				}
			}
			else {
				istringstream opStream(line);
				if (line.compare("") == 0) {
					continue;
				}
				//Decompisition of Operator
				while (opStream >> val) {
					//Check the variables validity
					if (count == 0 || count == 2 || count == 4 || count == 6) {
						for (i = 0; i < allVariables.size(); i++) {
							//Make sure var exists
							if (allVariables.at(i).getName().compare(val) == 0) {
								validVar = true;
								currOperand.push_back(allVariables[i]);
								break;
							}
						}
						if (validVar == false) 
							return EXIT_FAILURE;
					}
					//Get the operand (= for reg, +,-... for others)
					if (count==1 || count == 3)
						operand = val;
					count += 1;
				}
				modules += "      " + callOperator(currOperand, operand, operandCount);
                critPath+= calcOperationTime(currOperand, operand); //FIXME: not sure if crit path is some of all opeations or just longest op
				operandCount += 1;
			}
            //oFile.close();
			/*for (auto i : allVariables) {
					i.toString();
			}*/
			//Call Operand function
		}
	}
	else {
		return -1;
	}
    //writing to output file
    oFile.open(argv[2]);
    oFile << "'timescale 1ns / 1ps" << endl << endl;
    oFile << "module TopModule(";
    string tempString = "";
    for(Variable var : allVariables) {
        if (var.getVarType().compare("input") == 0 || var.getVarType().compare("output") == 0) {
            tempString += var.getName() + ", ";
        }
    }
    tempString = tempString.substr(0, tempString.length() - 2);
    oFile << tempString << ");" << endl;
    for(Variable var : allVariables) {
        oFile << "   " << var.getVarType();
        if (var.getVarType().compare("output") == 0)
            oFile << " reg ";
        oFile << "[" << var.getBitWidth() << ":0] " << var.getName() << ";" << endl;
    }
    oFile << endl << "   always @(";
    tempString = "";
    for (Variable var : allVariables) {
        if (var.getVarType().compare("input") == 0)
            tempString += var.getName() + ", ";
    }
    tempString = tempString.substr(0, tempString.length() - 2);
    oFile << tempString << ") begin" << endl;
    oFile << modules;
    oFile << "   end" << endl << endl;
    oFile << "endModule" << endl << endl;
    oFile << "//Critical Path : " << critPath << "ns" << endl;
    
	return 0;
};

string callOperator(vector<Variable> variables, string operand, int num) {
    string toReturn;
    if (operand.compare("=") == 0) {    //REG
        toReturn = "REG reg" + std::to_string(num) + "(" + variables.at(1).getName() + ", " + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare("+") == 0)    { //ADD
        toReturn = "ADD add" + std::to_string(num) + "(" + variables.at(1).getName() + ", " + variables.at(2).getName() + ", " + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare("-") == 0) {    //SUB
        toReturn = "SUB sub" + std::to_string(num) + "(" + variables.at(1).getName() + ", " + variables.at(2).getName() + ", " + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare("*") == 0) {    //MUL
        toReturn = "MUL mul" + std::to_string(num) + "(" + variables.at(1).getName() + ", " + variables.at(2).getName() + ", " + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare(">") == 0) {    //COMP (gt output)
        toReturn = "COMP comp" + to_string(num) + "(" + variables.at(1).getName() + ", " + variables.at(2).getName() + ", " + variables.at(0).getName() + ", 0, 0);\n";
    }
    else if (operand.compare("<") == 0) {    //COMP (lt output)
        toReturn = "COMP comp" + to_string(num) + "(" + variables.at(1).getName() + ", " + variables.at(2).getName() + ", 0, " + variables.at(0).getName() + ", 0);\n";
    }
    else if (operand.compare("==") == 0) {    //COMP (eq output)
        toReturn = "COMP comp" + to_string(num) + "(" + variables.at(1).getName() + ", " + variables.at(2).getName() + ", 0, 0, " + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare("?") == 0) {    //MUX2x1
        toReturn = "MUX2x1 mux" + to_string(num) + "(" + variables.at(2).getName() + ", " + variables.at(3).getName() + ", " + variables.at(1).getName() + ", " + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare(">>")== 0) {    //SHR
        toReturn = "SHR shr" + to_string(num) + "(" + variables.at(1).getName() + "," + variables.at(2).getName() + "," + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare("<<")== 0) {    //SHL
        toReturn = "SHL shl" + to_string(num) + "(" + variables.at(1).getName() + "," + variables.at(2).getName() + "," + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare("/") == 0) {    //DIV
        toReturn = "DIV div" + to_string(num) + "(" + variables.at(1).getName() + "," + variables.at(2).getName() + "," + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare("%") == 0) {    //MOD
        toReturn = "MOD mod" + to_string(num) + "(" + variables.at(1).getName() + "," + variables.at(2).getName() + "," + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare("+ 1") == 0) {    //INC UPDATE: needs to be tested
        toReturn = "INC inc" + to_string(num) + "(" + variables.at(1).getName() + "," + variables.at(0).getName() + ");\n";
    }
    else if (operand.compare("- 1") == 0)    { //DEC UPDATE: needs to be tested
        toReturn = "DEC dec" + to_string(num) + "(" + variables.at(1).getName() + "," + variables.at(0).getName() + ");\n";
    }
    else {
        toReturn = "FAILED";
    }
    return toReturn;
    
}

double calcOperationTime(vector<Variable> variables, string operand) {
    int maxBitWidth;
    int operationIndex;
    int operandCount = variables.size();
    if(operandCount == 3)
        maxBitWidth = std::max(std::max(variables.at(0).getBitWidth(), variables.at(1).getBitWidth()), variables.at(2).getBitWidth());
    else
        maxBitWidth = std::max(variables.at(0).getBitWidth(), variables.at(1).getBitWidth());
    
    //mapping of operand to operation
    if (operand.compare("=") != 0) { //temporary fix until register assingment is handled
        if (operand.compare("+") == 0) operationIndex = 1;
        else if (operand.compare("-") == 0) operationIndex = 2;
        else if (operand.compare("*") == 0) operationIndex = 3;
        else if (operand.compare("==") == 0 || operand.compare(">") == 0 || operand.compare("<") == 0) operationIndex = 4;
        else if (operand.compare("?") == 0) operationIndex = 5;
        else if (operand.compare(">>") == 0) operationIndex = 6;
        else if (operand.compare("<<") == 0) operationIndex = 7;
        else if (operand.compare("/") == 0) operationIndex = 8;
        else if (operand.compare("%") == 0) operationIndex = 9;
        else if (operand.compare("+ 1") == 0) operationIndex = 10; //FIXME: change this to deal with INC UPDATE: needs to be tested
        else if (operand.compare("- 1") == 0) operationIndex = 11; //FIXME: change this to deal with DEC UPDATE: needs to be tested
    } //FIXME handle register assignments
    else
        return 0;
    
    double critPath = critPathArray[operationIndex][maxBitWidth] + critPathArray[0][maxBitWidth]; //sum of operation mapped to array above plus register assignment for maxBitWidth
    
    return critPath;
}

