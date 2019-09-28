import sys

dictionary = dict()
dictionary['s'] = 'O'
dictionary['S'] = '.'
dictionary['b'] = ' '
dictionary['w'] = '#'
dictionary['m'] = '*'
dictionary['g'] = '+'

def decode_cell(cell):
    return dictionary[cell]

def decode_board(encoded):
    parts = encoded.split(':')
    size = parts[0]
    size_parts = size.split('x')
    width = int(size_parts[0])
    height = int(size_parts[1])
    rows = []
    rows.append('#' * (width + 2))
    cells = parts[1]
    i = 0
    for r in range(height):
        row = '#'
        for c in range(width):
            row += decode_cell(cells[i])
            i += 1
        row += '#'
        rows.append(row)
    rows.append('#' * (width + 2))
    return width + 2, height + 2, rows

sig = None
processed = []

def process(line):
    parts = line.split(' ')
    name = parts[0]
    sig = name.split('_')[0]
    processed.append(name)
    max_moves = int(parts[1])
    url = parts[2]
    encoded_board = url.split('#')[-1]
    width, height, rows = decode_board(encoded_board)
    with open(name + '.txt', 'w+') as f:
        f.write(str(height) + ' ' + str(width) + '\n' + str(max_moves) + '\n')
        f.write('\n'.join(rows))

def main():
    filename = sys.argv[1]
    with open(filename) as file:
        for line in file.readlines():
            process(line)

    with open(filename.replace('raw', 'list'), 'w+') as file:
        file.write('\n'.join(processed))

main()