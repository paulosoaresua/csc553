extern void println(int x);

/**
 * To test the effect of register allocation distribution.
 **/

void sum7(void) {
  int a, b, c, d, e, f, g;
  a = 1;
  b = 2;
  c = 3;
  d = 4;
  e = 5;
  f = 6;
  g = 7;

  println(a+b+c+d+e+f+g);
}

void main(void)
{
  int a, b, c, d, e, f, g;
  a = 1;
  b = 4;
  c = 9;
  d = 16;
  e = 25;
  f = 36;
  g = 49;

  /**
   * If registers are the same as the ones used in sum7, there will be a series
   * of writes and loads from memory at every call to sum7.
   * **/
  sum7(); /* Comment this line to test scenario 3 */
  sum7(); /* Comment this line to test scenario 3 */
  sum7(); /* Comment this line to test scenario 3 */
  sum7(); /* Comment this line to test scenario 3 */
  println(a+b+c+d+e+f+g);
  /*sum7();*/ /* Uncomment this line to test scenario 3 */
  /*sum7();*/ /* Uncomment this line to test scenario 3 */
  /*sum7();*/ /* Uncomment this line to test scenario 3 */
  /*sum7();*/ /* Uncomment this line to test scenario 3 */
}