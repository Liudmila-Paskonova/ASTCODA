#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>

#include <stdio.h>
#define BUF_SIZE 100
using namespace std;

int main(){
    char input[BUF_SIZE];
    double a, b, n;
	fgets(input,BUF_SIZE,stdin);
    sscanf(input,"%lf %lf %lf",&a,&b,&n);

    double max = 0;
    
    for(int x = 0 ; x <= b; x++)
    {
        double tmp = x / b;
        double ans = floor(a * tmp) - a * floor(tmp);
        if(ans > max)
        {
            max = ans;
        }
    }

    cout << max << endl;

}
