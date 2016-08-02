#!/Users/Steve/anaconda/bin/python

import subprocess as sp
from multiprocessing import Pool

sp.check_call('make mine && make basic', shell=True)

def runtest(testnum):
    # out = sp.check_output('tools/environment -d 20 20 "bin/MyBot dbg-%d.log" "bin/MyBotLastSub dbg-last-%d.log" | grep rank' % (testnum, testnum),
    out = sp.check_output('tools/environment -d 20 20 "bin/MyBot dbg-%d.log" "bin/BasicBot" | grep rank' % (testnum),
            shell=True)
    won = False
    for line in out.split('\n'):
        if 'Player #1' in line and 'came in rank #1' in line:
            won = True
            break
    print 'done', testnum, "WON!" if won else "lost..."
    return won

tests = 32
p = Pool(4)
didwin = p.map(runtest, range(tests))
print didwin.count(True), len(didwin)
