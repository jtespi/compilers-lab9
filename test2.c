int x;
int array[100];

int main(void) {
  int i;
  int iA;

  int a;
  int b;

  int uInput;
  uInput = 0;

  x = 5;
  array[0] = 42;
  array[1] = 41;
  
  write "array[0] equals:";
  write array[0];

  if ( x == 5 ) {
     write "in body of if stmt";
  }

  
  i = 2;
  iA = 1;
  while ( i >= 0 ) {
    if ( iA == 1 ) {
      write "in while stmt";
      write "  iterations left:";
      iA = 0;
    }
    write i;
    i = i - 1;
  }

  write "enter the number to count to:";
  read uInput;

  i = 0;
  while ( i <= uInput ) {
     write i;
     i = i + 1;
  }

  

}
