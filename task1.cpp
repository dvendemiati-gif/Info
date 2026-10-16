// =============================================================================
// TASK 1 - Generazione della griglia e del grafo di adiacenza
// =============================================================================
//
// Uso:      ./task1 N
// Output:   coords.txt        righe "n i j x y"   (un punto interno per riga)
//           connectivity.txt  righe "e n1 n2"     (un arco per riga)
//
// Il dominio e' il quadrato [0,1]^2, discretizzato con i punti
//     x_i = i*h,  y_j = j*h,  h = 1/(N+1),  i, j = 0, ..., N+1.
// I punti con i o j uguale a 0 oppure a N+1 stanno sul bordo: li escludiamo,
// perche' li' la temperatura e' nota (u0) e non sono incognite del sistema.
// Restano quindi i punti interni con i, j = 1, ..., N, cioe' N^2 nodi.
//
// Due nodi sono collegati da un arco se sono vicini nella griglia in
// orizzontale o in verticale (non in diagonale). La relazione e' simmetrica,
// quindi il grafo e' NON orientato: l'arco {a, b} e' lo stesso di {b, a}.
//
// CONTEGGI (utili come test del programma):
//   - nodi  |V| = N^2
//   - archi |E| = 2N(N-1): ogni riga ha N-1 archi orizzontali e le righe
//     sono N, quindi N(N-1); lo stesso vale per i verticali.
//     Esempio N = 3: 2*3*2 = 12 archi.
//
// COMPLESSITA' (modello RAM):
//   - Tempo: Theta(N^2). Ogni nodo viene visitato una volta e per ciascuno
//     si fanno al piu' 2 operazioni di costo costante (O(1)) per gli archi.
//     Il costo e' quindi lineare nella dimensione dell'output (|V| + |E|),
//     e non si puo' fare meglio: l'output stesso ha Theta(N^2) righe.
//   - Memoria aggiuntiva: O(1). Non memorizziamo nodi o archi in un vettore:
//     li scriviamo su file appena li calcoliamo. Possiamo farlo perche' la
//     formula dell'indice ci da' i vicini senza dover cercare nulla.
// =============================================================================

#include <cstdlib>   // std::atoi, per convertire l'argomento da testo a intero
#include <fstream>   // std::ofstream, per scrivere i file
#include <iomanip>   // std::setprecision, per il numero di cifre stampate
#include <iostream>  // std::cout, std::cerr

// -----------------------------------------------------------------------------
// Numerazione dei nodi PER RIGHE:
//     n = (j-1)*N + (i-1),   con i, j = 1, ..., N   ->   n = 0, ..., N^2 - 1
// Si fissa la riga j e si scorrono le colonne i, poi si passa alla riga dopo.
// Esempio con N = 3 (y cresce verso l'alto):
//     j=3:  6 7 8
//     j=2:  3 4 5
//     j=1:  0 1 2
//
// Vantaggio: i vicini si calcolano con una somma, in tempo O(1):
//     destra (i+1, j) -> n + 1        sinistra (i-1, j) -> n - 1
//     sopra  (i, j+1) -> n + N        sotto    (i, j-1) -> n - N
//
// Perche' "long" e non "int"? L'indice arriva fino a N^2 - 1. Un int (32 bit)
// arriva a circa 2.1 miliardi, quindi andrebbe in overflow per N > 46340.
// Con N = 1024 int basterebbe, ma long rende il codice sicuro anche oltre.
// Il cast va fatto PRIMA della moltiplicazione: (j-1)*N calcolato tra int
// andrebbe in overflow prima di essere convertito.
// -----------------------------------------------------------------------------
long indice(int i, int j, int N) {
    return static_cast<long>(j - 1) * N + (i - 1);
}

int main(int argc, char* argv[]) {
    // ---- Lettura e controllo dell'input ------------------------------------
    // argc conta gli argomenti, compreso il nome del programma (argv[0]):
    // con "./task1 32" abbiamo argc = 2 e argv[1] = "32".
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " N\n";
        return 1;  // valore di ritorno diverso da 0 = errore
    }
    int N = std::atoi(argv[1]);  // restituisce 0 se il testo non e' un numero
    if (N < 1) {
        std::cerr << "N deve essere un intero positivo\n";
        return 1;
    }

    // Passo della griglia. Si scrive 1.0 e non 1: con 1/(N+1) la divisione
    // sarebbe tra interi e darebbe 0.
    double h = 1.0 / (N + 1);

    // ---- coords.txt --------------------------------------------------------
    // Due cicli annidati nello stesso ordine della numerazione (j esterno,
    // i interno): cosi' l'indice n esce in ordine crescente 0, 1, 2, ...
    std::ofstream coords("coords.txt");
    if (!coords) {
        std::cerr << "Errore: impossibile creare coords.txt\n";
        return 1;
    }
    // setprecision(17): un double ha circa 16 cifre significative; con 17
    // cifre il valore scritto su file si rilegge identico, senza perdite.
    // Con la precisione di default (6 cifre) perderemmo informazione.
    coords << std::setprecision(17);
    
    for (int j = 1; j <= N; ++j) {
        for (int i = 1; i <= N; ++i) {
            // x = i*h calcolato ogni volta, e non sommando h a ogni passo:
            // sommare ripetutamente accumulerebbe errori di arrotondamento.
            coords << indice(i, j, N) << ' ' << i << ' ' << j << ' '
                   << i * h << ' ' << j * h << '\n';
            // '\n' invece di std::endl: endl forza anche lo svuotamento del
            // buffer (flush) a ogni riga, molto lento con milioni di righe.
        }
    }

    // ---- connectivity.txt --------------------------------------------------
    // Per ogni nodo scriviamo SOLO l'arco verso destra e quello verso l'alto.
    // Motivo: il grafo e' non orientato, quindi l'arco tra n e il suo vicino
    // a sinistra e' gia' stato scritto quando abbiamo visitato quel vicino
    // (per lui, n era "a destra"). Lo stesso per il vicino in basso.
    // In questo modo ogni arco compare esattamente una volta, senza bisogno
    // di controllare i duplicati.
    std::ofstream conn("connectivity.txt");
    if (!conn) {
        std::cerr << "Errore: impossibile creare connectivity.txt\n";
        return 1;
    }


    long e = 0;  // indice progressivo degli archi, da 0

    for (int j = 1; j <= N; ++j) {
        for (int i = 1; i <= N; ++i) {
            long n = indice(i, j, N);
            // i < N: i nodi dell'ultima colonna non hanno un vicino a destra
            // (sarebbe un punto di bordo, che non e' un nodo del grafo).
            if (i < N) conn << e++ << ' ' << n << ' ' << indice(i + 1, j, N) << '\n';
            // j < N: i nodi dell'ultima riga non hanno un vicino sopra.
            if (j < N) conn << e++ << ' ' << n << ' ' << indice(i, j + 1, N) << '\n';
            // e++ usa il valore attuale di e e POI lo incrementa.
        }
    }

    // Stampa di controllo: gli archi devono essere 2N(N-1).
    std::cout << "Nodi: " << static_cast<long>(N) * N << ", archi: " << e
              << " (attesi " << 2L * N * (N - 1) << ")\n";

    // I file si chiudono da soli alla fine di main: gli oggetti ofstream
    // vengono distrutti e il loro distruttore chiude il file.
    return 0;
}