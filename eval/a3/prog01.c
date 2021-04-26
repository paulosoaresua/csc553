extern void println(int x);

/**
 * To test the effect of local, global and register allocation optimization.
 **/

void main(void)
{
  int x, y;
  int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q ,r, s, t;

  /*Program 1*/
  for(x = 0; x < 10;x = x + 1) {
    x = x;
  }

  y = x;

  if(x == y) {
    println(1);
  }

  /*Program 2*/
  a = 1;
  b = a;
  c = b;
  d = c;
  e = d;
  f = e;
  g = f;
  h = g;
  i = h;
  j = i;
  k = j;
  l = k;
  m = l;
  n = m;
  o = n;
  p = o;
  q = p;
  r = q;
  s = r;
  t = s;

  /*Program 3*/
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