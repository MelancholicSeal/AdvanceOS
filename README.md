# Worker Pool (Exercise for AdvanceOS 2024-25)

This program simulates a worker pool system in which multiple worker processes handle tasks assigned by a parent process. The tasks are simulated by waiting for a random amount of time, and upon completion, the workers notify the parent. A log is maintained to record task completion details.

## Compilation and Execution

To compile and run the program, follow these steps:

1. **Compile the program:**
```bash
make
```
2. **Run the program:**
```bash
./main <log_file> <num_workers>
```
- `<output_file>`: The name of the output file where task completion logs will be written.
- `<num_workers>`: The number of workers in the pool (must be a positive integer).

## How It Works

### 1. Initialization:
- The program accepts two command-line arguments:
- The name of the output file where the logs will be stored.
- The number of workers in the pool.
- It sets up pipes to facilitate communication between the parent process and the workers.

### 2. Worker Pool:
- The parent process forks into multiple worker processes.
- Each worker waits for tasks sent from the parent via a pipe. Once a worker receives a task, it simulates work by sleeping for a random amount of time.
- After completing the task, the worker logs its activity to the output file and sends a notification back to the parent via a different pipe.

### 3. Parent Process:
- The parent process monitors the workers to determine which ones are available to receive new tasks. This is done using the `select()` system call.
- The parent assigns a new task to the first available worker and set's the worker unavailable.
- Using the `select()` system call, the parent process checks for completed tasks and updates the worker's availability.

### 4. Task Simulation:
- The task is simply simulated by having each worker sleep for a random amount of time.
- The task completion message includes the worker ID, the task ID.

### 5. Logging:
- A log of each worker's activity is written to the specified output file. The log includes:
- The process ID of the parent and the worker.

## Notes
- The program uses `fork()` to create worker processes and pipes to facilitate communication between them.
- `select()` is used to handle multiple file descriptors efficiently without blocking.
- The workers simulate task execution by sleeping for random durations, making the program suitable for demonstrating a worker pool's basic structure and operation.
