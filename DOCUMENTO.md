# Equazione del calore 2D e riordinamento nelle fattorizzazioni di Cholesky

Documento di progetto – versione finale

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
     punti vicini in orizzontale o verticale. I punti di bordo non
     sono nodi: lì la temperatura è nota e non è un'incognita.
   - **Scelte.** Numerazione per righe, n = (j − 1)·N + (i − 1): l'indice
     di un punto si calcola in tempo costante, senza dizionario. Ogni
     arco viene scritto una sola volta, partendo dal nodo a sinistra
     o in basso (si generano solo l'arco verso destra e quello verso
     l'alto).
   - **Grafo non memorizzato.** Il codice non memorizza il grafo: scrive
     gli archi direttamente su file. I vicini si ricavano dalla formula
     dell'indice in tempo costante, quindi non serve memorizzarli; se
     servissero le liste di adiacenza, si potrebbero costruire leggendo
     `connectivity.txt`.
   - **Complessità.** Tempo Θ(N²), lineare nel numero di nodi n = N² e
     nella dimensione dell'output. Memoria aggiuntiva O(1).
   - **Invariante.** Gli indici n in `coords.txt` compaiono in ordine
     crescente da 0 a N² − 1, quindi la riga k del file descrive il
     nodo k.
   - **Dipendenze.** Nessuna; è il primo modulo della catena.
   - **Verifica.** Il programma controlla l'argomento e l'apertura dei
     file, e stampa il numero di archi generati e quello atteso
     2N(N − 1). Con N = 3 i file sono stati controllati a mano: 9 nodi,
     12 archi, coordinate multipli di h = 0,25.

2. **Riordinamento dei nodi** (C++, Task 2)
   Divide ricorsivamente i nodi in V1, V2 e un separatore VS,
   alternando tagli verticali e orizzontali. Produce `ordering.txt`.

   - **Input.** `coords.txt` e `connectivity.txt` prodotti dal modulo 1
     (letti dalla cartella corrente). Il riordinamento è geometrico e
     usa `coords.txt`; `connectivity.txt` serve a verificare la
     proprietà del separatore.
   - **Output.** `ordering.txt` con N² righe `m n`: m è il nuovo
     indice, n l'indice originale del nodo.
   - **Specifiche.** Ordinamento di tipo nested dissection: si taglia
     il blocco sulla colonna (o riga) centrale, che diventa il
     separatore VS; si ordinano ricorsivamente le due parti V₁ e V₂ e
     si aggiunge VS per ultimo. I tagli si alternano tra verticali e
     orizzontali, come chiede il testo. Il taglio funziona perché un
     arco collega solo nodi della stessa colonna o di colonne
     adiacenti: nessun arco collega direttamente V₁ e V₂.
   - **Perché il separatore per ultimo.** Il blocco di A tra V₁ e V₂ è
     nullo; con il separatore in fondo resta nullo anche nel fattore L
     (L₂₁ = A₂₁·L₁₁⁻ᵀ = 0), e il riempimento resta confinato nei
     separatori.
   - **Strutture dati.** Una tabella `griglia[j][i]` che dà l'indice
     del nodo in posizione (i, j) in O(1); un vettore `ordine` con
     ordine[m] = n; per la verifica, un vettore `colonna[n]` che dà la
     colonna di un nodo in O(1). I confronti usano gli indici interi
     i, j e non le coordinate double, per evitare errori di
     arrotondamento.
   - **Caso base.** Un blocco con entrambi i lati lunghi meno di 3 (al
     più 2 × 2) non si divide; se solo il lato di turno è troppo corto,
     si taglia nell'altra direzione. Siccome si taglia solo un lato di
     almeno 3 nodi, non si creano mai blocchi vuoti.
   - **Terminazione e correttezza.** Ogni chiamata lavora su un blocco
     strettamente più piccolo, perché il separatore toglie almeno una
     riga o una colonna. Per induzione, V₁, V₂ e VS sono disgiunti e
     coprono il blocco, quindi ogni nodo compare una sola volta.
   - **Complessità.** Tempo Θ(N²): ogni nodo viene aggiunto una sola
     volta. Le chiamate formano un albero binario bilanciato, profondo
     circa 2·log₂N (circa 20 per N = 1024); l'ordine finale è una sua
     visita posticipata. Memoria Θ(N²).
   - **Verifica.** Il programma controlla che l'ordinamento sia una
     permutazione di 0, …, N² − 1; che nessun arco di
     `connectivity.txt` colleghi un nodo a sinistra del primo
     separatore con uno a destra; che gli archi siano 2N(N − 1), cioè
     che i due file siano coerenti. Con N = 3 il risultato è
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
     del nuovo ordinamento, e la riga r di `rhs.txt` corrisponde alle
     entrate di `A.txt` con primo indice r.
   - **Specifiche.** Ogni nodo interno dà un'equazione:
     κ/h²·(somma dei 4 vicini − 4u) + f = 0. Nella riga del nodo:
     −4κ/h² sulla diagonale, +κ/h² per ogni vicino interno, termine
     noto −f(x, y). I vicini sul bordo non sono incognite: il loro
     contributo κ/h²·u₀ viene sottratto dal termine noto (un nodo
     d'angolo ha due vicini di bordo, uno di lato uno solo).
   - **Riordinamento.** Con la permutazione nuovo[n] = m, l'entrata tra
     i nodi a e b finisce in posizione (nuovo[a], nuovo[b]): righe e
     colonne vengono permutate allo stesso modo (P·A·Pᵀ), quindi la
     matrice resta simmetrica.
   - **Strutture dati.** La tabella `griglia[j][i]` include anche i
     punti di bordo, marcati con −1, per riconoscerli in O(1). I vettori
     `ordine[m] = n` e `nuovo[n] = m` sono uno l'inverso dell'altro. La
     matrice non viene memorizzata: le entrate sono scritte su file
     man mano.
   - **Complessità.** Tempo Θ(N²), memoria Θ(N²).
   - **Verifica.** Il programma controlla che `ordering.txt` sia una
     vera permutazione in entrambe le direzioni: nessun nodo n e nessun
     nuovo indice m ripetuto, e N² righe. Il controllo su m è stato
     aggiunto dopo aver provato un file con un m ripetuto, che prima
     veniva accettato e produceva una matrice sbagliata. Il programma
     controlla anche che i non zeri siano N² + 4N(N − 1) (diagonale più
     ogni arco contato due volte). Risolvendo i sistemi con N = 3, 16 e
     40 abbiamo verificato che A è simmetrica e che i due ordinamenti
     danno la stessa soluzione, a meno della permutazione delle
     incognite.
   - **Opzionale.** La funzione `u0` è isolata nel codice: cambiandola,
     il termine noto si aggiorna in modo coerente. Verificato con
     u₀ = x + 2y: le differenze finite sono esatte sulle funzioni
     lineari, quindi la differenza tra la soluzione con questo u₀ e
     quella con u₀ = 0 vale x + 2y nei nodi interni (errore circa 10⁻¹⁴).
   - **Dipendenze.** Richiede l'output dei moduli 1 e 2.

4. **Risoluzione** (notebook Python, Task 4) – *implementato in `task4.ipynb`*
   Legge A e rhs, fattorizza −A = L·Lᵀ, risolve il sistema e disegna
   la soluzione per N = 32 con i due ordinamenti.

   - **Input.** N e la scelta dell'ordinamento. Il notebook lancia
     `task1`, `task2` e `task3` tramite `subprocess` per generare
     `A.txt` e `rhs.txt`, così ogni esperimento parte solo da N. Se un
     programma C++ fallisce, il notebook si ferma mostrando il suo
     messaggio di errore.
   - **Output.** La soluzione u, il fattore L, i tempi di
     fattorizzazione e di soluzione, il residuo, il grafico di u.
   - **Specifiche.** Legge A in formato COO (triple) e la converte in
     CSC. Fattorizza −A = LLᵀ con CHOLMOD (`scikit-sparse`), usando
     `order="natural"` perché la libreria non applichi un suo
     riordinamento: vogliamo misurare l'effetto del nostro. Risolve
     Ly = −b e poi Lᵀu = y con `spsolve_triangular`.
   - **Scelte.** Si fattorizza −A perché A è definita negativa. I
     sistemi triangolari usano il formato CSR, perché la sostituzione
     procede per righe: la copia CSR di L viene cancellata appena
     risolto il primo sistema, e per il secondo si usa L.T, che è già
     CSR senza copie. I file vengono letti con `np.fromfile`, molto
     più veloce di una lettura riga per riga.
   - **Adattamento della funzione del testo.** Con `scikit-sparse` 0.5.0
     la funzione `cholesky` restituisce una tupla (R, p) con R
     triangolare superiore e −A = RᵀR: la funzione del testo è stata
     quindi adattata prendendo L = Rᵀ, e si controlla che p sia
     l'identità (nessun riordinamento da parte di CHOLMOD). Il problema
     è emerso dal controllo del residuo, che risultava circa 1 invece
     di 10⁻¹⁴. Una cella del notebook lo dimostra con N = 8:
     ‖RᵀR + A‖ ≈ 9·10⁻¹⁶, mentre ‖RRᵀ + A‖ ≈ 0,56.
   - **Verifica.** Con N = 32 il residuo relativo ‖Au − b‖/‖b‖ è circa
     10⁻¹⁴ e i due ordinamenti danno la stessa soluzione, riportata
     all'ordine naturale (differenza circa 7·10⁻¹⁵).
   - **Risultato.** La temperatura è nulla sul bordo e ha il massimo
     (circa 0,73 per N = 32) vicino a (0,2; 0,2): la sorgente è più
     intensa nell'angolo, ma il bordo freddo disperde il calore.
   - **Dipendenze.** Richiede gli eseguibili dei moduli 1, 2 e 3.

5. **Confronto e analisi** (notebook Python, Task 5) – *implementato in `task5.ipynb`*

   - **Input.** I valori N = 32, 64, …, 1024 e i due ordinamenti. Il
     notebook genera i file con i programmi C++, come nel modulo 4, e
     ripete le stesse funzioni perché si possa eseguire da solo.
   - **Output.** Tabella e grafici log-log di nnz(L) e dei tempi di
     fattorizzazione e soluzione; immagini `spy` di A e L per N = 32
     con entrambi gli ordinamenti; grafici della soluzione per vari N.
     I risultati numerici sono salvati in `risultati_task5.json`, le
     soluzioni in `soluzioni_task5.npz` (ignorato da git perché
     pesante e rigenerabile).
   - **Organizzazione.** Prima tutti i casi con il nested dissection,
     poi quelli naturali dal più piccolo al più grande: il caso più
     pesante è l'ultimo. I risultati si salvano dopo ogni esperimento,
     e una cella di ripresa li ricarica se il kernel si riavvia. Per
     N ≤ 128 la misura si ripete 3 volte e si tiene il tempo minimo,
     perché i tempi di pochi millisecondi sono rumorosi.
   - **Attese teoriche.** Con l'ordinamento naturale A è a banda di
     semiampiezza N e Cholesky riempie la banda: nnz(L) ≈ N³, costo
     della fattorizzazione ≈ N⁴. Con il nested dissection:
     nnz(L) = O(N² log N), costo O(N³) (George, 1973).
   - **Limite di memoria.** Con l'ordinamento naturale e N = 512 il
     kernel è stato terminato per memoria esaurita (circa 1,3·10⁸ non
     zeri, con più copie in memoria su 8 GB di RAM). Il naturale è
     quindi calcolato fino a N = 256 (`N_MAX_NATURALE`), il nested
     dissection fino a N = 1024.
   - **Risultati.** I valori completi, compresi i tempi, sono in
     `risultati_task5.json` e nel notebook. I non zeri del fattore L
     (deterministici, uguali a ogni esecuzione) sono:

     | N | nnz(L) naturale | nnz(L) nested dissection | Rapporto |
     |---|---|---|---|
     | 32 | 32 799 | 16 033 | 2,0 |
     | 64 | 262 207 | 88 683 | 3,0 |
     | 128 | 2 097 279 | 463 733 | 4,5 |
     | 256 | 16 777 471 | 2 319 487 | 7,2 |
     | 512 | – (memoria) | 11 207 817 | – |
     | 1024 | – (memoria) | 52 723 859 | – |

     Con l'ordinamento naturale nnz(L) è praticamente N³ (pendenza
     log-log 3,00). Con il nested dissection la pendenza tra due
     raddoppi di N scende da 2,47 a 2,23, come previsto per N² log N.
     Nei tempi di fattorizzazione la pendenza del nested dissection è
     circa 3; quella del naturale varia tra le esecuzioni (da 3,5 a
     quasi 4), perché i tempi dipendono dal carico della macchina. Il
     residuo cresce con N circa come N², da 1,8·10⁻¹⁴ (N = 32) a
     1,7·10⁻¹¹ (N = 1024): dipende dal condizionamento di A, che
     cresce come 1/h², non dall'ordinamento.
   - **Dipendenze.** Richiede gli eseguibili dei moduli 1, 2 e 3.

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

```
N → [Modulo 1] → coords.txt, connectivity.txt
  → [Modulo 2] → ordering.txt
  → [Modulo 3] → A.txt, rhs.txt
  → [Modulo 4] → soluzione, tempi, grafici (N = 32)
  → [Modulo 5] → confronto per N = 32, …, 1024, risultati_task5.json
```

I notebook (moduli 4 e 5) eseguono i programmi C++ tramite
`subprocess`, quindi ogni esperimento si riproduce partendo solo da N.

## Questioni aperte (risolte)

- **Come numerare i nodi in coords.txt?** Per righe,
  n = (j − 1)·N + (i − 1): l'indice si calcola in tempo costante.
- **Quando fermare la ricorsione nel riordinamento?** Quando entrambi
  i lati del blocco hanno meno di 3 nodi (al più 2 × 2): per tagliare
  servono tre nodi, uno per parte e uno nel separatore. La ricorsione
  termina perché ogni chiamata lavora su un blocco con strettamente
  meno nodi di quello di partenza.
- **Quale struttura usare per la matrice sparsa in C++?** Nessuna: il
  C++ scrive le sole entrate non nulle su file in formato a triplette,
  e Python le legge e le converte in CSC.

## Conclusioni

Il riordinamento di tipo nested dissection non cambia la soluzione,
ma cambia profondamente il costo: il fattore L passa da una crescita
circa cubica a una quasi quadratica in N. In pratica questo è ciò che
rende risolvibile il problema per N = 1024, che con l'ordinamento
naturale esaurisce la memoria già a N = 512. La precisione non
dipende dall'ordinamento: il residuo cresce con N per effetto del
condizionamento della matrice, allo stesso modo con entrambi gli
ordinamenti.
