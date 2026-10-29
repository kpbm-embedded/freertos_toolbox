# Host-side reference for src/main.c: sum over i < 100 of ((i * 37) % 101) - 50.
# Written before 32_31_bend existed: syntax follows the Bend 2 README and is not yet checked by `bend`.
def sample(i: I32) -> I32:
  ((i * 37) % 101) - 50

def total(i: I32, acc: I32) -> I32:
  if i == 100:
    acc
  else:
    total(i + 1, acc + sample(i))

def main() -> I32:
  total(0, 0)
