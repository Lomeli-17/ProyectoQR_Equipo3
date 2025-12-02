#include <bits/stdc++.h>
using namespace std;

const int version=2; //define la versión del código qr
const int tamano=21+4*(version-1); // 25 (calcula el tamaño de la matriz)
const int total_bytes_datos=34; // v2-L (calcula los bytes totales de datos, o sea 34)
const int ECC_bytes=10; //calcula los bytes de corrección de errores, o sea 10
const int num_bloques=1; // número de bloques (v2-L tiene 1 bloque)

// GF(256) para Reed-Solomon
const int gf_polinomio=0x11d; // x^8 + x^4 + x^3 + x^2 + 1 (polinomio irreducible)
int gf_exp[512]; //tablas de exponenciación y logaritmos
int gf_log[256];

void gf_inicio() { // inicializa las tablas gf
    int x=1;
    for (int i=0;i<255;i++){ // genera tabla de exponenciación y logaritmos
        gf_exp[i]=x;
        gf_log[x]=i;
        x<<=1;
        if (x & 0x100) x ^= gf_polinomio; 
    }
    for (int i=255;i<512;i++) gf_exp[i] = gf_exp[i-255]; // para evitar el módulo en multiplicación
    gf_exp[255] = gf_exp[0]; 
}

int gf_suma(int a,int b){ return a^b; } // suma en gf es xor
int gf_mult(int a,int b){ // multiplicación en gf 
    if (a==0 || b==0) return 0;
    return gf_exp[(gf_log[a] + gf_log[b]) % 255]; 
}

// genera el polinomio generador para reed-solomon
vector<int>polinomio_generador(int bytes_correccion){
    vector<int>g={1}; //inicializa polinomio
    for (int i=0;i<bytes_correccion;i++){ //itera una vez por cada byte de correción
        vector<int>siguiente(g.size()+1, 0);
        for (size_t j=0;j<g.size();j++){  //recorre los coeficientes actuales
            // multiplica por (x + alpha^i)
            siguiente[j]^=g[j]; // siguiente[j] += g[j] (xor)
            // siguiente[j+1] += g[j]*(alpha^i)
            siguiente[j+1]^=gf_mult(g[j], gf_exp[i]);
        }
        g.swap(siguiente); //se cambia el polinomio actual por el que se acaba de calcular
    }
    return g;
}

// calcula los bytes de paridad para los datos dados
vector<int> rs_calcula_paridad(const vector<int>& datos, int bytes_correccion){
    vector<int>gen=polinomio_generador(bytes_correccion); // obtiene el polinomio generador
    vector<int>temporal(datos.begin(), datos.end());
    temporal.resize(datos.size()+bytes_correccion, 0);
    for (size_t i=0;i<datos.size();i++){ // procesa cada byte de datos
        int factor=temporal[i];
        if (factor!=0){
            for (size_t j=0;j<gen.size();j++){
                temporal[i+j]^=gf_mult(gen[j], factor);
            }
        }
    }
    vector<int> paridad(bytes_correccion); //extrae los bytes de paridad
    for (int i=0;i<bytes_correccion;i++) paridad[i] = temporal[datos.size()+i];
    return paridad; 
}

//matriz y funciones para colocar patrones
enum CellStat { sin_establecer= -1, blanco=0, negro=1, reservado=2 };

