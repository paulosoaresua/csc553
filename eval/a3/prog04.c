extern void println(int x);

/**
 * To test the effect of variable cost on node spilling.
 **/

void main(void)
{
  int i, j, k;
  int a, b, c, d, e;

  for(i = 0; i < 10; i = i + 1) {
    a = i;
  }

  for(i = 0; i < 10; i = i + 1) {
    for(j = 0; j < 10; j = j + 1) {
      b = i * j;
    }
  }

  for(i = 0; i < 10; i = i + 1) {
    for(i = 0; i < 10; i = i + 1) {
      for (k = 0; k < 10; k = k + 1) {
        c = i * j * k;
      }
    }
  }

  for(i = 0; i < 10; i = i + 1) {
    for(j = 0; j < 10; j = j + 1) {
      d = i * j;
    }
  }

  for(i = 0; i < 10; i = i + 1) {
    e = i;
  }

  println(a+b+c+d+e);
}