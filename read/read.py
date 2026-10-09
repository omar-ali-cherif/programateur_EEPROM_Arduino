import serial
import sys
import time

PORT = "/dev/ttyUSB0"           # Windows: "COM3", Linux/Mac: "/dev/ttyUSB0"
BAUD = 115200
OUT  = "bios_dump.bin"
SIZE = 4 * 1024 * 1024  # 4 MB

ser = serial.Serial(PORT, BAUD, timeout=5)

# Wait for the Arduino's READY line
print("Waiting for Arduino...")
while True:
    line = ser.readline().decode(errors="ignore").strip()
    if line:
        print("Arduino:", line)
    if line == "READY":
        break

ser.write(b"\x01")   # tell Arduino to start
print("Reading flash...")

received = 0
with open(OUT, "wb") as f:
    while received < SIZE:
        chunk = ser.read(min(4096, SIZE - received))
        if not chunk:
            print(f"\nTimeout at {received} bytes")
            break
        f.write(chunk)
        received += len(chunk)
        # progress
        pct = received * 100 // SIZE
        print(f"\r{received:>9}/{SIZE} bytes ({pct}%)", end="", flush=True)

print()
print(f"Saved {received} bytes to {OUT}")
ser.close()
