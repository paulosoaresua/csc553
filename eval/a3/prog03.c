extern void println(int x);

/**
 * To test the effect of variable cost on preferential set.
 **/

void sum15(void) {
  int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o;
  a = 1 + 0;
  b = 2 + 0;
  c = 3 + 0;
  d = 4 + 0;
  e = 5 + 0;
  f = 6 + 0;
  g = 7 + 0;
  h = 8 + 0;
  i = 9 + 0;
  j = 10 + 0;
  k = 11 + 0;
  l = 12 + 0;
  m = 13 + 0;
  n = 14 + 0;
  o = 15 + 0;

  println(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o);
}

void main(void)
{
  int a, b, c, d, e;
  a = 1 + 0;
  b = 2 + 0;
  c = 3;
  d = 4;
  e = 5;

  sum15();
  println(a+b+c+d+e);

  for(; c <= 6; c=c+1) {
    /* The cost of variable c will be 10 and must be chosen to be allocated
     * first. At this point, the other variables are not live anymore and
     * only the register chosen for c has to be saved before the function
     * call if it's different than $s7. With the optimization implemented, this
     * will not happen.*/
    sum15();
  }
}