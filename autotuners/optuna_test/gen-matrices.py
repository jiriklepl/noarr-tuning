#!/usr/bin/python3

import random, struct, sys

len(sys.argv) == 3 or exit(f'Usage: {sys.argv[0]} OUTPUT_FILE MATRIX_SIZE')

with open(sys.argv[1], 'wb') as out:
	size = int(sys.argv[2])
	n = 2*size
	for i in range(n):
		out.write(b''.join(random.choices([struct.pack('<f', f) for f in range(4)], k=size)))
		print(i*10000//n/100, end='%\r')
