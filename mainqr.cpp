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


vector<int>estructura_de_datos(const vector<int> &bits_iniciales){
    vector<int>bits = bits_iniciales;  
    // copio los bits que hizo Renata para trabajar sobre ellos

    int tamano_total = 152;  
    // esta versión del QR ocupa 152 bits para los datos

    bool alternar = true;  
    // sirve para ir cambiando entre EC y 11 como pide la norma

    while(bits.size() < tamano_total){
        // mientras todavía falten bits, sigo agregando los bytes de relleno

        if(alternar){
            int pad = 0xEC;  
            // primer byte de relleno (11101100)
            for(int b = 7; b >= 0; b--){
                bits.push_back((pad >> b) & 1);  
                // meto cada bit del byte al vector
            }
        }else{
            int pad = 0x11;  
            // segundo byte de relleno (00010001)
            for(int b = 7; b >= 0; b--){
                bits.push_back((pad >> b) & 1);  
                // igual convierto este byte a bits y lo agrego
            }
        }

        alternar = !alternar;  
        // cambio para que el siguiente sea el otro byte
    }

    if(bits.size() > tamano_total){
        bits.resize(tamano_total);  
        // por si me paso por error, lo dejo exacto en 152 bits
    }

    return bits;  
    // regreso los bits ya completos
}

int main(){
    string url;
    cout<<"Ingrese la URL: ";
    getline(cin, url);

    vector<int>datos = codificacion_de_datos(url);
    vector<int>datos_completos = estructura_de_datos(datos);

    return 0;
}
