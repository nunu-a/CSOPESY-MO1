# CSOPESY Process Scheduler and CLI

A command-line emulator of an operating system's process scheduler. It runs a simulated CPU on a background thread, executes simulated processes on several cores, and lets the user create, observe, and report on those processes from a console.

**Course:** CSOPESY Major Output: Process Scheduler and CLI
**Group:** [Group number and member names]
**Developer:** Azzam Alonto

---

## Features

- Main menu console: `initialize`, `screen`, `scheduler-start`, `scheduler-stop`, `report-util`, `exit`
- Process screens: `process-smi` and `exit`
- Named processes (`screen -s`) and generated dummy processes (`scheduler-start`)
- Round-robin (`rr`) and first-come, first-served (`fcfs`) scheduling
- Simulated instructions: `PRINT`, `DECLARE`, `ADD`, `SUBTRACT`, `SLEEP`, and nested `FOR` loops
- Busy-wait delay between instructions (`delay-per-exec`)
- CPU utilization report on screen and in `csopesy-log.txt`

## Project Structure

| File | Purpose |
|------|---------|
| `main.cpp` | Main menu, command parsing, and screen sessions |
| `config.h` | `Config` struct and `loadConfig()` for `config.txt` |
| `utils.h` | `currentTimestamp()` helper |
| `process.h` / `process.cpp` | `Process` (process control block), instructions, loops, and random instruction generation |
| `resource_emulator.h` / `resource_emulator.cpp` | Scheduler: cores, ready queue, CPU cycle, and reports |
| `config.txt` | Scheduler and process settings |

## Requirements

- Windows
- Visual Studio 2022 (MSVC)
- C++14 or later (the Visual Studio default)

The code uses `localtime_s` and `std::system("cls")`, which are Windows and MSVC features.

## Build and Run

1. In Visual Studio 2022, create a new **Empty Project (C++)**.
2. Add all `.h` and `.cpp` files from the table above.
3. Set the platform to **x64** and the configuration to **Debug**.
4. Place `config.txt` in the project folder.
5. Set **Project → Properties → Configuration Properties → Debugging → Working Directory** to `$(ProjectDir)`.
6. Build with **Ctrl+Shift+B**, then run with **Ctrl+F5**.

## Configuration

`config.txt` holds space-separated name and value pairs:

```
num-cpu 16
scheduler "rr"
quantum-cycles 5
batch-process-freq 1
min-ins 5000
max-ins 5000
delay-per-exec 2
```

| Parameter | Meaning |
|-----------|---------|
| `num-cpu` | Number of simulated CPU cores |
| `scheduler` | `"rr"` for round-robin, or `"fcfs"` for first-come, first-served |
| `quantum-cycles` | Time slice in CPU cycles for round-robin. No effect on FCFS |
| `batch-process-freq` | Creates one dummy process every this many CPU cycles (`scheduler-start`) |
| `min-ins` | Minimum number of instruction lines per process |
| `max-ins` | Maximum number of instruction lines per process |
| `delay-per-exec` | Busy-wait cycles before each instruction. `0` means one instruction per cycle |

If a name is missing from the file, its default is used: `num-cpu 4`, `scheduler "rr"`, `quantum-cycles 5`, `batch-process-freq 1`, `min-ins 1000`, `max-ins 2000`, `delay-per-exec 0`.

## Commands

Type commands at the `root:\>` prompt.

### Main menu

| Command | Description |
|---------|-------------|
| `initialize` | Reads `config.txt` and starts the CPU thread. Must run before any other command except `exit` |
| `screen -s <name>` | Creates a process named `<name>` with random instructions and opens its screen |
| `screen -r <name>` | Opens an unfinished process's screen. Prints `Process <name> not found.` otherwise |
| `screen -ls` | Shows CPU utilization, cores used and available, and the running and finished processes |
| `scheduler-start` | Starts generating dummy processes named `p01`, `p02`, ... |
| `scheduler-stop` | Stops generating dummy processes. Processes already created keep running |
| `report-util` | Saves the `screen -ls` report to `csopesy-log.txt` |
| `exit` | Stops the CPU thread and closes the program |

### Process screen

Entered with `screen -s` or `screen -r`.

| Command | Description |
|---------|-------------|
| `process-smi` | Shows the process name, ID, logs, current instruction line, and lines of code. Prints `Finished!` when the process is done |
| `exit` | Returns to the main menu. The process keeps running |

### Example session

```
root:\> initialize
Emulator initialized.
root:\> screen -s proc-01
root:\> process-smi
root:\> exit
root:\> screen -ls
root:\> screen -r proc-01
root:\> process-smi
root:\> exit
root:\> scheduler-start
Dummy process generation started.
root:\> scheduler-stop
Dummy process generation stopped.
root:\> report-util
Report generated at csopesy-log.txt!
root:\> exit
```

## Process Instructions

| Instruction | Behavior |
|-------------|----------|
| `PRINT` | Logs text. With no variable, logs `Hello world from <name>!`. With a variable, logs the text followed by the variable's value |
| `DECLARE` | Sets a variable to a value |
| `ADD` | `var = a + b`, where each operand is a variable or a number |
| `SUBTRACT` | `var = a - b`, where each operand is a variable or a number |
| `SLEEP` | Leaves the core for the given number of CPU cycles |
| `FOR` / `ENDFOR` | Repeats the instructions between them. Loops can be nested |

Rules:

- Every process starts with the variable `x` set to 0.
- A variable that has not been set reads as 0.
- Values are kept between 0 and 65535 (uint16).
- Generated processes contain random instructions. The user does not type them.

## Scheduler

- A background thread runs one CPU cycle every 100 milliseconds (`CYCLE_MS` in `resource_emulator.cpp`).
- Each cycle creates dummy processes if generation is on, wakes sleeping processes, runs each busy core, and fills free cores from the ready queue.
- **FCFS:** a process keeps its core until it finishes or sleeps.
- **Round-robin:** after `quantum-cycles` cycles, a process moves to the back of the ready queue.
- **SLEEP:** the process leaves its core and returns to the queue when its countdown reaches zero.
- **delay-per-exec:** after each instruction, the process stays on its core for that many cycles before the next instruction runs.

## Output Examples

`process-smi` (example):

```
Process name: proc-01
ID: 1
Logs:
(01/18/2026 09:15:22AM) Core:0 "Hello world from proc-01!"

Current instruction line: 153
Lines of code: 1240
```

`screen -ls` (example):

```
CPU utilization: 100%
Cores used: 4
Cores available: 0

---------------------------------------
Running processes:
p05         (01/18/2026 09:15:22AM)  Core: 0   1235 / 5876

Finished processes:
p01         (01/18/2026 09:00:21AM)  Finished  5876 / 5876
---------------------------------------
```

## Known Limitations

- Generated processes use random instructions. They do not yet follow a strict `PRINT` / `ADD` alternation.
- Generated `ADD` always adds 1. The 1 to 10 range is not used.
- Generated `PRINT` always logs the default `Hello world from <name>!` message.
- `scheduler-test` and `screen-ls` are not recognized. Use `scheduler-start` and `screen -ls`.
- `config.txt` values are not range-checked, and `SLEEP` values are not clamped to uint8.
