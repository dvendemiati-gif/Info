// =============================================================================
// TASK 2 - Riordinamento dei nodi di tipo nested dissection
// =============================================================================
//
// Uso:      ./task2           (legge coords.txt dalla cartella corrente)
// Input:    coords.txt        righe "n i j x y", prodotto da task1
// Output:   ordering.txt      righe "m n": m = nuovo indice (0..N^2-1),
//                             n = indice originale del nodo in coords.txt
//
// OBIETTIVO. Dividere i nodi in V = V1 u V2 u VS in modo che:
//   - V1 e V2 siano disgiunti e nessun arco li colleghi (VS li "separa");
//   - VS sia piccolo;
//   - V1 e V2 abbiano circa la stessa cardinalita'.
//
// EURISTICA GEOMETRICA. Trovare il separatore ottimo e' costoso in generale.
// Tagliamo invece il dominio con una retta x = x^ che passa per una colonna
// di nodi: x < x^ -> V1, x > x^ -> V2, x = x^ -> VS.
// Funziona perche' un arco collega solo nodi della stessa colonna (arco
// verticale) o di colonne adiacenti (arco orizzontale, i differisce di 1):
// per passare da sinistra a destra bisogna attraversare la colonna di taglio.
// Lo stesso vale per un taglio orizzontale y = y^.
//
// RICORSIONE (nested dissection). L'ordine finale e'
//     [ordine di V1] [ordine di V2] [nodi di VS]
// dove V1 e V2 si ordinano con la stessa idea, alternando tagli verticali e
// orizzontali come chiede il testo. Il separatore va SEMPRE dopo le due
// parti che separa: cosi' il separatore piu' grande (la colonna centrale di
// tutta la griglia) finisce in fondo, e quelli piccoli all'inizio.
// Motivo: nella matrice A le entrate tra nodi di V1 e di V2 sono nulle
// (non ci sono archi); mettendo VS per ultimo, la fattorizzazione di
// Cholesky conserva nulli quei blocchi e il fattore L resta piu' sparso.
//
// PERCHE' TAGLI ALTERNATI. Conviene tagliare perpendicolarmente al lato piu'
// lungo, cosi' il separatore e' lungo quanto il lato corto. Partendo da un
// quadrato, un taglio verticale produce rettangoli alti e stretti, che vanno
// tagliati in orizzontale, e cosi' via: da qui l'alternanza.
//
// CASO BASE. Per tagliare un lato servono almeno 3 nodi (uno a sinistra, uno
// nel separatore, uno a destra). Se il lato di turno e' troppo corto si prova
// l'altra direzione; se entrambi i lati hanno meno di 3 nodi il blocco non si
// divide: si aggiungono i suoi nodi e la ricorsione termina.
//
// ESEMPIO N = 3 (risultato atteso, usato come test):
//     j=3:  6 7 8
//     j=2:  3 4 5        ordine finale:  0 6 3 | 2 8 5 | 1 4 7
//     j=1:  0 1 2                         V1      V2      VS
//
// COMPLESSITA' (modello RAM):
//   - Tempo: Theta(N^2). La lettura di coords.txt costa Theta(N^2); nella
//     ricorsione ogni nodo viene aggiunto alla lista esattamente una volta
//     (in un separatore o in un caso base) e ogni chiamata fa lavoro O(1)
//     oltre ai nodi che aggiunge. Le chiamate sono al piu' O(N^2).
//   - Profondita' della ricorsione: ogni taglio dimezza un lato, quindi
//     circa 2*log2(N) livelli (circa 20 per N = 1024): nessun rischio di
//     esaurire lo stack.
//   - Memoria: Theta(N^2), per la tabella dei nodi e per la lista finale.
//
// NOTA. connectivity.txt non viene letto: l'euristica e' geometrica e usa
// solo la posizione dei nodi (gli indici di griglia i, j di coords.txt).
// =============================================================================