struct Matrix { //permite agrupar variables relacionadas con la matriz del código qr
    int n;
    vector<vector<int>> cell; 
    vector<vector<bool>> reservado; 
    Matrix(int n=0): n(n), cell(n, vector<int>(n, sin_establecer)), reservado(n, vector<bool>(n,false)) {}
    void set(int r,int c,int v,bool siReservado=false){ //establece el valor de una celda y si está reservada
        if (r<0||c<0||r>=n||c>=n) return;
        cell[r][c]=v;
        if (siReservado) reservado[r][c]=true;
    }
    int get(int r,int c) const { //si consulta fuera de rango devuelve blanco
        if (r<0||c<0||r>=n||c>=n) return blanco;
        return cell[r][c];
    }
    bool siReservado(int r,int c) const { //indica si una celda está reservada
        if (r<0||c<0||r>=n||c>=n) return true;
        return reservado[r][c];
    }
   void printAscii() const { //funcion para imprimir la matriz en ASCII
        for (int r=0;r<n;r++){
            for (int c=0;c<n;c++){
                int v = cell[r][c];
                bool n = (v==negro);
                cout << (n ? "\xE2\x96\x88" : " ");
            }
            cout << "\n";
        }
    }
    void toPPM(const string &fname, int scale=8) const { //funcion para guardar la matriz en un archivo PPM
        int w = n*scale;
        int h = n*scale;
        FILE* f = fopen(fname.c_str(),"wb");
        if(!f) return;
        fprintf(f,"P6\n%d %d\n255\n", w, h);
        for (int r=0;r<n;r++){
            for (int sy=0; sy<scale; sy++){
                for (int c=0;c<n;c++){
                    bool n = (cell[r][c]==negro);
                    unsigned char col[3];
                    if (n){ col[0]=col[1]=col[2]=0; }
                    else { col[0]=col[1]=col[2]=0xDD; }
                    for (int sx=0;sx<scale;sx++) fwrite(col,1,3,f);
                }
            }
        }
        fclose(f);
    }
};

// patrones fijos
void marcadores_posicion(Matrix &M, int r, int c){ //marcadores de posición
    static const int patt[7][7] = {
        {1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1},
        {1,0,1,1,1,0,1},
        {1,0,1,1,1,0,1},
        {1,0,1,1,1,0,1},
        {1,0,0,0,0,0,1},
        {1,1,1,1,1,1,1}
    };
    for (int dr=0;dr<7;dr++) for (int dc=0;dc<7;dc++){
        M.set(r+dr, c+dc, patt[dr][dc]==1 ? negro : blanco, true);
    }
    // borde blanco
    for (int i=-1;i<=7;i++){
        if (r-1>=0 && c+i>=0 && c+i<M.n) M.set(r-1,c+i,blanco,true);
        if (r+7<M.n && c+i>=0 && c+i<M.n) M.set(r+7,c+i,blanco,true);
        if (r+i>=0 && r+i<M.n && c-1>=0) M.set(r+i,c-1,blanco,true);
        if (r+i>=0 && r+i<M.n && c+7<M.n) M.set(r+i,c+7,blanco,true);
    }
}

void timing(Matrix &M){
    for (int i=8;i<M.n-8;i++){
        if (!M.siReservado(6,i)) M.set(6,i, (i%2)==0 ? negro : blanco, true);
        if (!M.siReservado(i,6)) M.set(i,6, (i%2)==0 ? negro : blanco, true);
    }
}

//patrones de alineación
void patrones_alineacion(Matrix &M, const vector<int>& centro){
    for (int cy : centro){
        for (int cx : centro){
            // evitar empalmarse con los marcadores de posición
            if ((cy==6 && (cx==6 || cx==M.n-7)) || (cy==M.n-7 && cx==6)) continue;
            for (int dy=-2; dy<=2; dy++){ 
                for (int dx=-2; dx<=2; dx++){
                    int rr=cy+dy, cc=cx+dx;
                    int val;
                    if (abs(dy)==2 || abs(dx)==2) val=negro; //borde negro, centro negro, interior blanco
                    else if (dy==0 && dx==0) val=negro;
                    else val=blanco;
                    M.set(rr,cc,val,true); 
                }
            }
        }
    }
}

void dark_module(Matrix &M){
    int r = 8;
    int c = 4*version + 9;
    M.set(r,c,negro,true); 
}

void reservar_bits_formato(Matrix &M){
    int n = M.n;
    // linea horizontal en fila 8
    for (int c=0;c<=8;c++){
        if (c==6) continue;
        if (!M.siReservado(8,c)) M.set(8,c,blanco,true);
    }
    // vertical en columna 8
    for (int r=0;r<=8;r++){
        if (r==6) continue;
        if (!M.siReservado(r,8)) M.set(r,8,blanco,true);
    }
    // copia esquina superior derecha
    for (int c = n-8; c<n; c++){
        if (!M.siReservado(8,c)) M.set(8,c,blanco,true);
    }
    // copia esquina inferior izquierda
    for (int r = n-8; r<n; r++){
        if (!M.siReservado(r,8)) M.set(r,8,blanco,true);
    }
}

