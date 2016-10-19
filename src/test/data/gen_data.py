import random
rows = ("{} {}".format(random.randint(0,1), " ".join(["{}:{}".format(i, random.randint(0,100)) for i in xrange(1,500)])) for n in xrange(10000))
f = open("./rand_data", "w+")
f.write("\n".join(rows))
f.close()

