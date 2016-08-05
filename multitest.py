#!/Users/Steve/anaconda/bin/python

import subprocess as sp
import sys
import multiprocessing

bot1 = sys.argv[1]
bot2 = sys.argv[2]

def runtest(testnum):
    global bot1
    global bot2
    botcmds = [
        "%s dbg-A-%d.log" % (bot1, testnum),
        "%s dbg-B-%d.log" % (bot2, testnum),
    ]
        
    shellcmd = 'tools/environment -d 20 20 ' + ' '.join(['"%s"' % cmd for cmd in botcmds]) + ' | grep rank'
    # print shellcmd
    out = sp.check_output(shellcmd, shell=True)
    won = False
    for line in out.split('\n'):
        if 'Player #1' in line and 'came in rank #1' in line:
            won = True
            break
    print 'done', testnum, "WON!" if won else "lost..."
    return won

tests = 128
p = multiprocessing.Pool(2)
didwin = p.map(runtest, range(tests))
print didwin.count(True), len(didwin)
