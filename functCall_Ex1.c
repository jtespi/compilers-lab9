int x4[100];
int f ( int z)
  {
   write "inside f, the param given was:";
   write z;
   return z * 5;
   }

void main(void) {
   int g;
   g = 100;
   write "about to call a function";
   write f(2);
   write g;
}
