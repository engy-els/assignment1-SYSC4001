#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>

// Function to load the vector table from a file
std::map<int, std::string> loadVectorTable(const std::string &filename)
{
    std::map<int, std::string> vectorTable;
    std::ifstream file(filename);
    std::string line;
    int interruptNumber = 0;

    if (!file.is_open())
    {
        std::cerr << "Error opening vector table file: " << filename << std::endl;
        return vectorTable; // Return empty map if file cannot be opened
    }

    while (getline(file, line))
    {
        std::istringstream iss(line);
        std::string isrAddress;
        if (iss >> isrAddress)
        {
            vectorTable[interruptNumber] = isrAddress;
            interruptNumber++; // Auto increment for each interrupt number
        }
        else
        {
            std::cerr << "Error reading line: " << line << std::endl;
        }
    }

    file.close();
    return vectorTable;
}

// Function to log events
void logEvent(std::ofstream &output, int time, int duration, const std::string &eventType)
{
    output << time << ", " << duration << ", " << eventType << std::endl;
}

// Function to process SYSCALL with simplified distribution
void processSYSCALL(int eventValue, int duration, std::ofstream &execLogFile, int &currentTime, std::map<int, std::string> vectorTable)
{
    int third = duration / 3;     // Split the duration into three parts
    int remainder = duration % 3; // Handle the remainder by adding it to ISR

    int isrDuration = third + remainder;
    int transferDuration = third;
    int errorCheckDuration = third;

    // Simulate the system call and interrupt handling

    // Switch to kernel mode
    logEvent(execLogFile, currentTime, 1, "switch to kernel mode");
    currentTime += 1;

    // Save context
    int contextSaveTime = rand() % 3 + 1; // Random between 1-3 ms
    logEvent(execLogFile, currentTime, contextSaveTime, "context saved");
    currentTime += contextSaveTime;

    std::string isrAddress = vectorTable[eventValue];

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << std::hex << (eventValue * 2);
    std::string hexString = ss.str();

    // Find vector and ISR address
    logEvent(execLogFile, currentTime, 1, "find vector " + std::to_string(eventValue) + " in memory position 0X" + hexString); // eventValue*2 in hex);
    currentTime += 1;

    logEvent(execLogFile, currentTime, 1, "load address " + isrAddress + " into the PC");
    currentTime += 1;

    logEvent(execLogFile, currentTime, isrDuration, "SYSCALL: run the ISR");
    currentTime += isrDuration;

    logEvent(execLogFile, currentTime, transferDuration, "transfer data");
    currentTime += transferDuration;

    logEvent(execLogFile, currentTime, errorCheckDuration, "check for errors");
    currentTime += errorCheckDuration;

    logEvent(execLogFile, currentTime, 1, "IRET");
    currentTime += 1;
}

// Function to process CPU events
void processCPU(int cpu_duration, std::ofstream &execLogFile, int &currentTime)
{
    logEvent(execLogFile, currentTime, cpu_duration, "CPU execution");
    currentTime += cpu_duration;
}

// Function to process END_IO events
void processEND_IO(int eventValue, int duration, std::ofstream &execLogFile, int &currentTime, std::map<int, std::string> vectorTable)
{
    std::string isrAddress = vectorTable[eventValue];

    // Check priority of the interrupt
    logEvent(execLogFile, currentTime, 1, "check priority of interrupt");
    currentTime += 1;

    logEvent(execLogFile, currentTime, 1, "check if masked");
    currentTime += 1;

    logEvent(execLogFile, currentTime, 1, "switch to kernel mode");
    currentTime += 1;

    // Save context
    int contextSaveTime = rand() % 3 + 1; // Random between 1-3 ms
    logEvent(execLogFile, currentTime, contextSaveTime, "context saved");
    currentTime += contextSaveTime;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << std::hex << (eventValue * 2);
    std::string hexString = ss.str();

    // Find vector and ISR address
    logEvent(execLogFile, currentTime, 1, "find vector " + std::to_string(eventValue) + " in memory position 0X" + hexString);
    currentTime += 1;

    logEvent(execLogFile, currentTime, 1, "load address " + isrAddress + " into the PC");
    currentTime += 1;

    logEvent(execLogFile, currentTime, duration, "END_IO");
    currentTime += duration;

    // IRET after END_IO
    logEvent(execLogFile, currentTime, 1, "IRET");
    currentTime += 1;
}

// Function to process a single trace file
void processTraceFile(const std::string &traceFileName, const std::string &execFileName, std::map<int, std::string> &vectorTable)
{
    std::ifstream traceFile(traceFileName);
    std::ofstream execLogFile(execFileName);

    if (!traceFile.is_open())
    {
        std::cerr << "Error opening trace file: " << traceFileName << std::endl;
        return;
    }
    if (!execLogFile.is_open())
    {
        std::cerr << "Error opening execution log file: " << execFileName << std::endl;
        return;
    }

    std::string line;
    int currentTime = 0;

    // Process each line of the trace file
    while (getline(traceFile, line))
    {
        std::stringstream ss(line);
        std::string eventType;
        int eventValue, duration;
        char comma;

        ss >> eventType >> eventValue >> comma >> duration;

        // Process event based on type
        if (eventType.find("CPU") != std::string::npos)
        {
            processCPU(eventValue, execLogFile, currentTime);
        }
        else if (eventType.find("SYSCALL") != std::string::npos)
        {
            processSYSCALL(eventValue, duration, execLogFile, currentTime, vectorTable);
        }
        else if (eventType.find("END_IO") != std::string::npos)
        {
            processEND_IO(eventValue, duration, execLogFile, currentTime, vectorTable);
        }
        else
        {
            std::cerr << "Unknown event type: " << eventType << "\n";
        }
    }

    // Close the files
    traceFile.close();
    execLogFile.close();
}

// Main simulator function to handle multiple trace files
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <trace_file_name>" << std::endl;
        return 1;
    }

    std::string traceFileName = argv[1];

    // Find the number from the trace file name (assuming the format is trace<number>.txt)
    size_t tracePos = traceFileName.find("trace");
    size_t dotPos = traceFileName.find(".txt");
    if (tracePos == std::string::npos || dotPos == std::string::npos) {
        std::cerr << "Error: Invalid trace file name format" << std::endl;
        return 1;
    }

    // Extract the number part
    std::string number = traceFileName.substr(tracePos + 5, dotPos - (tracePos + 5));

    // Create the execution file name based on the number
    std::string execFileName = "execution" + number + ".txt";

    // Load the vector table from vector_table.txt
    std::map<int, std::string> vectorTable = loadVectorTable("./vector_table.txt");

    std::cout << "Processing " << traceFileName << " -> " << execFileName << std::endl;
    processTraceFile(traceFileName, execFileName, vectorTable);

    return 0;
}