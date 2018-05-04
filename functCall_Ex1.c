int x4[100];
int f ( int z)
  {
   write "inside f";
   write z;
   return z;
   }

void main(void) {
   int g;
   g = 100;
   f(2);
   write g;
}