// insertar datos (zig-zag)
void insertar_datos(Matrix &M, const vector<int>& bits){
    int n=M.n;
    int pos=0;
    int col=n-1;
    int dir=-1; // -1 arriba, +1 abajo
    int fila=n-1;
    while (col>0){
        if (col==6) col--; //saltar timing vertical
        for (;;){ 
            for (int c = col; c >= col-1; c--){ // dos columnas a la vez
                if (c<0 || c>=n) continue;
                if (!M.siReservado(fila,c) && M.cell[fila][c]==sin_establecer){
                    int bit=0;
                    if (pos < (int)bits.size()) bit = bits[pos++]; // obtener siguiente bit
                    M.set(fila,c, bit ? negro : blanco, false); //colocar bit
                }
            }
            fila+=dir; //mueve la posición una fila arriba o abajo según la dirección
            if (fila < 0 || fila >= n) break; //salir del bucle si llega al extremo
        }
        dir=-dir;
        fila+=dir;
        col-=2;
    }
}
//máscara
bool mascaras(int mask, int r, int c){
    switch(mask){ //evalua segun las máscaras
        case 0: return ((r + c) % 2) == 0; //invierte cuando la suma de fila y columna es par
        case 1: return (r % 2) == 0; //invierte cuando la fila es par
        case 2: return (c % 3) == 0; //invierte cuando la columna es multiplo de 3
        case 3: return ((r + c) % 3) == 0; //invierte cuando la suma es multiplo de 3
        case 4: return (((r/2) + (c/3)) % 2) == 0; //invierte cuando (fila/2)+(col/3) es par
        case 5: return (((r*c) % 2) + ((r*c) % 3)) == 0; //invierte r*c es divisible entre 6
        case 6: return ((((r*c) % 2) + ((r*c) % 3)) % 2) == 0; //invierte r*c es divisible entre 6 y además la suma entre 2
        case 7: return ((((r+c) % 2) + ((r*c) % 3)) % 2) == 0; //si la suma es par se invierte
    }
    return false; //si la máscara no es válida
}

Matrix aplicar_mascara(const Matrix &src, int mascara){
    Matrix dst = src;
    for (int r=0;r<dst.n;r++) for (int c=0;c<dst.n;c++){
        if (src.siReservado(r,c)) continue;
        int v = src.cell[r][c];
        if (v==sin_establecer) continue;
        if (mascaras(mascara,r,c)){
            dst.cell[r][c] = (v==negro) ? blanco : negro;
        }
    }
    return dst;
}

inline bool esNegro(const Matrix &M, int r, int c){
    return M.cell[r][c] == negro;
}
int penalizacion(const Matrix &M){
    int n=M.n, pen=0;

    auto procesaLinea = [&](auto obtener){
        int colorAct=-1, longitud=0;
        for (int i=0;i<n;i++){
            int v = obtener(i);
            int color = (v==negro);
            if (color == colorAct) longitud++;
            else{
                if (longitud >= 5) pen += 3 + (longitud - 5);
                colorAct = color;
                longitud = 1;
            }
        }
        if (longitud >= 5) pen += 3 + (longitud - 5);
    };

    // filas
    for (int r=0;r<n;r++)
        procesaLinea([&](int i){ return M.cell[r][i]; });

    // columnas
    for (int c=0;c<n;c++)
        procesaLinea([&](int i){ return M.cell[i][c]; });
    return pen;
}

// bloques 2x2
int penalizacion_2x2(const Matrix &M){
    int n=M.n, pen=0;

    for (int r=0;r<n-1;r++)
        for (int c=0;c<n-1;c++){
            int b =
                esNegro(M,r,c) +
                esNegro(M,r,c+1) +
                esNegro(M,r+1,c) +
                esNegro(M,r+1,c+1);
            if (b == 4) pen += 3;
        }

    return pen;
}

