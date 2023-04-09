from pathlib import Path
from string import ascii_lowercase
from random import choice

N = 4
M = 4
K = 4
lib = Path("testlib")

for file_n in range(N):
    for file_m in range(M):
        file = lib / f"{file_n}_{file_m}.txt"

        text = ""

        for _ in range(K):
            text += "".join([choice(ascii_lowercase) for _ in range(8)]) + "_book\n"

        file.write_text(text)