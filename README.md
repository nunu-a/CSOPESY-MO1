# CSOPESY Process Scheduler and CLI Emulator

A C++ command-line emulator that simulates a multi-core operating system process scheduler, complete with a command interpreter, process control blocks (PCBs), and a hardware resource emulator.

## 1. Project Overview
This emulator mimics a barebones operating system. It features a main command-line interface (CLI) that reads configurations, manages processes, and acts as a shell. A background worker thread acts as the CPU, executing pre-determined process instructions sequentially or via a Round-Robin scheduler based on the provided configuration. 

## 2. How to Compile and Run

### Prerequisites
* A C++17 compatible compiler (e.g., GCC, Clang, MSVC).
* Windows (required for the `cls` console clear command in `main.cpp`).

### Compilation
Using `g++`:
```bash
g++ main.cpp resource_emulator.cpp process.cpp -o csopesy -std=c++17 -pthread
