CUinSPACE Simulated Flight - README
Overview
This is the CUinSPACE Simulated Flight program! This application is designed to simulate the management of rocket subsystems using a multithreaded architecture. The simulation involves multiple systems consuming and producing resources while a central manager oversees the overall operation. The program demonstrates key programming concepts, including dynamic memory management, multithreading, synchronization using semaphores, and priority-based event handling.

This README will guide you through compiling, running, and understanding the program.

Instructions to Compile the Program
To compile the program, you will need a linux machine that has a GCC compiler. Follow these steps to build and run the program:

Ensure All Source Files Are in the Same Directory:

The following files should be present:

main.c, system.c, manager.c, resource.c, event.c

Header file: defs.h

Makefile

Compile Using make:

Run the following command:
    - make


This will create an executable file named cuinspace.

Run the Program:

Execute the compiled program with:
    - ./cuinspace


Clean Up Build Artifacts:

To remove object files and the executable:
    - make clean


Optional Debugging and Memory Check:

To test for memory leaks and invalid memory usage, use:
    - valgrind --leak-check=full --track-origins=yes ./cuinspace


Citations
The development of this project was supported by various learning and debugging resources. Below are the key sources referenced during the creation of CUinSPACE Simulated Flight:

COMP2401 Course Materials: Lecture notes, assignments, and tutorials.

Valgrind User Manual:

Valgrind – Tool for memory debugging.

Event Queue Implementations:

General examples of priority queues and linked lists from online coding platforms.

Stack Overflow: Helpful for troubleshooting complex memory and threading issues.

Contributors
We, the following individuals contributed to the development of this project:

Roshan Kanagarajah (101311484)

Mohammad Saad (101306472)

Questions and Assumptions
Input and Output Behavior
Input Format Assumptions:

The program assumes that the data for resources and systems is correctly formatted when loaded. For example, all resource names are strings, and initial amounts are integers.

Event Queue Processing:

Events are processed in a priority-based manner. If multiple events have the same priority, they are processed in the order they were added.

System Status Changes:

Systems automatically adjust their statuses (e.g., SLOW, FAST, TERMINATE) based on resource availability and events.

Debugging Support
Debug Messages:

Debugging statements are included throughout the program to trace execution and identify issues. These messages can be enabled or disabled by toggling the DEBUG macro.

Edge Cases
Critical Resource Depletion:

If critical resources like oxygen or fuel are fully depleted, the program terminates the simulation with a relevant message.

Resource Capacity Management:

Systems respect the maximum capacity of resources and handle overflow gracefully by generating events.

Usage Tips - Helpful things we found when running!
Debugging Mode:

Enable detailed debug logs by ensuring the DEBUG macro is set to 1 in the source files.

Disable debugging by setting DEBUG to 0 before final submission or production use.

Simulating with Different Data:

The simulation can be customized by modifying the load_data function in main.c to add new resources or systems with different characteristics.

Error Handling:

The program includes detailed error messages for invalid operations, such as failing to allocate memory or reaching invalid system states.

Project Specifications
Development Environment:

Developed and tested on a Unix-based system using GCC as the compiler.

Multithreaded behavior tested using standard threading libraries available in the development environment.

File Dependencies:

Ensure all source files (main.c, manager.c, system.c, resource.c, event.c) and the header file (defs.h) are present in the same directory for successful compilation.

Threading and Synchronization:

Each system operates in its own thread, synchronized with semaphores to ensure safe access to shared data (e.g., EventQueue, Resource).

Stress Testing:

Test cases include scenarios with multiple high-priority events, concurrent resource usage, and simultaneous system activity to verify thread safety and event handling.

Thank you so much for using the CUinSPACE Simulated Flight program! If you have any questions let us know right away and, feel free to contact us for further information or support.