#include<iostream>
#include<string>
#include<vector>
using namespace std;

vector<int>codificacion_de_datos(const string &s){
    vector<int>bits;
    // modo bytes (0100=4)
    int indicador=0b0100;
    for(int b=3; b>=0; --b)
    bits.push_back((indicador>>b)&1);
    // longitud del url
    int longitud=s.size();
    for(int b=7; b>=0; --b)
    bits.push_back((longitud>>b)&1);
    // datos, cada caracter en 8 bits (ascii)
    for(unsigned char c:s){
        for(int b=7; b>=0; --b){
            bits.push_back((c>>b) &1);
        }
    }
    // bits de relleno
    while(bits.size()%8 !=0){
        bits.push_back(0);
    }
    
    return bits;
}

int main(){
    string url;
    cout<<"Ingrese la URL: ";
    getline(cin, url);

    vector<int>datos=codificacion_de_datos(url);
 
    return 0;
}
