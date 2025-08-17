#include<iostream>
int main() {
	int a, b;
	std::cin>>a>>b;
	int first = a;
	int count = 1;
	for (int i = 0; first < b; ++i)
	{
		if (first >= b)
		{
			break;
		} else 
		{
			count++;
			first += a - 1;
		}
	}
	std::cout<<count<<std::endl;
}