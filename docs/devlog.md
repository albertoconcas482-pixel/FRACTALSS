# FRACTALSS — Development Log

Diario di sviluppo del progetto. Documenta le decisioni prese, le implementazioni completate e le idee in sospeso, in ordine cronologico.

---

## 1 Maggio 2026 — Sessione iniziale

### Struttura del progetto

Il progetto è partito con una struttura già ben definita, divisa in moduli indipendenti con responsabilità separate:

- `fractal_el` — struttura dati del singolo punto nel piano complesso
- `fractal_pl` — griglia del piano complesso, storage row-major flat
- `mandel_set` — kernel di calcolo Mandelbrot
- `render` — colorazione e scrittura su file
- `colors` — registry dei schemi colore

La pipeline è: `fractal_pl → mandel_set → render`.

### Ottimizzazioni implementate nel kernel

Il kernel `mandel_set::mandel_check` implementa due ottimizzazioni principali:

**Early exit per cardioide e bulbo di periodo 2** — i punti che ricadono analiticamente dentro la cardioide principale o il bulbo di periodo 2 vengono saltati prima di entrare nel loop iterativo. Questi sono la maggior parte dei punti interni e la loro classificazione è O(1).

**Cache dei quadrati** — `zr²` e `zi²` vengono calcolati una volta per iterazione e riusati sia nel test di escape (`zr² + zi² > 4`) sia nel calcolo del passo successivo, evitando moltiplicazioni ridondanti.

**Build in Release mode** — benchmark mostrano uno speedup di ~2.65x rispetto a Debug. È il primo parametro da controllare prima di misurare qualsiasi altra cosa.

### Sistema di colorazione

Il sistema di colorazione usa un registry statico (`color_registry`) basato su una `std::unordered_map<string, function>`. Ogni schema colore (classe `bw`, classe `crazy`) si registra automaticamente al boot del programma tramite un membro statico `registered_` inizializzato prima di `main()`. Questo permette di aggiungere nuovi schemi senza modificare codice esistente.

Schemi implementati:
- `bw` — bianco e nero puro, dentro/fuori
- `crazy` — RGB calcolato da `escapeiter` con moltiplicatori e offset diversi per canale, produce un effetto ciclico colorato

### Formato output: passaggio da P3 a P6

Il formato di output PPM è stato aggiornato da **P3 (ASCII)** a **P6 (binario)**.

In P3 ogni pixel veniva scritto come tre interi ASCII separati da spazi (~12 byte per pixel). In P6 i byte RGB vengono scritti direttamente come dati binari (3 byte per pixel esatti). Il risultato è un file ~4x più leggero a parità di risoluzione e qualità identica.

La modifica ha richiesto due cambiamenti in `render.cpp`:
- `std::ios::binary` nell'apertura del file
- `out.write(reinterpret_cast<const char*>(rgb), 3)` invece di `<<` con cast a `int`

### Documentazione

Tutti i file sorgente sono stati documentati con commenti che spiegano le decisioni non ovvie: il layout row-major della griglia, il pattern di auto-registrazione dei colorizer, le formule matematiche del kernel, le ottimizzazioni di early exit.

Il README copre: pipeline, struttura, note architetturali, istruzioni di build, formato output, roadmap ottimizzazioni, roadmap colorazione, riferimento ai benchmark.

---

## Roadmap — Prossimi step

### Ottimizzazioni kernel

- **Periodicity checking** — rilevamento ciclico per i punti interni che non vengono catturati dall'early exit analitico. Se `z` torna a un valore già visto, il punto è interno e si può uscire subito.
- **Sfruttamento della simmetria** — il Mandelbrot è simmetrico rispetto all'asse reale. Quando il viewport lo permette, si può calcolare solo la metà superiore e specchiare.
- **Parallelizzazione** — partizionamento per range di indici su `fractal_pl::data`, naturalmente parallelizzabile perché ogni punto è indipendente.

### Colorazione

- **Smooth coloring** — salvare il valore finale di `z` all'escape in `fractal_el` permette di calcolare `smooth = escapeiter - log2(log2(|z|))`, un indice continuo invece che intero. Produce gradienti fluidi invece di bande nette. Richiede interpolazione in float dentro il colorizer, con conversione a `unsigned char` solo alla fine.

### Output

- **PNG nativo** — integrare `stb_image_write.h` (header-only, zero dipendenze esterne) per scrivere direttamente in PNG. Una griglia 12000×9000 pesa ~10–30 MB in PNG contro ~300 MB in P6. Il PPM rimane come metodo di debug.

### Note progettuali

**Relazione risoluzione/iterazioni** — i due parametri sono indipendenti ma si moltiplicano nel costo. La regola: `maxiter` dovrebbe crescere con il livello di zoom, non linearmente. Per viewport a zoom moderato, 200–400 iterazioni sono già più che sufficienti. Aumentare `maxiter` oltre il necessario non aggiunge dettaglio visibile ma moltiplica il tempo di calcolo.

**Colori in `unsigned char`** — per colorazioni basate su lookup diretto di `escapeiter`, `unsigned char` è sufficiente e corretto. Il discorso dei float diventa rilevante solo con smooth coloring, dove l'indice nella palette è continuo e richiede interpolazione.
