void foo(int x, int y, int z)
{
	write x;
	write y; 
	write z;
	return x * y * z;
}

int main(void){
	write foo(1,2,3);
}
