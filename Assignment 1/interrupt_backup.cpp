#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>
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

// Function to simulate execution and log the event
void logEvent(std::ofstream &output, int time, int duration, const std::string &eventType)
{
    output << time << ", " << duration << ", " << eventType << std::endl;
}

// Main simulator function
int main()
{
    // Load the vector table from vector_table.txt
    std::map<int, std::string> vectorTable = loadVectorTable("./vector_table.txt");

    // Open trace.txt for reading and execution.txt for writing
    std::ifstream traceFile("trace1.txt");
    std::ofstream execLogFile("execution1.txt");

    if (!traceFile.is_open())
    {
        std::cerr << "Error opening trace file." << std::endl;
        return 1;
    }
    if (!execLogFile.is_open())
    {
        std::cerr << "Error opening execution log file." << std::endl;
        return 1;
    }

    std::string line;
    int currentTime = 0;

    // Process each line of the trace file
    while (getline(traceFile, line))
    {
        std::stringstream ss(line);
        std::string eventType;
        int eventValue, duration;

        ss >> eventType >> eventValue >> duration;

        // Process event based on type
        if (eventType.find("CPU") != std::string::npos)
        {
            logEvent(execLogFile, currentTime, eventValue, "CPU execution");
            currentTime += eventValue; // Advance the time
        }
        else if (eventType.find("SYSCALL") != std::string::npos)
        {
            // Simulate the system call and interrupt handling

            // Switch to kernel mode
            logEvent(execLogFile, currentTime, 1, "switch to kernel mode");
            currentTime += 1;

            // Save context
            int contextSaveTime = rand() % 3 + 1; // Random between 1-3 ms
            logEvent(execLogFile, currentTime, contextSaveTime, "context saved");
            currentTime += contextSaveTime;

            std::string isrAddress = vectorTable[eventValue];

            // Find vector and ISR address
            logEvent(execLogFile, currentTime, 1, "find vector " + std::to_string(eventValue) + " in memory position " + isrAddress);
            currentTime += 1;

            logEvent(execLogFile, currentTime, 1, "load address " + isrAddress + " into the PC");
            currentTime += 1;

            // Simulate ISR execution: exact tasks and their breakdown
            int time_ISR = duration / 3 + 1;
            int time_transfer_data = time_ISR / 2 + 1;
            int time_check_for_errors = duration - time_transfer_data - time_ISR;

            logEvent(execLogFile, currentTime, time_ISR, "SYSCALL: run the ISR");
            currentTime += time_ISR;

            logEvent(execLogFile, currentTime, time_transfer_data, "transfer data");
            currentTime += time_transfer_data;

            logEvent(execLogFile, currentTime, time_check_for_errors, "check for errors");
            currentTime += time_check_for_errors;

            // IRET (return from interrupt)
            logEvent(execLogFile, currentTime, 1, "IRET");
            currentTime += 1;
        }
        else if (eventType.find("END_IO") != std::string::npos)
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

            // Find vector and ISR address
            logEvent(execLogFile, currentTime, 1, "find vector " + std::to_string(eventValue) + " in memory position " + isrAddress);
            currentTime += 1;

            logEvent(execLogFile, currentTime, 1, "load address " + isrAddress + " into the PC");
            currentTime += 1;

            logEvent(execLogFile, currentTime, duration, "END_IO");
            currentTime += duration;

            // IRET after END_IO
            logEvent(execLogFile, currentTime, 1, "IRET");
            currentTime += 1;
        }
    }

    // Close the files
    traceFile.close();
    execLogFile.close();

    return 0;
}
