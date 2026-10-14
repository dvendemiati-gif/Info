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

- Come numerare i nodi in coords.txt (per righe o per colonne)?
- Quando fermare la ricorsione nel riordinamento?
- Quale struttura usare per la matrice sparsa in C++?
