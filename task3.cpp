// =============================================================================
// TASK 3 - Costruzione della matrice A e del termine noto
// =============================================================================
//
// Uso:      ./task3 naturale      ordinamento naturale (quello di coords.txt)
//           ./task3 riordinato    ordinamento di ordering.txt (nested dissection)
// Input:    coords.txt            righe "n i j x y", prodotto da task1
//           ordering.txt          righe "m n", prodotto da task2 (solo se
//                                 si sceglie "riordinato")
// Output:   A.txt                 righe "r c A[r,c]", solo le entrate non nulle
//           rhs.txt               riga r = termine noto dell'equazione r
// Gli indici r, c sono quelli del NUOVO ordinamento.
//
// DAL PROBLEMA AL SISTEMA. In ogni punto interno (x_i, y_j) imponiamo
//     k/h^2 (u_{i-1,j} + u_{i+1,j} + u_{i,j-1} + u_{i,j+1} - 4 u_{i,j})
//         + f(x_i, y_j) = 0,
// con k = 0.01 e f(x, y) = exp(-10 (x^2 + y^2)). Ogni nodo interno da' una
// equazione, cioe' una riga di A u = b. Portando f a destra:
//   - diagonale (il nodo stesso):        A = -4 k/h^2
//   - ogni vicino INTERNO:               A = +k/h^2
//   - tutte le altre colonne:            A = 0 (non scritte nel file)
//   - termine noto:                      b = -f(x_i, y_j)
//
// PUNTI DI BORDO. Se un vicino sta sul bordo (i o j uguale a 0 o N+1), il suo
// valore u0 e' noto: non e' un'incognita e non ha una colonna in A. Il suo
// termine k/h^2 * u0 si porta a destra, sottraendolo da b:
//     b = -f(x_i, y_j) - k/h^2 * (somma di u0 sui vicini di bordo).
// Con u0 = 0 (caso del testo) il contributo e' nullo, ma il codice lo calcola
// comunque: per usare un altro u0 basta cambiare la funzione u0 qui sotto
// (parte opzionale del task), e il termine noto resta coerente.
// Esempio N = 3: il nodo 0, nell'angolo in basso a sinistra, ha solo due
// vicini interni (1 a destra, 3 sopra): la sua riga ha 3 entrate non nulle,
// e i vicini a sinistra e sotto stanno sul bordo.
//
// RIORDINAMENTO. Riordinare le incognite significa permutare righe e colonne
// di A con la stessa permutazione: il nodo n diventa l'incognita nuovo[n].
// L'entrata tra i nodi a e b finisce in posizione (nuovo[a], nuovo[b]).
// La matrice resta simmetrica; cambia solo dove stanno i non zeri.
//
// PROPRIETA' DI A (utili come test e all'orale):
//   - quadrata N^2 x N^2 e simmetrica (se a e' vicino di b, b e' vicino di a);
//   - le entrate fuori diagonale corrispondono agli archi di connectivity.txt,
//     ciascun arco due volte (posizioni (a,b) e (b,a)), quindi
//     non zeri = N^2 + 2|E| = N^2 + 4N(N-1); per N = 3: 9 + 24 = 33;
//   - definita negativa: per questo nel Task 4 si fattorizza -A = L L^T.
//
// COMPLESSITA' (modello RAM):
//   - Tempo: Theta(N^2). Lettura dei file Theta(N^2); per ogni riga si
//     esaminano 4 vicini in O(1) grazie alla tabella griglia[j][i].
//   - Memoria: Theta(N^2) per tabelle e permutazione. La matrice non viene
//     memorizzata: le entrate si scrivono su file appena calcolate.
// =============================================================================

#include <cmath>     // std::exp, per la funzione f
#include <cstddef>   // std::size_t
#include <fstream>   // std::ifstream e std::ofstream, per leggere e scrivere file
#include <iomanip>   // std::setprecision
#include <iostream>  // std::cout, std::cerr
#include <string>    // std::string, per l'argomento da riga di comando
#include <vector>    // std::vector

// Costante di diffusione del problema.
const double K = 0.01;

// Sorgente di calore f(x, y) = exp(-10 (x^2 + y^2)).
double f(double x, double y) {
    return std::exp(-10.0 * (x * x + y * y));
}

// Temperatura sul bordo. Il testo usa u0 = 0; per la parte opzionale basta
// cambiare questa funzione (ad esempio return x;), il resto si adatta.
double u0(double x, double y) {
    (void)x;  // evita il warning "parametro non usato" finche' u0 = 0
    (void)y;
    return 0.0;
}

// Un nodo cosi' come compare in una riga di coords.txt.
struct Nodo {
    long n;      // indice originale
    int i, j;    // indici di griglia, da 1 a N
    double x, y; // coordinate
};

