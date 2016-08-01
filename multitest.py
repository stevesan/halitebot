#!/Users/Steve/anaconda/bin/python

import subprocess as sp
from multiprocessing import Pool

sp.check_call('make mine && make basic', shell=True)

def runtest(testnum):
    out = sp.check_output('Halite-Mac/environment -d 20 20 "bin/MyBot dbg-%d.log" bin/BasicBot | grep rank' % testnum,
            shell=True)
    won = False
    for line in out.split('\n'):
        if 'TheDarkness' in line and 'came in rank #1' in line:
            won = True
            break
    print 'done', testnum, "WON!" if won else "lost..."
    return won

tests = 16
p = Pool(4)
didwin = p.map(runtest, range(tests))
print didwin.count(True), len(didwin)
