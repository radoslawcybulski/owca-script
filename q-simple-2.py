#!/usr/bin/env python3

import time

def foo1(s):
    return s + 1
def foo2(s):
    return foo1(s)
def foo3(s):
    return foo2(s)
def foo4(s):
    return foo3(s)
def foo5(s):
    return foo4(s)
s = 0
start = time.time()
i = 0
while i < 100000000:
    s = foo5((s * 11035 + 12345) & 0xffff)
    i += 1
end = time.time()

print(f"Time taken: {end - start:.2f} seconds")
print(f"Final result: {s}")