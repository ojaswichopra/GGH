#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace std;

// Circuit representation
typedef unordered_map<string, string> Circuit;

// Function to parse the circuit file and create the circuit representation
Circuit parseCircuitFile(const string &circuitFile)
{
    Circuit circuit;

    ifstream file(circuitFile);
    if (!file)
    {
        cout << "Failed to open circuit file: " << circuitFile << endl;
        return circuit;
    }

    string line;
    while (getline(file, line))
    {
        // Remove spaces from the expression
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        size_t pos = line.find('=');
        if (pos != string::npos)
        {
            string node = line.substr(0, pos);
            string expression = line.substr(pos + 1);

            // Store the expression as-is in the circuit representation
            circuit[node] = expression;
        }
    }

    return circuit;
}

// Function to evaluate the logical expression for a given node in the circuit
int evaluateExpression(const Circuit &circuit, const string &node, vector<int> &inputVector,unordered_map<string,int> &m)
{
    if(m.find(node)!=m.end()) return m[node];
    if (circuit.find(node) != circuit.end())
    {
        string expression = circuit.at(node);

        if (expression[0] == '~')
        {
            // NOT gate
            int result = evaluateExpression(circuit, expression.substr(1), inputVector,m);
            return m[node]=!result;
        }
        else if (expression.find('&') != string::npos)
        {
            // AND gate
            size_t pos = expression.find('&');
            string operand1 = expression.substr(0, pos);
            string operand2 = expression.substr(pos + 1);
            int result1 = evaluateExpression(circuit, operand1, inputVector,m);
            int result2 = evaluateExpression(circuit, operand2, inputVector,m);
            return m[node]=result1 && result2;
        }
        else if (expression.find('|') != string::npos)
        {
            // OR gate
            size_t pos = expression.find('|');
            string operand1 = expression.substr(0, pos);
            string operand2 = expression.substr(pos + 1);
            int result1 = evaluateExpression(circuit, operand1, inputVector,m);
            int result2 = evaluateExpression(circuit, operand2, inputVector,m);
            return m[node]=result1 || result2;
        }
        else if (expression.find('^') != string::npos)
        {
            // XOR gate
            size_t pos = expression.find('^');
            string operand1 = expression.substr(0, pos);
            string operand2 = expression.substr(pos + 1);
            int result1 = evaluateExpression(circuit, operand1, inputVector,m);
            int result2 = evaluateExpression(circuit, operand2, inputVector,m);
            return m[node]=result1 ^ result2;
        }
        else if (expression == "1" || expression == "0")
        {
            // Fault Site
            return m[node]=stoi(expression);
        }
    }
    else
    {
        // Input nodes
        if (node == "A")
            return m[node]=inputVector[0]; // Input A is 1
        else if (node == "B")
            return m[node]=inputVector[1]; // Input B is 0
        else if (node == "C")
            return m[node]=inputVector[2]; // Input C is 1
        else if (node == "D")
            return m[node]=inputVector[3]; // Input D is 1
    }

    cout << "Invalid node: " << node << endl;
    return m[node]=-1;
}

// Function to evaluate the circuit and get the output value
int evaluateCircuit(const Circuit &circuit, vector<int> &inputVector)
{
    unordered_map<string,int> m;
    string outputNode = "Z";
    return evaluateExpression(circuit, outputNode, inputVector,m);
}

// Function to generate all possible input vectors
vector<vector<int>> generateInputVectors(int numInputs)
{
    vector<vector<int>> inputVectors;
    int totalCombinations = pow(2, numInputs);

    for (int i = 0; i < totalCombinations; ++i)
    {
        vector<int> bits;
        string binary = bitset<4>(i).to_string();
        for (char bit : binary)
        {
            bits.push_back(bit - '0');
        }

        inputVectors.push_back(bits);
    }

    return inputVectors;
}

// Function to apply the fault at the given node
void applyFault(Circuit &circuit, const string &faultNode, const string &faultType)
{
    if (faultType == "SA0")
    {
        circuit[faultNode] = "0";
    }
    else if (faultType == "SA1")
    {
        circuit[faultNode] = "1";
    }
    else
    {
        cout << "Invalid fault type: " << faultType << endl;
    }
}

// Function to print the input vector and expected output to the output file
void printOutput(const vector<int> &inputVector, int expectedOutput, const string &outputFile)
{
    ofstream file(outputFile);
    if (!file)
    {
        cout << "Failed to open output file: " << outputFile << endl;
        return;
    }

    file << "[A,B,C,D] = [ ";
    for (int i : inputVector)
    {
        file << i << " ";
    }
    file << "], ";

    file << "Z = " << expectedOutput << endl;

    file.close();
}

int main()
{
    // Circuit file and fault details
    string circuitFile = "input.txt";
    string faultNode;
    string faultType;

    cout << "Enter Fault Node Location: ";
    cin >> faultNode;

    cout << "Enter FaultType (SA0/SA1): ";
    cin >> faultType;

    // Parse the circuit file
    Circuit circuit = parseCircuitFile(circuitFile);

    // Generate all possible input vectors
    vector<vector<int>> inputVectors = generateInputVectors(4);

    // Iterate through each input vector
    for (auto inputVector : inputVectors)
    {
        // Apply the fault at the given node
        Circuit faultyCircuit = circuit;
        applyFault(faultyCircuit, faultNode, faultType);

        // Simulate the circuit with fault-free inputs and record the expected output value
        
        int expectedOutput = evaluateCircuit(circuit, inputVector);

        // Simulate the circuit with the candidate input vector and the fault applied
        int output = evaluateCircuit(faultyCircuit, inputVector);

        // Check if the output value matches the expected output value
        if (output != expectedOutput)
        {
            // Print the potential input vector and the expected output to the output file
            printOutput(inputVector, output, "output.txt");
            return 0;
        }
    }
    // ATPG untestable fault
    ofstream file("output.txt");
    if (!file)
    {
        cout << "Failed to open output file: output.txt" << endl;
        return 0;
    }
    file << "Given fault is ATPG untestable fault" << endl;
    file.close();

    return 0;
}
