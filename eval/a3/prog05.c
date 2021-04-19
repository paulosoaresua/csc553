extern void println(int x);

/**
 * To test the effect of compiling time.
 **/

void main(void)
{
  int a, b, c, d, e, f;
  int i, j, k, l, m, n, o, p, q ,r, s;

  for(i = 0; i < 2; i = i + 1) {
    for(j = 0; j < 2; j = j + 1) {
      for(k = 0; k < 2; k = k + 1) {
        for(l = 0; l < 2; l = l + 1) {
          for(m = 0; m < 2; m = m + 1) {
            for(n = 0; n < 2; n = n + 1) {
              for(o = 0; o < 2; o = o + 1) {
                for(p = 0; p < 2; p = p + 1) {
                  for(q = 0; q < 2; q = q + 1) {
                    for(r = 0; r < 2; r = r + 1) {
                      for(s = 0; s < 2; s = s + 1) {
                        a = 1;
                        b = a + 2;
                        c = b + 3;
                        d = c + 4;
                        e = d + 5;
                        f = e + 6;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  println(a + b + c + d + e + f + i + j + k + l + m + n + o + p + q + r + s);
}