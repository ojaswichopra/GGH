#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace std;

// Circuit representation
typedef unordered_map<string, string> Circuit;

// Function to parse the circuit file and create the circuit representation
Circuit parseCircuitFile(const string &ipFile)
{
    Circuit circuit;

    ifstream file(ipFile);
    if (!file)
    {
        cout << "Failed to open circuit file: " << ipFile << endl;
        return circuit;
    }

    string line;
    while (getline(file, line))
    {
        // Remove spaces from the logicExpression
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        size_t pos = line.find('=');
        if (pos != string::npos)
        {
            string node = line.substr(0, pos);
            string logicExpression = line.substr(pos + 1);

            // Store the logicExpression as-is in the circuit representation
            circuit[node] = logicExpression;
        }
    }

    return circuit;
}

// Function to generate all possible input vectors
vector<vector<int>> generate(int numInputs)
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

// Function to evaluate the logical logicExpression for a given node in the circuit
int evalExpression(const Circuit &circuit, const string &node, vector<int> &inputVector, unordered_map<string, int> &m)
{
    if (m.find(node) != m.end())
        return m[node];
    if (circuit.find(node) != circuit.end())
    {
        string logicExpression = circuit.at(node);

        if (logicExpression[0] == '~')
        {
            // NOT gate
            int result = evalExpression(circuit, logicExpression.substr(1), inputVector, m);
            return m[node] = !result;
        }
        else if (logicExpression.find('&') != string::npos)
        {
            // AND gate
            size_t pos = logicExpression.find('&');
            string operand1 = logicExpression.substr(0, pos);
            string operand2 = logicExpression.substr(pos + 1);
            int result1 = evalExpression(circuit, operand1, inputVector, m);
            int result2 = evalExpression(circuit, operand2, inputVector, m);
            return m[node] = result1 && result2;
        }
        else if (logicExpression.find('|') != string::npos)
        {
            // OR gate
            size_t pos = logicExpression.find('|');
            string operand1 = logicExpression.substr(0, pos);
            string operand2 = logicExpression.substr(pos + 1);
            int result1 = evalExpression(circuit, operand1, inputVector, m);
            int result2 = evalExpression(circuit, operand2, inputVector, m);
            return m[node] = result1 || result2;
        }
        else if (logicExpression.find('^') != string::npos)
        {
            // XOR gate
            size_t pos = logicExpression.find('^');
            string operand1 = logicExpression.substr(0, pos);
            string operand2 = logicExpression.substr(pos + 1);
            int result1 = evalExpression(circuit, operand1, inputVector, m);
            int result2 = evalExpression(circuit, operand2, inputVector, m);
            return m[node] = result1 ^ result2;
        }
        else if (logicExpression == "1" || logicExpression == "0")
        {
            // Fault Site
            return m[node] = stoi(logicExpression);
        }
    }
    else
    {
        // Input nodes
        if (node == "A")
            return m[node] = inputVector[0]; // Input A is 1
        else if (node == "B")
            return m[node] = inputVector[1]; // Input B is 0
        else if (node == "C")
            return m[node] = inputVector[2]; // Input C is 1
        else if (node == "D")
            return m[node] = inputVector[3]; // Input D is 1
    }

    cout << "Invalid node: " << node << endl;
    return m[node] = -1;
}

// Function to evaluate the circuit and get the Op value
int evalCircuit(const Circuit &circuit, vector<int> &inputVector)
{
    unordered_map<string, int> m;
    string outputNode = "Z";
    return evalExpression(circuit, outputNode, inputVector, m);
}

// Function to print the input vector and expected Op to the Op file
void printToOutput(const vector<int> &inputVector, int expectedOp, const string &outputFile)
{
    ofstream file(outputFile);
    if (!file)
    {
        cout << "Failed to open Op file: " << outputFile << endl;
        return;
    }

    file << "[A,B,C,D] = [ ";
    for (int i : inputVector)
    {
        file << i << " ";
    }
    file << "], ";

    file << "Z = " << expectedOp << endl;

    file.close();
}

int main()
{
    // Circuit file and fault details
    string ipFile = "input.txt";
    string faultNode;
    string faultType;

    cout << "Enter Fault Node Location: ";
    cin >> faultNode;

    cout << "Enter FaultType (SA0/SA1): ";
    cin >> faultType;

    // Parse the circuit file
    Circuit circuit = parseCircuitFile(ipFile);

    // Generate all possible input vectors
    vector<vector<int>> inputVectors = generate(4);

    // Apply the fault at the given node
    Circuit faultyCircuit = circuit;
    if (faultType == "SA0")
    {
        faultyCircuit[faultNode] = "0";
    }
    else if (faultType == "SA1")
    {
        faultyCircuit[faultNode] = "1";
    }
    else
    {
        cout << "Invalid fault type: " << faultType << endl;
        return 0;
    }

    // Iterate through each input vector
    for (auto inputVector : inputVectors)
    {
        // Simulate the circuit with fault-free inputs and record the expected Op value
        int expectedOp = evalCircuit(circuit, inputVector);

        // Simulate the circuit with the candidate input vector and the fault applied
        int Op = evalCircuit(faultyCircuit, inputVector);

        // Check if the Op value matches the expected Op value
        if (Op != expectedOp)
        {
            // Print the potential input vector and the expected Op to the Op file
            printToOutput(inputVector, Op, "output.txt");
            return 0;
        }
    }
    // ATPG untestable fault
    ofstream file("output.txt");
    if (!file)
    {
        cout << "Failed to open Op file: Op.txt" << endl;
        return 0;
    }
    file << "Given fault is ATPG untestable fault" << endl;
    file.close();

    return 0;
}
