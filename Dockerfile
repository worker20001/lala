# Use a specific Python version for consistency
FROM python:3.11-slim

# Install the GNU C++ compiler, which is needed to build your library
RUN apt-get update && apt-get install -y g++ make && rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /app

# Copy your application's code into the container
COPY . .

# Install Python dependencies from your requirements file
RUN pip install --no-cache-dir -r requirements.txt

# Compile your C++ code into a shared library (.so file)
# The output file 'soul.so' will be created in the /app directory
RUN g++ -shared -fPIC -std=c++14 -pthread -o soul.so soul.cpp

# The command to run when the container starts
# It uses waitress to serve your Flask app
CMD waitress-serve --port=$PORT api:app
