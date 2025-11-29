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
 const int N = 21; 
    vector<vector<int>> matriz(N, vector<int>(N, -1));
    vector<vector<bool>> usado(N, vector<bool>(N, false));
    
void poner_finder_con_separador(int r0, int c0) {
    for (int r = 0; r < 7; ++r) {
        for (int c = 0; c < 7; ++c) {
            bool borde = (r == 0 || r == 6 || c == 0 || c == 6);
            bool centro = (r >= 2 && r <= 4 && c >= 2 && c <= 4);
            matriz[r0 + r][c0 + c] = (borde || centro) ? 1 : 0;
            usado[r0 + r][c0 + c] = true;
        }
    }
    // separador blanco 
    for (int r = -1; r <= 7; ++r) {
        for (int c = -1; c <= 7; ++c) {
            int rr = r0 + r, cc = c0 + c;
            if (rr < 0 || rr >= N || cc < 0 || cc >= N) continue;
            if (r >= 0 && r <= 6 && c >= 0 && c <= 6) continue; // no pisar el finder
            matriz[rr][cc] = 0;
            usado[rr][cc] = true;
            }
         }
    }
void poner_finders() {
    poner_finder_con_separador(0, 0);      // arriba-izquierda (fila 0, col 0)
    poner_finder_con_separador(0, N - 7); // arriba-derecha (fila 0, col 14)
    poner_finder_con_separador(N - 7, 0); // abajo-izquierda (fila 14, col 0)
} 

 void poner_timing_simples() {
       for (int c = 0; c < N; ++c) {
        if (!usado[6][c]) { matriz[6][c] = (c % 2 == 0) ? 1 : 0; usado[6][c] = true; }
        }
    for (int r = 0; r < N; ++r) {
        if (!usado[r][6]) { matriz[r][6] = (r % 2 == 0) ? 1 : 0; usado[r][6] = true; }
        }
    }
    
   void reservar_format_info() {
    // copia original (arriba/izquierda de la matriz).
    for (int c = 0; c <= 8; ++c) {
        if (!usado[8][c]) { matriz[8][c] = 0; usado[8][c] = true; }
    }
    for (int r = 0; r <= 8; ++r) {
        if (!usado[r][8]) { matriz[r][8] = 0; usado[r][8] = true; }
    }
    // copia espejo (abajo/derecha de la matriz).
    for (int i = 0; i <= 8; ++i) {
        int c = N - 1 - i; // columnas N-1 ... N-9
        if (c >= 0 && !usado[8][c]) { matriz[8][c] = 0; usado[8][c] = true; }
        int r = N - 1 - i; // filas N-1 ... N-9
        if (r >= 0 && !usado[r][8]) { matriz[r][8] = 0; usado[r][8] = true; }
    }
}
    
   void poner_dark_module() {
    int rr = 8;
    int cc = N - 8;
    if (!usado[rr][cc]) {
        matriz[rr][cc] = 1;
        usado[rr][cc] = true;
    }
}
    
  void construir_matriz_basica_simple() {
    for (int r = 0; r < N; ++r) for (int c = 0; c < N; ++c) { matriz[r][c] = -1; usado[r][c] = false; }
    poner_finders();
    poner_timing_simples();
    reservar_format_info();
    poner_dark_module();
}

void insertar_datos(const vector<int> &bitsDatos, const vector<int> &bitsECC) {
    vector<int> bitsTotales = bitsDatos;
    bitsTotales.insert(bitsTotales.end(), bitsECC.begin(), bitsECC.end());

    int i = 0;
    int col = N - 1;
    bool hacia_arriba = true; // el primer par (20,19) va hacia arriba

    while (col > 0) {
        if (col == 6) { // se salta la columna 6 por el timing
            col--;
            //par siguiente conserva la misma dirección
            if (col <= 0) break;
        }
        // definir las 2 columnas actuales
        int c1 = col;
        int c2 = col - 1;
        
        if (hacia_arriba) { //de abajo hacia arriba
            for (int r = N - 1; r >= 0; --r) {
                for (int j = 0; j < 2; ++j) {
                    int c = (j == 0) ? c1 : c2;
                    if (c < 0 || c >= N) continue;
                    if (usado[r][c]) continue;
                    matriz[r][c] = (i < (int)bitsTotales.size()) ? bitsTotales[i++] : 0;
                    usado[r][c] = true;
                }
            }
        } else { // de arriba hacia abajo
            for (int r = 0; r < N; ++r) {
                for (int j = 0; j < 2; ++j) {
                    int c = (j == 0) ? c1 : c2;
                    if (c < 0 || c >= N) continue;
                    if (usado[r][c]) continue; // si ya está usado lo salta
                    matriz[r][c] = (i < (int)bitsTotales.size()) ? bitsTotales[i++] : 0;
                    usado[r][c] = true; // marca como usado
                }
            }
        }

        // avanza al siguiente par de columnas y alterna  la dirección
        col -= 2;
        hacia_arriba = !hacia_arriba;
    }
}

void imprimir_matriz() {
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (matriz[r][c] == 1) cout << "██";
            else cout << "  ";
        }
        cout << '\n';
    }
}
/*    void imprimir_matriz() {
    
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
        CAMBIE ESTA FUNCIÓN SEGÚN YO PARA QUE SE VEA MEJOR, PERO NO SÉ QUE FORMATO PREFIERAN DEJAR*/

int main(){
    string url;
    cout<<"Ingrese la URL: ";
    getline(cin, url);

    vector<int>datos = codificacion_de_datos(url);
    vector<int>datos_completos = estructura_de_datos(datos);
    vector<int>bits_correccion = correccion_errores(datos_completos);

    construir_matriz_basica_simple();
    insertar_datos(datos_completos, bits_correccion);
    imprimir_matriz();
    
    return 0;
}
