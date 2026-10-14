# Equazione del calore 2D e riordinamento nelle fattorizzazioni di Cholesky

Documento di progetto – versione 0.1 (schema iniziale)

## Obiettivo

Calcolare la temperatura di equilibrio su una piastra quadrata [0,1]²,
discretizzando il dominio con una griglia di N×N punti interni e
risolvendo il sistema lineare che ne deriva tramite fattorizzazione
di Cholesky. Vogliamo confrontare l'ordinamento "naturale" delle
incognite con un riordinamento di tipo nested dissection, misurando
tempi e numero di elementi non nulli del fattore L.

## Moduli principali

1. **Generazione della griglia e del grafo** (C++, Task 1)
   Dato N, crea i punti interni del dominio e gli archi tra punti
   vicini. Produce `coords.txt` e `connectivity.txt`.
   1. **Generazione della griglia e del grafo** (C++, Task 1) – *implementato in `task1.cpp`*

   - **Input.** Intero N ≥ 1, passato da riga di comando (`./task1 N`).
   - **Output.** `coords.txt` con N² righe `n i j x y`;
     `connectivity.txt` con 2N(N − 1) righe `e n1 n2`.
   - **Specifiche.** Genera i punti interni (i, j = 1, …, N) con
     x = i·h, y = j·h, h = 1/(N + 1), e un arco per ogni coppia di
     punti vicini in orizzontale o verticale.
   - **Scelte.** Numerazione per righe, n = (j − 1)·N + (i − 1). Ogni
     arco viene scritto una sola volta, partendo dal nodo a sinistra
     o in basso.
   - **Complessità.** Tempo Θ(N²), lineare nella dimensione
     dell'output. Memoria aggiuntiva O(1): i dati vengono scritti su
     file man mano, senza essere memorizzati.
   - **Invariante.** Gli indici n in `coords.txt` compaiono in ordine
     crescente da 0 a N² − 1, quindi la riga k del file descrive il
     nodo k.
   - **Dipendenze.** Nessuna; è il primo modulo della catena.
   - **Verifica.** Il programma stampa il numero di archi generati e
     quello atteso 2N(N − 1).

 

2. **Riordinamento dei nodi** (C++, Task 2)
   Divide ricorsivamente i nodi in V1, V2 e un separatore VS,
   alternando tagli verticali e orizzontali. Produce `ordering.txt`.

3. **Costruzione del sistema lineare** (C++, Task 3)
   Costruisce la matrice A e il termine noto secondo l'ordinamento
   scelto (naturale o riordinato). Produce `A.txt` e `rhs.txt`.

4. **Risoluzione e analisi** (notebook Python, Task 4 e 5)
   Legge A e rhs, fattorizza −A = L·Lᵀ, risolve il sistema, confronta
   i due ordinamenti per N = 32, 64, …, 1024 e produce i grafici.

## Strutture dati fondamentali (idea iniziale)

- Un nodo: indice n, indici di griglia (i, j), coordinate (x, y).
- Numerazione dei nodi per righe: n = (j − 1)·N + (i − 1), con
  i, j = 1, …, N. I vicini di n sono n ± 1 (orizzontali) e n ± N
  (verticali), quindi gli archi si generano in tempo O(1) ciascuno.
- Numero di nodi: N². Numero di archi: 2N(N − 1).
- La lista dei nodi e la lista degli archi (coppie di indici n1, n2).
- Un ordinamento: corrispondenza tra nuovo indice m e indice originale n.
- La matrice A: memorizzata in forma sparsa, solo le entrate non nulle.

## Interazioni tra moduli

I moduli comunicano tramite file di testo:

N → [Modulo 1] → coords.txt, connectivity.txt
  → [Modulo 2] → ordering.txt
  → [Modulo 3] → A.txt, rhs.txt
  → [Modulo 4] → soluzione, tempi, grafici

## Questioni aperte

- Come numerare i nodi in coords.txt (per righe o per colonne)? Numerati per righe
- Quando fermare la ricorsione nel riordinamento?
- Quale struttura usare per la matrice sparsa in C++?
