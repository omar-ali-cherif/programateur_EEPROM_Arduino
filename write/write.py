import serial, time, hashlib

PORT       = "/dev/ttyUSB0"
BAUD       = 115200
IMAGE_PATH = "/home/user/Desktop/Project/medm/new.bin"
PAGE_SIZE  = 64

def wait_for(ser, expected, timeout=30):
    deadline = time.time() + timeout
    while time.time() < deadline:
        line = ser.readline().decode(errors="ignore").strip()
        if line:
            print("  <<", line)
        if line == expected:
            return
        if line.startswith("X") or line.endswith("FAIL"):
            raise RuntimeError(f"Arduino: {line}")
    raise TimeoutError(expected)

with open(IMAGE_PATH, "rb") as f:
    image = f.read()

assert len(image) <= 4 * 1024 * 1024
if len(image) % PAGE_SIZE:
    image += b"\xFF" * (PAGE_SIZE - len(image) % PAGE_SIZE)

num_pages = len(image) // PAGE_SIZE
print(f"{len(image)} bytes, {num_pages} pages of {PAGE_SIZE}")

ser = serial.Serial(PORT, BAUD, timeout=5)
time.sleep(0.5)
wait_for(ser, "BOOT", timeout=10)

print("Erasing...")
ser.write(b"E")
wait_for(ser, "R", timeout=120)

print("Writing...")
t0 = time.time()
for i in range(num_pages):
    page = image[i * PAGE_SIZE:(i + 1) * PAGE_SIZE]
    ser.write(b"P")
    ser.write(i.to_bytes(2, "little"))
    ser.write(page)
    wait_for(ser, "K", timeout=10)
    if i % 500 == 0 or i == num_pages - 1:
        print(f"  {i+1}/{num_pages}  {time.time()-t0:.1f}s")

ser.write(b"D")
wait_for(ser, "F", timeout=10)
ser.close()

print("SHA256:", hashlib.sha256(image).hexdigest())