#include <cstddef>   // std::size_t, tipo senza segno per indici e dimensioni
#include <fstream>   // std::ifstream e std::ofstream, per leggere e scrivere file
#include <iostream>  // std::cout, std::cerr
#include <vector>    // std::vector, array dinamico

// -----------------------------------------------------------------------------
// Un nodo cosi' come compare in una riga di coords.txt.
// x e y non servono all'algoritmo (usiamo gli interi i, j), ma li leggiamo
// per consumare la riga intera e restare fedeli al formato del file.
// -----------------------------------------------------------------------------
struct Nodo {
    long n;      // indice originale del nodo
    int i, j;    // indici di griglia: colonna i, riga j, da 1 a N
    double x, y; // coordinate del punto
};

// -----------------------------------------------------------------------------
// Variabili globali condivise dalle funzioni ricorsive.
// Usarle evita di passare gli stessi vettori come parametro a ogni chiamata.
// -----------------------------------------------------------------------------

// Tabella posizione -> nodo: griglia[j][i] = indice n del nodo in (i, j).
// La costruiamo leggendo coords.txt invece di usare la formula del Task 1:
// cosi' questo programma dipende solo dai dati del file. Accesso in O(1).
std::vector<std::vector<long>> griglia;

// La lista dei nodi nel nuovo ordine: ordine[m] = n.
std::vector<long> ordine;

// -----------------------------------------------------------------------------
// Aggiunge in fondo alla lista tutti i nodi del rettangolo [i0,i1] x [j0,j1].
// Si usa per i separatori (rettangoli larghi o alti 1) e per i casi base.
// Costo: proporzionale al numero di nodi aggiunti (push_back e' O(1)
// ammortizzato, e qui O(1) vero grazie al reserve fatto nel main).
// -----------------------------------------------------------------------------
void aggiungiBlocco(int i0, int i1, int j0, int j1) {
    for (int j = j0; j <= j1; ++j)
        for (int i = i0; i <= i1; ++i)
            ordine.push_back(griglia[j][i]);
}

// -----------------------------------------------------------------------------
// Ordina ricorsivamente il blocco con colonne i0..i1 e righe j0..j1.
// verticale = true se a questo livello tocca un taglio verticale (x = x^),
// false per un taglio orizzontale (y = y^). A ogni chiamata ricorsiva il
// valore si inverte: e' cosi' che si ottiene l'alternanza dei tagli.
// -----------------------------------------------------------------------------
void ordina(int i0, int i1, int j0, int j1, bool verticale) {
    int larghezza = i1 - i0 + 1;  // numero di colonne del blocco
    int altezza   = j1 - j0 + 1;  // numero di righe del blocco

    // Caso base: entrambi i lati hanno meno di 3 nodi, non si puo' dividere.
    if (larghezza < 3 && altezza < 3) {
        aggiungiBlocco(i0, i1, j0, j1);
        return;
    }

    // Se il lato da tagliare e' troppo corto, si taglia nell'altra direzione
    // (che a questo punto ha sicuramente almeno 3 nodi).
    if (verticale && larghezza < 3) verticale = false;
    else if (!verticale && altezza < 3) verticale = true;

    if (verticale) {
        // Colonna centrale c: e' il separatore VS, con x^ = x_c.
        // Divisione intera: con larghezza pari le due parti differiscono di
        // un nodo per riga, che va bene ("approssimativamente" bilanciate).
        int c = (i0 + i1) / 2;
        ordina(i0, c - 1, j0, j1, false);  // V1: colonne a sinistra
        ordina(c + 1, i1, j0, j1, false);  // V2: colonne a destra
        aggiungiBlocco(c, c, j0, j1);      // VS: per ultimo
    } else {
        // Riga centrale r: separatore orizzontale, con y^ = y_r.
        int r = (j0 + j1) / 2;
        ordina(i0, i1, j0, r - 1, true);   // V1: righe sotto
        ordina(i0, i1, r + 1, j1, true);   // V2: righe sopra
        aggiungiBlocco(i0, i1, r, r);      // VS: per ultimo
    }
}

