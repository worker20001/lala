FROM python:3.11-slim

# Install compilers and tools
RUN apt-get update && apt-get install -y g++ make && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy your C++ source (rename from soulcracks.cpp if it's legitimate)
COPY soulcracks.cpp .

# YOUR requested compilation command:
RUN g++ -std=c++14 soulcracks.cpp -o soul -pthread

# Also build a shared library if you need Python ctypes to call functions
# (Your original plan required a .so file, not an executable)
# For an executable, you would run it separately, not as an API endpoint.

# For a proper API that calls C++ functions, you need a shared library:
# RUN g++ -shared -fPIC -std=c++14 soulcracks.cpp -o libsoul.so -pthread

COPY main.py .

CMD uvicorn main:app --host 0.0.0.0 --port $PORT
