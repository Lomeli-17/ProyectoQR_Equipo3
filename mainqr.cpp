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
// multiplicación en gf(2^8), aritmética modular en un día.
int gf_mult(int a, int b) {
    int r = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) r ^= a;
        int alto = a & 0x80;
        a <<= 1;
        if (alto) a ^= 0x11D;
        b >>= 1;
    }
    return r & 0xFF;
}

// convierte bits a bytes debido a que reed Solomon no puede trabajar con bits, solo bytes
vector<int> bitsABytes(const vector<int>& bits) {
    vector<int> out;
    for (size_t i = 0; i < bits.size(); i += 8) {
        int x = 0;
        for (int j = 0; j < 8; j++) {
            x = (x << 1) | bits[i + j];
        }
        out.push_back(x & 0xFF);
    }
    return out;
}

// corrección de errores Reed-Solomon 
vector<int> correccion_errores(const vector<int> &bitsDatos) {
    vector<int> datos = bitsABytes(bitsDatos);
    // como solo son 7 coeficientes, pues los pongo directamente
    vector<int> gen = {87, 229, 146, 149, 238, 102, 21};
    // la cantidad de bytes de corrección que se haran
    int nsym = gen.size();

    vector<int> ecc(nsym, 0);

    for (size_t idx = 0; idx < datos.size(); ++idx) {
        int d = datos[idx];
        int f = d ^ ecc[0];

        for (int i = 0; i < nsym - 1; i++)
            ecc[i] = ecc[i + 1];
        ecc[nsym - 1] = 0;
        //se ultiliza el xor para saber si se va a corregiro mo
         if (f != 0) {
            for (int i = 0; i < nsym; i++)
                ecc[i] ^= gf_mult(gen[i], f);
        }
    }
    //transformó todo otra vez a bytes

    vector<int> bitsECC;
    bitsECC.reserve(nsym * 8);
    for (int e : ecc) {
        for (int b = 7; b >= 0; b--)
            bitsECC.push_back((e >> b) & 1);
    }
    return bitsECC;
}

int main(){
    string url;
    cout<<"Ingrese la URL: ";
    getline(cin, url);

    vector<int>datos = codificacion_de_datos(url);
    vector<int>datos_completos = estructura_de_datos(datos);
    vector<int>bits_correccion = correccion_errores(datos_completos);

    return 0;
}
