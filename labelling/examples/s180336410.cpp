#include <stdio.h>		// standard input output
#include <iostream>		// input output stream, cin cout( console in console out)
#include <cmath>		// circle math ( sin, con, sinh
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>		// input output manipulation

using namespace std;
#define rep(i, n) for(int i = 0; i < (int)n; i++)
#define rep3(i, a, n) for(int i = (int)(a); i < (int)(n); i++)
#define ll long long

template<class T> inline bool chmax(T& a, T b) { if (a < b) { a = b; return true; } return false; }
template<class T> inline bool chmin(T& a, T b) { if (a > b) { a = b; return true; } return false; }






int main() {
  ll N,M,K; // 変数の宣言
  cin >> N >> M >> K; // 標準入力
  ll A[N],B[M];
  rep(i,N) cin >> A[i];
  rep(i,M) cin >> B[i] ;
  ll a[N];
  ll b[M];
  a[0]=A[0];
  b[0]=B[0];
  rep3(i,1,N){
    a[i]=a[i-1] + A[i];
    //cout << a[i] << endl;;
  }
  rep3(i,1,M){
    b[i]=b[i-1] + B[i];
    //cout << b[i]<< endl;
  }
  ll ans =0;
  
  rep(i,N){
    if(a[i]>K) break;
    ll j=M;
    while(a[i] + b[j]>K){
     
            //cout << ans << a[i] + b[j] <<endl; // 標準出力
      
        j--;
    }
    ans = max(ans,i+j+2);
  }
  cout << ans << endl; // 標準出力
	
}
