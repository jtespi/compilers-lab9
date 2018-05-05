int a[10];

int foo(int x, int y, int z)
{
	write "(in function foo)";
	write x;
	write y; 
	write z;
	write "(foo returning)";
	return x * y * z;
}

int bar( int i )
{
	return i / 5;
}

int main(void)
{
	write "foo(1,2,3) returned:";
	write foo(1,2,3);
	a[ bar(5) ] = 156;
	write "a[ bar(5) ] is:";
        write a[ bar(5) ];
}
