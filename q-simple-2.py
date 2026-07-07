#!/usr/bin/env python3

import time

class A:    
    def foo1(self, s):
        return s + 1
    def foo2(self, s):
        return self.foo1(s)
    def foo3(self, s):
        return self.foo2(s)
    def foo4(self, s):
        return self.foo3(s)
    def foo5(self, s):
        return self.foo4(s)
s = 0
a = A()
start = time.time()
i = 0
while i < 100000000:
    s = a.foo5((s * 11035 + 12345) & 0xffff)
    i += 1
end = time.time()

print(f"Time taken: {end - start:.2f} seconds")
print(f"Final result: {s}")