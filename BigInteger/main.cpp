#include "BigInteger.h"
#include "doctest/doctest.h"

int main(int argc, char *argv[]) {
  BigInteger a = "9223372036854775807";
  BigInteger b = "-9223372036854775808";
  a.print();
  b.print();
  printf("----------------------\n");
  BigInteger c(a + b);
  c.print();
  return 0;
}
