#include <bits/stdc++.h>
using namespace std;

int main(){
long long n,count;
  cin>>n;
  count=0;
  vector<long long> a(n);
  for(long long i=0;i<n;i++){
cin>>a.at(i);
  }
  sort(a.begin(),a.end());
  for(long long i=0;i<n;i++){
for(long long j=i-1;j<n;j++){
  if(i>0){
  if(a.at(i)==a.at(i-1)){
count++;
    break;
  }
  }
if(a.at(j)%a.at(j)==0){
count++;
  break;
}
}
  }
  

  cout<<n-count<<endl;
}
