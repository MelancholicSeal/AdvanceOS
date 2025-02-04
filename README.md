# RPC (Exercise for AdvanceOS 2024-25)

This program creates an multiprocess RPC-server by customising rpcgen code. Currently addition is the only supported operation 

## Compilation and Execution

To compile and run the program, follow these steps:

1. **Clone Repo:**
```bash
git clone https://github.com/MelancholicSeal/AdvanceOS.git
```
2. **Build & Run Docker:**
```bash
docker build -t rpc_image .
sudo docker run -it rpc_image
```
- It automatically starts the server
3. **Run Client(Local Machine):**
  -In a new terminal, in the same folder
```bash
make -f Makefile.add
./add_client <dockerServer_IP> <number1> <number2>
```
- `<dockerServer_IP>`: The ip of the running docker. To find you can use docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' <container_name_or_id>.
- `<number1>, <number2>`: The numbers to be added their sum will be printed.
3. **Run Client(Docker):**
  - It hasn't been tested yet
  - You should create your own docker file that runs the add_client to test

--Note--
- To test parallelism you should run many clients


## How It Works

### 1. Initialization:
- Main server Process sets ups pipes and forks into multiple worker processes.

### 2. Worker Pool:
- Each worker waits for tasks sent from the parent via a pipe. 
- For more informaion see previous worker pool commits 

### 3. Parent Process:
- The parent process monitors the RPC server's read file descriptor bit mask (svc_fdset). This is done using the `select()` system call.
- The parent assigns a new task to the first available worker and set's the worker unavailable.
- Using the `select()` system call, the parent process checks for completed tasks and updates the worker's availability.

### 4. Worker Process:
- Waits for parent's message
- Services Client using svc_getreqset() routine


### 5. Task Simulation:
- Currently only addition is available.
- To simulate heavier tasks and test parallelism the workers after doing the addition wait for predetermined ammount of time.

### 6. Graceful Shutdown

The program handles graceful shutdown by catching **SIGINT** (interrupt) signals, typically sent when the user presses **Ctrl+C**.

#### In the Parent Process:
- The program uses the `sigaction` system call to register a signal handler for **SIGINT**. When the signal is caught, the `kill_child()` function is executed.
- The `kill_child()` function sends the **SIGINT** signal to all child processes to terminate them gracefully. Then, it waits for each child process to exit using `waitpid()`.
- After ensuring all workers have exited, the program closes any open pipes and frees dynamically allocated memory to prevent memory leaks.

#### In the Worker Processes:
- Each worker also sets up a signal handler for **SIGINT** using `sigaction`. When the signal is received, the worker enters the `term_child()` function, which:
    - Closes its pipes.
    - Frees dynamically allocated memory before exiting.

### 7. Client:
- Client gets Servers IP and numbers to be added as arguments
- Prints result when recieved from server


## Known Issues

### 1.
- Due to the way worker processes find client requests it is possible for one worker to serve multiple clients at once creating a personal queue.
- Even worse there is a chance that 2 different workers try complete the same request simultaneously, wasting resources.

### 2.
- Graceful Shutdown needs to be updated to terminate RPC server
- Currently even after shutting down clients wait until RPC times out

## Future Work

### Fix Known Issues

### Add more functions
- Matrix multiplication
- Scientific Calculator

### Re add Log File from previous Worker Pool Implementation
- To be honest I don't know why I removed it

### Create a dockerfile for creating Clients
- Easier Testing
- Accesible for more machines (Currently clients have been tested only through my local linux machine)


## Notes
- The program uses `fork()` to create worker processes and pipes to facilitate communication between them.
- `select()` is used to handle multiple file descriptors efficiently without blocking.
