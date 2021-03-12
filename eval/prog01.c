extern void println(int x);

/** This program evaluates the efficiency of the peephole optimizations
performed**/

int power(int number, int p) {
  int x
  int pow;

  pow = 1;
  for(x = 0; x < p; x = x + 1) {
    pow = pow * number;
  }

  return pow;
}

void main(void) {
  int x;

  x = 0;
  for(;x < 10;x = x + 1) {
    println(power(x, 2));
  }

}