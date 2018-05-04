int x;
int array[100];

int main(void) {
  int i;
  int iA;

  int a;
  int b;

  int abCombo[5];

  int uInput;
  uInput = 0;

  x = 5;
  array[0] = 42;
  array[1] = 41;
  array[2] = 22;
  array[ 1 + 1 - 2] = 10;
  array[42] = 442;
 
  write "array[ array[0] ] is:";
  write array[ array[ 0 ] ];
  
  write "array[1] equals:";
  write array[1];

  if ( x == 5 ) {
     write "in body of if stmt";
  }

  
  i = 2;
  iA = 1;
  while ( i >= 0 ) {
    if ( iA == 1 ) {
      write "in while stmt";
      write " iterations left:";
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

  a = 0;
  b = 0;

  write "enter value for a:";
  write "enter value for b:";
  read a;
  read b;
  write a;
  write b;

  if ( a < b ) { write "a less than b"; }
  if ( a <= b ) { write "a less than or equal to b"; }
  if ( a > b ) { write "a greater than b"; }
  if ( a == b ) { write "a is equal to b"; }
  if ( a != b ) { write "a is not equal to b"; }
  if ( a >= b ) { write "a greater than or equal to b"; }
  
  abCombo[0] = a * b;
  if ( b != 0 ) {
    abCombo[1] = a / b;
  } else {
    abCombo[1] = 9999;
  }
  abCombo[2] = a - b;
  abCombo[3] = a + b;
  abCombo[4] = a == b; 
 
  write "a * b equals:";
  write abCombo[0];
  
  write "a / b equals:";
  if ( abCombo[1] != 9999 ) { write abCombo[1]; }
   else { write "INVALID!"; }

  write "a - b equals:";
  write abCombo[2];

  write "a + b equals:";
  write abCombo[3];

  write "a == b value is:";
  write abCombo[4];
  
  write "END";
}
