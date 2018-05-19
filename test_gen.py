import os, sys, random

def main(argv):
    path = argv[1]
    size = int(argv[2]) * 1024*1024 # in MB
    wrote = 0
    random.seed(0)
    with open(path,"w") as f:
        while wrote < size:
            lsize = random.randint(5,25)
            for i in xrange(lsize):
                f.write("bla ")
                wrote += 4
            f.write("\n")
            wrote += 2
            

if  __name__ == "__main__":
    main(sys.argv)