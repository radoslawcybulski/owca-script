#!/usr/bin/env python3

import time

s = 0
start = time.time()
i = 0
while i < 10000000:
    s = (s * 11035 + 12345) & 0xffff
    i += 1
end = time.time()

print(f"Time taken: {end - start:.2f} seconds")
print(f"Final result: {s}")