int main(int argc, char* argv[]) {
    // ---- Scelta dell'ordinamento -------------------------------------------
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " naturale|riordinato\n";
        return 1;
    }
    std::string scelta = argv[1];
    if (scelta != "naturale" && scelta != "riordinato") {
        std::cerr << "Errore: l'argomento deve essere naturale o riordinato\n";
        return 1;
    }

    // ---- Lettura di coords.txt ---------------------------------------------
    std::ifstream inCoords("coords.txt");
    if (!inCoords) {
        std::cerr << "Errore: impossibile aprire coords.txt "
                     "(eseguire prima ./task1 N)\n";
        return 1;
    }
    std::vector<Nodo> nodi;
    Nodo nd;
    while (inCoords >> nd.n >> nd.i >> nd.j >> nd.x >> nd.y)
        nodi.push_back(nd);

    // Ricaviamo N e controlliamo che i nodi siano N^2, come nel Task 2.
    int N = 0;
    for (const Nodo& p : nodi)
        if (p.i > N) N = p.i;
    long numNodi = static_cast<long>(nodi.size());
    if (N == 0 || numNodi != static_cast<long>(N) * N) {
        std::cerr << "Errore: coords.txt non contiene N^2 nodi\n";
        return 1;
    }

    // Per l'invariante del Task 1 la riga k di coords.txt e' il nodo k:
    // lo controlliamo, perche' piu' avanti usiamo nodi[n] per trovare il nodo n.
    for (long k = 0; k < numNodi; ++k) {
        if (nodi[k].n != k) {
            std::cerr << "Errore: gli indici di coords.txt non sono 0, 1, 2, ...\n";
            return 1;
        }
    }

    // Tabella posizione -> nodo, come nel Task 2: griglia[j][i] = n.
    // La dimensione (N+2) x (N+2) include anche i punti di bordo (indici 0 e
    // N+1), che marchiamo con -1: cosi' "vicino sul bordo" si riconosce con
    // un semplice griglia[j][i] == -1, senza casi particolari.
    std::vector<std::vector<long>> griglia(N + 2, std::vector<long>(N + 2, -1));
    for (const Nodo& p : nodi)
        griglia[p.j][p.i] = p.n;

    // ---- Permutazione ------------------------------------------------------
    // ordine[m] = n: il nodo che diventa l'incognita m (scorre le righe di A).
    // nuovo[n]  = m: la posizione del nodo n nel nuovo ordine (da' le colonne).
    // Sono una l'inversa dell'altra.
    // Entrambi partono da -1 ("non ancora assegnato"): cosi' si riconosce sia
    // un nodo n ripetuto (nuovo[n] gia' assegnato) sia un nuovo indice m
    // ripetuto (ordine[m] gia' assegnato).
    std::vector<long> ordine(numNodi, -1), nuovo(numNodi, -1);

    if (scelta == "naturale") {
        // Ordinamento naturale: l'incognita m e' il nodo m.
        for (long n = 0; n < numNodi; ++n) {
            ordine[n] = n;
            nuovo[n] = n;
        }
    } else {
        std::ifstream inOrd("ordering.txt");
        if (!inOrd) {
            std::cerr << "Errore: impossibile aprire ordering.txt "
                         "(eseguire prima ./task2)\n";
            return 1;
        }
        long m, n, righe = 0;
        while (inOrd >> m >> n) {
            // Controlli: indici nel range, nessun nodo n ripetuto e nessun
            // nuovo indice m ripetuto. Una permutazione e' una corrispondenza
            // biunivoca, quindi va controllata in ENTRAMBE le direzioni:
            // controllando solo n, un m ripetuto lascerebbe una riga di A
            // senza nodo (ordine[m] resterebbe al valore iniziale).
            if (m < 0 || m >= numNodi || n < 0 || n >= numNodi ||
                nuovo[n] != -1 || ordine[m] != -1) {
                std::cerr << "Errore: ordering.txt non e' una permutazione valida\n";
                return 1;
            }
            ordine[m] = n;
            nuovo[n] = m;
            ++righe;
        }
        // Ne' n ne' m ripetuti + N^2 righe = vera permutazione: ogni nodo ha
        // esattamente un nuovo indice e ogni nuovo indice un solo nodo.
        if (righe != numNodi) {
            std::cerr << "Errore: ordering.txt non ha N^2 righe "
                         "(generato con un N diverso?)\n";
            return 1;
        }
    }

    // ---- Apertura dei file di output ---------------------------------------
    std::ofstream outA("A.txt");
    if (!outA) {
        std::cerr << "Errore: impossibile creare A.txt\n";
        return 1;
    }
    std::ofstream outB("rhs.txt");
    if (!outB) {
        std::cerr << "Errore: impossibile creare rhs.txt\n";
        return 1;
    }
    outA << std::setprecision(17);
    outB << std::setprecision(17);

    // ---- Costruzione del sistema -------------------------------------------
    double h = 1.0 / (N + 1);
    double c = K / (h * h);  // coefficiente k/h^2, calcolato una volta sola

    // Spostamenti verso i 4 vicini: sinistra, destra, sotto, sopra.
    const int di[4] = {-1, +1, 0, 0};
    const int dj[4] = {0, 0, -1, +1};

    long nonZeri = 0;
    // Scorriamo le righe NEL NUOVO ORDINE: la riga r e' l'equazione del nodo
    // ordine[r]. Cosi' A.txt e rhs.txt sono gia' ordinati per riga, e la riga
    // r di rhs.txt corrisponde alle righe di A.txt con primo indice r.
    for (long r = 0; r < numNodi; ++r) {
        const Nodo& p = nodi[ordine[r]];

        // Diagonale: coefficiente di u_{i,j} nella sua stessa equazione.
        outA << r << ' ' << r << ' ' << -4.0 * c << '\n';
        ++nonZeri;

        double b = -f(p.x, p.y);  // termine noto, prima dei contributi di bordo

        for (int d = 0; d < 4; ++d) {
            int vi = p.i + di[d];
            int vj = p.j + dj[d];
            long v = griglia[vj][vi];
            if (v != -1) {
                // Vicino interno: e' un'incognita, colonna nuovo[v].
                outA << r << ' ' << nuovo[v] << ' ' << c << '\n';
                ++nonZeri;
            } else {
                // Vicino di bordo: valore noto, si porta a destra.
                b -= c * u0(vi * h, vj * h);
            }
        }
        outB << b << '\n';
    }

    // Stampa di controllo: i non zeri devono essere N^2 + 4N(N-1).
    std::cout << "Ordinamento " << scelta << ", N = " << N
              << ", non zeri: " << nonZeri
              << " (attesi " << numNodi + 4L * N * (N - 1) << ")\n";
    return 0;
}