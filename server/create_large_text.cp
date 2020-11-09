#include <iostream>
#define ll long long int
using namespace std;
int main(){
    ll size;
    cin >> size;
    size = 1024 * 1024 * size;
    cout << "The no of ch. are " << size <<"\n\n";
    for(ll i=0;i<size;i++){
        printf("%c",'a'+ i%25);
        if(i%6==0) cout << " ";
        if(i%30==0) cout << "\n";
    }
    return 0;
}