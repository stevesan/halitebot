#!/Users/Steve/anaconda/bin/python

import subprocess as sp
import multiprocessing

sp.check_call('make mine && make basic', shell=True)


def runtest(testnum):
    botcmds = [
        "bin/MyBot dbg-%d.log" % testnum,
        "bin/MyBotLastSub dbg-last-%d.log" % testnum
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
