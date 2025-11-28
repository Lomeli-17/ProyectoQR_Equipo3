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
 const int N = 21;
    
    vector<vector<int>> matriz(N, vector<int>(N, -1));
    vector<vector<bool>> usado(N, vector<bool>(N, false));
    
void poner_finders_simples() {
    
        // finder arrriba izquierda (7x7)
        for(int r=0; r<7; r++){
            for(int c=0; c<7; c++){
                bool borde = (r==0 || r==6 || c==0 || c==6);
                bool centro = (r>=2 && r<=4 && c>=2 && c<=4);
    
                if(borde || centro)
                    matriz[r][c] = 1;
                else
                    matriz[r][c] = 0;
    
                usado[r][c] = true;
            }
        }
    
        // finder arriba derecha (0,14)
        for(int r=0; r<7; r++){
            for(int c=14; c<21; c++){
                int cc = c - 14;
                bool borde = (r==0 || r==6 || cc==0 || cc==6);
                bool centro = (r>=2 && r<=4 && cc>=2 && cc<=4);
    
                if(borde || centro)
                    matriz[r][c] = 1;
                else
                    matriz[r][c] = 0;
    
                usado[r][c] = true;
            }
        }
    
        // finder abajo izquierda (14,0)
        for(int r=14; r<21; r++){
            for(int c=0; c<7; c++){
                int rr = r - 14;
                bool borde = (rr==0 || rr==6 || c==0 || c==6);
                bool centro = (rr>=2 && rr<=4 && c>=2 && c<=4);
    
                if(borde || centro)
                    matriz[r][c] = 1;
                else
                    matriz[r][c] = 0;
    
                usado[r][c] = true;
            }
        }
    }
    
 void poner_timing_simples() {
    
        // horizontal (fila 6)
        for(int c=8; c<=12; c++){
            matriz[6][c] = (c % 2 == 0 ? 1 : 0);
            usado[6][c] = true;
        }
    
        // vertical (columna 6)
        for(int r=8; r<=12; r++){
            matriz[r][6] = (r % 2 == 0 ? 1 : 0);
            usado[r][6] = true;
        }
    }
    
    void reservar_format_info_simple() {
    
        // vertical cerca del finder de arriba izquierda
        for(int r=0; r<=5; r++){
            usado[r][8] = true;
        }
    
        // horizontal cerca del finder arriba izquierda
        for(int c=0; c<=5; c++){
            usado[8][c] = true;
        }
    
        // parte duplicada
        for(int r=15; r<=20; r++){
            usado[r][8] = true;
        }
    
        for(int c=13; c<=20; c++){
            usado[8][c] = true;
        }
    
        // cruces
        usado[6][8] = true;
        usado[8][6] = true;
    }
    
    void poner_dark_simple() {
        matriz[13][8] = 1;
        usado[13][8] = true;
    }
    
    void construir_matriz_basica_simple() {
    
        // Poner los patrones base
        poner_finders_simples();
        poner_timing_simples();
        reservar_format_info_simple();
        poner_dark_simple();
    }

    void imprimir_matriz() {
    
        // imprime la matriz usando ascii 254 para negro y espacio para blanco
        for(int r = 0; r < N; r++){
            for(int c = 0; c < N; c++){
    
                if(matriz[r][c] == 1){
                    cout << (char)254;  // negro
                }
                else if(matriz[r][c] == 0){
                    cout << ' ';        // blanco
                }
                else{
                    cout << '?';        // por si algo quedo sin asignar (por ahora va a ser hasta que se inserten los datos)
                }
            }
            cout << "\n";
        }
   }

int main(){
    string url;
    cout<<"Ingrese la URL: ";
    getline(cin, url);

    vector<int>datos = codificacion_de_datos(url);
    vector<int>datos_completos = estructura_de_datos(datos);

    construir_matriz_basica_simple();
    imprimir_matriz();
    
    return 0;
}
