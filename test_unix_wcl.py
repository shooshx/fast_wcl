import sys, os, subprocess, time

def timeit(cmd):
    start = time.clock()
    print cmd
    subprocess.call(cmd)
    elapsed = time.clock() - start
    print "took", elapsed, "sec"
    return elapsed

def timeit_n(cmd, n):
    r = []
    for i in xrange(0, n):
        r.append(timeit(cmd))
    return sum(r)/n
    
N = 10
def main():
    a10 = timeit_n("wc -l test10.txt", N)
    a100 = timeit_n("wc -l test100.txt", N)
    a500 = timeit_n("wc -l test500.txt", N)
    a1g = timeit_n("wc -l test1000.txt", N)
    print "\ntest10:  %s\ntest100: %s\ntest500: %s\ntest1g:  %s" % (a10,a100,a500,a1g)






if __name__ == "__main__":
    main()