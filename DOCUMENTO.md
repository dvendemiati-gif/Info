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
   - Il codice non memorizza il grafo scrive gli archi direttamente su file e usa memoria O(1). I vicini si ricavano dalla formula dell'indice in tempo costante,
     quindi non serve memorizzarli. Per le liste di adiacenza le costruisco leggendo connectivity.txt
 

2. **Riordinamento dei nodi** (C++, Task 2)
   Divide ricorsivamente i nodi in V1, V2 e un separatore VS,
   alternando tagli verticali e orizzontali. Produce `ordering.txt`.
   
   - **Input.** `coords.txt` prodotto dal modulo 1 (letto dalla
     cartella corrente). `connectivity.txt` non serve, perché
     l'euristica è geometrica.
   - **Output.** `ordering.txt` con N² righe `m n`: m è il nuovo
     indice, n l'indice originale del nodo.
   - **Specifiche.** Ordinamento di tipo nested dissection: si taglia
     il blocco sulla colonna (o riga) centrale, che diventa il
     separatore VS; si ordinano ricorsivamente le due parti V₁ e V₂ e
     si aggiunge VS per ultimo. I tagli si alternano tra verticali e
     orizzontali.
   - **Strutture dati.** Una tabella `griglia[j][i]` che dà l'indice
     del nodo in posizione (i, j) in O(1); un vettore `ordine` con
     ordine[m] = n.
   - **Caso base.** Un blocco con entrambi i lati lunghi meno di 3 non
     si divide; se solo il lato di turno è troppo corto, si taglia
     nell'altra direzione.
   - **Complessità.** Tempo Θ(N²): ogni nodo viene aggiunto una sola
     volta. Profondità della ricorsione circa 2·log₂N. Memoria Θ(N²).
   - **Verifica.** Il programma controlla che l'ordinamento sia una
     permutazione di 0, …, N² − 1. Con N = 3 il risultato atteso è
     0, 6, 3, 2, 8, 5, 1, 4, 7.
   - **Dipendenze.** Richiede l'output del modulo 1.

3. **Costruzione del sistema lineare** (C++, Task 3)
   Costruisce la matrice A e il termine noto secondo l'ordinamento
   scelto (naturale o riordinato). Produce `A.txt` e `rhs.txt`.
  
   - **Input.** `coords.txt` (modulo 1) e, se si sceglie il
     riordinamento, `ordering.txt` (modulo 2). L'ordinamento si sceglie
     da riga di comando: `./task3 naturale` oppure `./task3 riordinato`.
   - **Output.** `A.txt` con righe `r c A[r,c]` (solo entrate non
     nulle) e `rhs.txt` con un valore per riga; gli indici sono quelli
     del nuovo ordinamento.
   - **Specifiche.** Ogni nodo interno dà un'equazione:
     κ/h²·(somma dei 4 vicini − 4u) + f = 0. Nella riga del nodo:
     −4κ/h² sulla diagonale, +κ/h² per ogni vicino interno, termine
     noto −f(x, y). I vicini sul bordo non sono incognite: il loro
     contributo κ/h²·u₀ viene sottratto dal termine noto.
   - **Riordinamento.** Con la permutazione nuovo[n] = m, l'entrata tra
     i nodi a e b finisce in posizione (nuovo[a], nuovo[b]): righe e
     colonne vengono permutate allo stesso modo.
   - **Strutture dati.** La tabella `griglia[j][i]` include anche i
     punti di bordo, marcati con −1, per riconoscerli in O(1). La
     matrice non viene memorizzata: le entrate sono scritte su file
     man mano.
   - **Complessità.** Tempo Θ(N²), memoria Θ(N²).
   - **Verifica.** Il programma controlla che i non zeri siano
     N² + 4N(N − 1) (diagonale più ogni arco contato due volte). Con
     N = 3 abbiamo verificato che A è simmetrica e definita negativa e
     che i due ordinamenti danno la stessa soluzione, a meno della
     permutazione delle incognite.
   - **Opzionale.** La funzione `u0` è isolata nel codice: cambiandola,
     il termine noto si aggiorna in modo coerente.
   - **Dipendenze.** Richiede l'output dei moduli 1 e 2.

4. **Risoluzione e analisi** (notebook Python, Task 4)
   Legge A e rhs, fattorizza −A = L·Lᵀ, risolve il sistema, confronta
   i due ordinamenti per N = 32, 64, …, 1024 e produce i grafici.
  
   - **Input.** N e la scelta dell'ordinamento. Il notebook lancia
     `task1`, `task2` e `task3` tramite `subprocess` per generare
     `A.txt` e `rhs.txt`, così ogni esperimento parte solo da N.
   - **Output.** La soluzione u, il fattore L, i tempi di
     fattorizzazione e di soluzione, il residuo, il grafico di u.
   - **Specifiche.** Legge A in formato COO (triple) e la converte in
     CSC. Fattorizza −A = LLᵀ con CHOLMOD (`scikit-sparse`), usando
     `order="natural"` perché la libreria non applichi un suo
     riordinamento: vogliamo misurare l'effetto del nostro. Risolve
     Ly = −b e poi Lᵀu = y con `spsolve_triangular`.
   - **Scelte.** Si fattorizza −A perché A è definita negativa. I
     sistemi triangolari usano il formato CSR, perché la sostituzione
     procede per righe. I file vengono letti con `np.fromfile`, molto
     più veloce di una lettura riga per riga.
     Con `scikit-sparse` 0.5.0 la funzione `cholesky` restituisce una
     tupla (R, p) con R triangolare superiore e −A = RᵀR: la funzione
     del testo è stata quindi adattata prendendo L = Rᵀ, e si controlla
     che p sia l'identità (nessun riordinamento da parte di CHOLMOD).
     Il problema è emerso dal controllo del residuo, che risultava
     circa 1 invece di 10⁻¹⁴.
   - **Verifica.** Il residuo relativo ‖Au − b‖/‖b‖ è dell'ordine della
     precisione di macchina (circa 10⁻¹⁴). Con N = 32 i due ordinamenti
     danno la stessa soluzione, riportata all'ordine naturale.
   - **Risultato.** La temperatura è nulla sul bordo e ha il massimo
     (circa 0,73 per N = 32) vicino a (0,2; 0,2): la sorgente è più
     intensa nell'angolo, ma il bordo freddo disperde il calore.
   - **Dipendenze.** Richiede gli eseguibili dei moduli 1, 2 e 3.

5. **Confronto e analisi** (Python, Task 5) – *da fare*

   Confronto tra ordinamento naturale e riordinato per
   N = 32, 64, …, 1024: tempi, non zeri di L, grafici della soluzione
   e della struttura delle matrici con `spy`.

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
  
  Il notebook (moduli 4 e 5) esegue i programmi C++ tramite `subprocess`,
quindi ogni esperimento si riproduce partendo solo da N.

## Questioni aperte

- Come numerare i nodi in coords.txt (per righe o per colonne)? Numerati per righe
- Quando fermare la ricorsione nel riordinamento? ogni chiamata ricorsiva lavora su un blocco con strettamente meno nodi di quello di partenza
- Quale struttura usare per la matrice sparsa in C++? nessuna, ci pensa Python convertendola in CSC
