
# IO Scheduling Simulation

## Overview
This IO scheduling simulation project implements several disk scheduling algorithms designed to optimize the processing of I/O operations in a simulated hard disk environment. The primary goal is to minimize the total movement of the disk's read/write head and reduce the overall wait time for I/O operations, thereby enhancing the efficiency of the disk.

The project is based on a typical operating systems problem where I/O requests are queued and must be managed effectively to optimize access to disk resources. This simulation provides a hands-on approach to understanding how different algorithms can impact performance under various conditions.

![image](https://github.com/tarun2001sharma/IO-Scheduler/assets/59308544/60f22d4d-24b6-470b-8aba-062dcf527f32)


Implemented algorithms include:
- **FIFO (First In, First Out)**: Processes requests in the order they arrive.
- **SSTF (Shortest Seek Time First)**: Selects the request closest to the current head position.
- **LOOK**: Moves the head towards the nearest end and processes requests in this direction until it needs to reverse.
- **CLOOK (Circular LOOK)**: Similar to LOOK but only moves in one direction and jumps back to the beginning without reversing.
- **FLOOK (Foresight LOOK)**: An enhancement over LOOK that uses an additional queue to look ahead and plan disk movements more efficiently.

Each of these algorithms is suited to different scenarios, and the simulation allows for the exploration of their strengths and weaknesses in a controlled environment. This project is ideal for students and professionals interested in the fundamentals of operating systems, disk management, and algorithm efficiency.

## Getting Started

### Prerequisites
Ensure you have a GCC compiler installed on your system to build the project. This project is tested on `linserv*.cims.nyu.edu`.

### Building the Project
To build the project, navigate to the project directory and use the provided Makefile:

```bash
make
```

This will compile the source files and generate an executable named `iosched`.

### Running the Simulation
To run the simulation, use the following command format:

```bash
./iosched [-s<schedalgo>] [-v] [-q] [-f] <inputfile>
```

- `-s<schedalgo>`: Specify the scheduling algorithm to use. Valid options are `n` for FIFO, `s` for SSTF, `l` for LOOK, `c` for CLOOK, and `f` for FLOOK.
- `-v`: Enable verbose output for debugging.
- `-q`: Show details of the IO queue and direction of movement.
- `-f`: Show additional queue information during FLOOK.
- `<inputfile>`: Path to the input file containing the IO operations.

**Example:**

```bash
./iosched -s f input.txt
```

This will run the scheduler using the FLOOK algorithm with the operations specified in `input.txt`.

## Input File Format
The input file should contain lines with two integers: the time step at which the IO operation is issued and the track that is accessed. Lines starting with `#` are treated as comments.

**Example of an input file:**

```
# Sample Input
1 339
131 401
...
```

## Output Format
The program will output details for each IO operation followed by a summary line. Here's what the output includes:

- Individual IO request information.
- Summary statistics including total simulated time, total number of track movements, IO utilization, average turnaround time, average wait time, and maximum wait time.

**Example Output:**

```
0:     1     1   431
1:    87   467   533
...
SUM: 10000 150 0.9500 20.00 15.00 300
```

## License
Distributed under the MIT License. See `LICENSE` for more information.

