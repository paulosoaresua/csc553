extern void println(int x);

/** To test the effect of dead code elimination. **/

void main(void)
{
  int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q ,r;

  /*Block 1*/
  a = 1;
  b = a;
  c = b;
  d = c;
  e = d;
  f = e;

  /*Block 2*/
  println(1);
  g = f;
  h = g;
  i = h;
  j = i;
  k = j;
  l = k;

  /*Block 3*/
  println(1);
  m = l;
  n = m;
  o = n;
  p = o;
  q = p;
  r = q;

}