bool zonaBlanca(const Matrix &M, int r, int c0, int len, bool horizontal){
    for (int i=0;i<len;i++){
        int rr=r+(horizontal ? 0 : i);
        int cc=c0+(horizontal ? i : 0);
        if (esNegro(M,rr,cc)) return false;
    }
    return true;
}

int penalizacion_patron(const Matrix &M){
    int n=M.n, pen=0;

    // filas
    for (int r=0;r<n;r++){
        for (int c=0;c+6<n;c++){
            if ( esNegro(M,r,c) && !esNegro(M,r,c+1) &&
                 esNegro(M,r,c+2) && esNegro(M,r,c+3) && esNegro(M,r,c+4) &&
                 !esNegro(M,r,c+5) && esNegro(M,r,c+6) ){

                bool izq  = (c>=4)     && zonaBlanca(M,r,c-4,4,true);
                bool der = (c+10<n)   && zonaBlanca(M,r,c+7,4,true);

                if (izq || der) pen += 40;
            }
        }
    }

    // columnas
    for (int c=0;c<n;c++){
        for (int r=0;r+6<n;r++){
            if ( esNegro(M,r,c) && !esNegro(M,r+1,c) &&
                 esNegro(M,r+2,c) && esNegro(M,r+3,c) && esNegro(M,r+4,c) &&
                 !esNegro(M,r+5,c) && esNegro(M,r+6,c) ){

                bool arriba   = (r>=4)     && zonaBlanca(M,r-4,c,4,false);
                bool abajo = (r+10<n)   && zonaBlanca(M,r+7,c,4,false);

                if (arriba || abajo) pen += 40;
            }
        }
    }

    return pen;
}

int penalizacion_balance(const Matrix &M){
    int n=M.n;
    int total = n * n;
    int negros = 0;
    for (int r=0;r<n;r++)
        for (int c=0;c<n;c++)
            negros += esNegro(M,r,c);
    int ratio = (negros * 100) / total;
    return (abs(ratio - 50) / 5) * 10;
}

int total_penalizacion(const Matrix &M){ 
    return penalizacion(M)
         + penalizacion_2x2(M)
         + penalizacion_patron(M)
         + penalizacion_balance(M);
}
int bits_de_formato(int bits, int patron_mascara){
    int datos = (bits << 3) | (patron_mascara & 0x7); // 5 bits
    int d = datos << 10; 
    int gp = 0b10100110111;
    for (int i = 14; i >= 10; i--){
        if ((d >> i) & 1) d ^= (gp << (i - 10));
    }
    int residuo = d & 0x3FF;
    int formato = ((datos << 10) | residuo) ^ 0b101010000010010; 
    return formato & 0x7FFF;
}

//bits de formato
void escribir_bits_de_formato(Matrix &M, int bitsformato){
    int n = M.n;
    vector<pair<int,int>> pos1 = {
        {8,0},{8,1},{8,2},{8,3},{8,4},{8,5},{8,7},{8,8},{7,8},{5,8},{4,8},{3,8},{2,8},{1,8},{0,8}
    };
    vector<pair<int,int>> pos2 = {
        {n-1,8},{n-2,8},{n-3,8},{n-4,8},{n-5,8},{n-6,8},{n-7,8},{8,n-8},{8,n-7},{8,n-6},{8,n-5},{8,n-4},{8,n-3},{8,n-2},{8,n-1}
    };
    for (int i=0;i<15;i++){
        int bit = (bitsformato >> (14 - i)) & 1;
        auto [r1,c1] = pos1[i];
        auto [r2,c2] = pos2[i];
        M.set(r1,c1, bit?negro:blanco, true);
        M.set(r2,c2, bit?negro:blanco, true);
    }
}

