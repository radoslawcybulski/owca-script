#!/usr/bin/env python3

import time

start = time.time()

states = {}
final_result = 0

def update_state(name, is_bid, val, count):
    global final_result
    v = states.setdefault(name, {})
    key = val * 2
    if is_bid:
        key = key + 1
    # if count > 0:
    #     key = '+'
    # else:
    #     key = '-'
    # key = key + str(val)
    old = v.get(key, 0)
    new = old + count
    if old > 0 and new <= 0:
        del v[key]
    else:
        v[key] = new
        final_result = (final_result * 3 + new) & 0xffffffff

def run():
    class Random:
        def __init__(self):
            self.state = 0
        def next(self):
            self.state = (self.state * 11035 + 12345) & 0xffff
            return self.state
    random = Random()

    for i in range(0, 10000000):
    #for i in range(0, 10):
        v = random.next()
        s_index = v % 1000
        is_bid = (random.next() % 2) == 0
        val = random.next() % 20
        count = (random.next() % 10) - 5
        update_state(s_index, is_bid, val, count)

run()
end = time.time()

print(f"Time taken: {end - start:.2f} seconds")
print(f"Final result: {final_result}")