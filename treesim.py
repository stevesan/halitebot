import random

chain_len = 3

STILL = 0
MOVE = 1

strs0 = [random.randint(0, 255) for _ in range(chain_len)]
prods = [random.randint(0, 255) for _ in range(chain_len)]

def sim_moves(moves):
    assert len(moves) == chain_len*chain_len

    rounds = len(moves)/chain_len
    strs = list(strs0)
    total_output = 0
    
    for r in range(rounds):
        round_moves = moves[r*chain_len : (r+1)*chain_len]
        assert len(round_moves) == chain_len
        for pos in range(chain_len):
            move = round_moves[pos]
            if move == STILL:
                strs[pos] = min(255, strs[pos] + prods[pos])
            else:
                assert move == MOVE
                if pos == 0:
                    total_output += strs[pos]
                else:
                    strs[pos-1] = min(255, strs[pos-1] + strs[pos])
                strs[pos] = 0

    return total_output 

# brute force, recursive
def max_output(moves, index):
    global strs0
    global prods
    if index >= len(moves):
        # time to actually play out this path
        return (sim_moves(moves), list(moves))

    else:
        moves[index] = STILL
        o1 = max_output(moves, index+1)

        moves[index] = MOVE
        o2 = max_output(moves, index+1)

        if o1[0] >= o2[0]:
            return o1
        else:
            return o2

moves = [STILL for _ in range(chain_len*chain_len)]

print strs0
print prods
best_output, best_moves = max_output(moves, 0)

print "best out = ", best_output
for r in range(chain_len):
    print best_moves[r*chain_len : (r+1)*chain_len-r]