// buffer (memoria temporal)
struct memoriaTemp {
    vector<int> bits;
    void agregarBits(int val, int cant){
        for (int i=cant-1;i>=0;i--) bits.push_back((val>>i)&1);
    }
    void agregarBytes(const vector<uint8_t> &bytes){
        for (auto b: bytes) for (int i=7;i>=0;i--) bits.push_back((b>>i)&1);
    }
    // funcion convertir a bytes 
    vector<uint8_t> bytes(int bytesTotales){
        vector<int> b=bits;
        int maxBits=bytesTotales*8;
        //terminar con hasta 4 ceros
        if ((int)b.size() < maxBits){
            int term = min(4, maxBits - (int)b.size());
            for (int i=0;i<term;i++) b.push_back(0);
        }
        // rellenar hasta el límite de byte
        while (b.size()%8) b.push_back(0);
        // convertir a bytes
        vector<uint8_t>salida;
        for (size_t i=0;i<b.size(); i+=8){
            uint8_t v=0;
            for (int j=0;j<8;j++)v=(v<<1) | b[i+j];
            salida.push_back(v);
        }
        static const uint8_t pads[2]={0xEC, 0x11};
        while ((int)salida.size()<bytesTotales) salida.push_back(pads[salida.size()%2]);
        salida.resize(bytesTotales);
        return salida;
    }
};

int main(){
    gf_inicio();

    cout << "Introduce la URL o texto a codificar: ";
    string input;
    getline(cin, input); // lee la entrada del usuario
    memoriaTemp bb;
    bb.agregarBits(0b0100, 4); // indicador de modo 0100
    if (input.size() > 255){ // conteo de caracteres
        cerr<<"Input demasiado largo (>=256), truncando a 255\n";
        input = input.substr(0,255);
    }
    bb.agregarBits((int)input.size(), 8);
    //bytes de datos
    vector<uint8_t> raw;
    for (unsigned char ch: input) raw.push_back(ch);
    bb.agregarBytes(raw);
    //obtener bytes finales de datos
    vector<uint8_t> datos = bb.bytes(total_bytes_datos);
    //convierte a enteros para RS
    vector<int> datosInt(datos.begin(), datos.end());
    //calcular paridad RS
    vector<int> paridad= rs_calcula_paridad(datosInt, ECC_bytes);
    //bytes finales del código QR
    vector<int> bytes_finales = datosInt;
    bytes_finales.insert(bytes_finales.end(), paridad.begin(), paridad.end());

    // convertir a bits finales
    vector<int> bits_finales;
    for (int cw: bytes_finales){
        for (int i=7;i>=0;i--) bits_finales.push_back( (cw>>i)&1 );
    }

    //construir la matriz base con patrones fijos
    Matrix base(tamano);
    marcadores_posicion(base, 0, 0);
    marcadores_posicion(base, 0, tamano-7);
    marcadores_posicion(base, tamano-7, 0);
    timing(base);
    vector<int> centros={6, 18};
    patrones_alineacion(base, centros);
    dark_module(base);
    reservar_bits_formato(base);

    // colocar datos (bits en zig-zag)
    Matrix conDatos = base;
    insertar_datos(conDatos, bits_finales);

    // elegir la mejor máscara
    int mejor=0;
    int mejorPenalizacion=INT_MAX;
    Matrix mejorMat;
    for (int mask=0; mask<8; mask++){ // probar cada máscara
        Matrix m = conDatos;
        for (int r=0;r<m.n;r++) for (int c=0;c<m.n;c++){ // aplicar máscara
            if (m.siReservado(r,c)) continue;
            if (m.cell[r][c]==sin_establecer) continue;
            if (mascaras(mask,r,c)) m.cell[r][c] = (m.cell[r][c]==negro) ? blanco : negro;
        }
        // escribir bits de formato
        int ec_bits = 0b01;
        int fbits = bits_de_formato(ec_bits, mask);
        escribir_bits_de_formato(m, fbits);

        int pen = total_penalizacion(m);
        if (pen < mejorPenalizacion){
            mejorPenalizacion = pen;
            mejor = mask;
            mejorMat = m;
        }
    }

    cout<<"Código QR:\n";
    cout<<"\n\n";
    mejorMat.printAscii();
    mejorMat.toPPM("out.ppm", 8);
    cout << "\n\n";

    return 0;
}
