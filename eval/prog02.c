extern void println(int x);

/** To test the effect of copy propagation optimization. **/

void main(void)
{
  int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p,q ,r ,s ,t;

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
}