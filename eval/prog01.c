extern void println(int x);

/** To test the effect of peephole optimization. **/

void main(void)
{
  int x, y;

  for(x = 0; x < 10;x = x + 1) {
    x = x;
  }

  y = x;

  if(x == y) {
    println(1);
  }

}