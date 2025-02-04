# Use the official Ubuntu image as a base
FROM ubuntu:latest

# Set the working directory inside the container
WORKDIR /app

# Install necessary packages for building the RPC server and client
RUN apt-get update && apt-get install -y \
    build-essential \
    rpcbind \
    nfs-common \
    libtirpc-dev \
    gcc \
    make \
    net-tools \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Create necessary directories for rpcbind and set correct ownership
RUN mkdir -p /run/rpcbind /run/sendsigs.omit.d && \
    chown root:root /run/rpcbind /run/sendsigs.omit.d

# Expose the RPC server port (adjust this if your server uses a different port)
EXPOSE 50051

# Copy the project files into the container
COPY . /app

# Verify that the Makefile and other project files are copied correctly
RUN ls -l /app

# Check gcc and make versions to ensure they are installed correctly
RUN gcc --version
RUN make --version

# Run the Makefile to build your project
RUN make -f Makefile.add

# Set the ownership of /run/rpcbind to root:root
RUN chown -R root:root /run/rpcbind /run/sendsigs.omit.d

# Command to start rpcbind and then the server
CMD ["sh", "-c", "rpcbind && ./add_server 0.0.0.0:50051"]




