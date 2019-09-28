import os
import time
import sys
import subprocess
import io
import judge

def load_level(filename):
    with open(filename) as file:
        lines = [line.strip() for line in file.readlines()]
        h, w = [int(v) for v in lines[0].split(' ')]
        j = int(lines[1])
        level = [['x' for c in range(w)] for r in range(h)]
        for r in range(h):
            line = lines[r + 2]
            for c in range(w):
                if line[c] == '.':
                    level[r][c] = 'o'
                else:
                    level[r][c] = line[c]
        return level

def load_solution(filename):
    return open(filename).read()

def offset(direction):
    if direction == '0':
        return (0, -1)
    elif direction == '1':
        return (1, -1)
    elif direction == '2':
        return (1, 0)
    elif direction == '3':
        return (1, 1)
    elif direction == '4':
        return (0, 1)
    elif direction == '5':
        return (-1, 1)
    elif direction == '6':
        return (-1, 0)
    elif direction == '7':
        return (-1, -1)

def locate_vehicle(level):
    w, h = get_size(level)
    for r in range(h):
        for c in range(w):
            if level[r][c] == '.' or level[r][c] == 'o':
                return r, c

def trace(level, direction):
    dc, dr = offset(direction)
    r, c = locate_vehicle(level)
    if level[r][c] == 'o':
        level[r][c] = 'O'
    else:
        level[r][c] = ' '
    while True:
        if level[r+dr][c+dc] == '*':
            return False

        if level[r+dr][c+dc] == '+':
            level[r+dr][c+dc] = ' '

        if level[r+dr][c+dc] == 'O':
            level[r+dr][c+dc] = 'o'
            return True

        if level[r+dr][c+dc] == '#':
            level[r][c] = '.'
            return True

        r += dr
        c += dc

def apply_solution(level, solution):
    for d in solution:
        if not trace(level, d):
            return False
    return True

def get_size(level):
    return (len(level[0]), len(level))

def print_level(level):
    w, h = get_size(level)
    for r in range(h):
            print(''.join(v for v in level[r]))

def check(problem_filename, solution, max_moves):
    '''
    if len(solution) == 0:
        return 'EMPTY'

    if len(solution) > max_moves:
        return 'TOO_LONG'

    level = load_level(problem_filename)
    w, h = get_size(level)
    if not apply_solution(level, solution):
        return 'MINE'

    c = 0
    for r in level:
        for v in r:
            if v == '+':
                c += 1
    return 'JEWELS_LEFT' if c > 0 else 'OK'
    '''
    pr = judge.readDane(problem_filename)
    pr["N"] = max_moves
    return judge.spr(pr, solution.strip())

def min_moves(solvername, name, timeout, a):
    best = 9999
    r = 'BRAK'
    while True:
        infile = 'input/' + name + '.txt'
        result = None
        start = time.time()
        max_moves = best - 1
        try:
            result = subprocess.run([solvername, str(max_moves)], stdout=subprocess.PIPE, stdin=open(infile), timeout=timeout)
        except subprocess.SubprocessError:
            break
        end = time.time()

        if result is None:
            break

        out = ''.join(a for a in str(result.stdout) if a in ['0', '1', '2', '3', '4', '5', '6', '7'])

        if 'BRAK' in out:
            break
        else:
            r = out.strip().split('\\n')[-1]
            moves = len(r)
            if moves <= 0:
                break
            if a:
                che = check(infile, out, max_moves)
                if che == 'OK':
                    best = moves
                print(name, che, moves, '{:.3f}s'.format(end - start), r)

    if not a:
        print(name, best, r)

    return best

def main():
    solvername = sys.argv[1]
    filename = sys.argv[2]
    timeout = int(sys.argv[3]) # seconds
    a = bool(sys.argv[4])
    total = 0
    with open(filename) as file:
        for line in file:
            total += min_moves(solvername, line.strip(), timeout, a)
            print('')

    print('Total: {}'.format(total))

main()