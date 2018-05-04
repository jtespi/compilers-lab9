int result;

int fib1( int n ) {

   if ( n == 0 ) {
     return 0;
    }

   if ( n == 1 ) {
     return 1;
   }


   return  fib1( n-1 ) + fib1( n-2 );

}

void main (void ) {
   int uInput;   

   write "about to call Fibonacci function";
   uInput = 0;

   read uInput;

   write "entered n value was:";
   write uInput;

   write "Fibonacci sequence on n equals:";
   write fib1( uInput );

   write "END";

}

   
