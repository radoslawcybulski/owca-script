import os

def proc(p):
    max = 0
    for f in os.listdir(p):
            ff = os.path.join(p,f)
            if os.path.isdir(ff):
                    v = proc(ff)
            else:
                    try:
                            v = int(f)
                    except:
                            v = -1
            if v > max:
                    max = v
    return max
	
print(proc('.'))