int main() {
    // ---- Lettura di coords.txt ---------------------------------------------
    std::ifstream in("coords.txt");
    if (!in) {
        std::cerr << "Errore: impossibile aprire coords.txt "
                     "(eseguire prima ./task1 N)\n";
        return 1;  // valore di ritorno diverso da 0 = errore
    }

    std::vector<Nodo> nodi;
    Nodo nd;
    // L'operatore >> legge un valore alla volta, saltando spazi e a capo.
    // Il ciclo si ferma quando la lettura fallisce, cioe' a fine file.
    while (in >> nd.n >> nd.i >> nd.j >> nd.x >> nd.y)
        nodi.push_back(nd);

    // ---- Ricostruzione di N e controllo del file ---------------------------
    // N e' il massimo indice di colonna. I nodi devono essere esattamente N^2,
    // altrimenti il file e' incompleto o non generato da task1.
    // "const Nodo&": scorriamo il vettore per riferimento, senza copiare
    // ogni nodo, e const garantisce che non lo modifichiamo.
    int N = 0;
    for (const Nodo& p : nodi)
        if (p.i > N) N = p.i;
    if (N == 0 || static_cast<long>(nodi.size()) != static_cast<long>(N) * N) {
        std::cerr << "Errore: coords.txt non contiene N^2 nodi\n";
        return 1;
    }

    // ---- Tabella posizione -> nodo -----------------------------------------
    // Dimensione (N+1) x (N+1): usiamo gli indici da 1 a N, mentre la riga e
    // la colonna 0 restano inutilizzate. Cosi' griglia[j][i] usa gli stessi
    // i, j del testo, senza sottrarre 1. Il valore -1 segna "nessun nodo".
    griglia.assign(N + 1, std::vector<long>(N + 1, -1));
    for (const Nodo& p : nodi)
        griglia[p.j][p.i] = p.n;

    // ---- Riordinamento -----------------------------------------------------
    // reserve: riserva subito lo spazio per N^2 elementi, cosi' push_back non
    // deve mai riallocare e ricopiare il vettore mentre cresce.
    ordine.reserve(nodi.size());
    // Prima chiamata: tutta la griglia, con un taglio verticale (x = x^).
    ordina(1, N, 1, N, true);

    // ---- Controllo: l'ordine deve essere una permutazione ------------------
    // Ogni n da 0 a N^2-1 deve comparire esattamente una volta. visto[n]
    // diventa true la prima volta che incontriamo n: se lo troviamo gia'
    // true, il nodo e' duplicato.
    std::vector<bool> visto(nodi.size(), false);
    for (long n : ordine) {
        if (n < 0 || n >= static_cast<long>(nodi.size()) || visto[n]) {
            std::cerr << "Errore: l'ordinamento non e' una permutazione\n";
            return 1;
        }
        visto[n] = true;
    }
    // Nessun duplicato + stesso numero di elementi = nessun nodo mancante.
    if (ordine.size() != nodi.size()) {
        std::cerr << "Errore: mancano dei nodi nell'ordinamento\n";
        return 1;
    }

    // ---- Scrittura di ordering.txt -----------------------------------------
    std::ofstream out("ordering.txt");
    if (!out) {
        std::cerr << "Errore: impossibile creare ordering.txt\n";
        return 1;
    }

    // La posizione m nella lista e' il nuovo indice del nodo ordine[m].
    // std::size_t e' il tipo di ordine.size(): usarlo per m evita il
    // confronto tra intero con segno e senza segno (warning del compilatore).
    for (std::size_t m = 0; m < ordine.size(); ++m)
        out << m << ' ' << ordine[m] << '\n';
        // '\n' invece di std::endl: niente flush a ogni riga, molto piu' veloce.

    // Stampa di controllo.
    std::cout << "Riordinati " << ordine.size() << " nodi (N = " << N << ")\n";

    // I file si chiudono da soli alla fine di main: gli oggetti ifstream e
    // ofstream vengono distrutti e il loro distruttore chiude il file.
    return 0;
}