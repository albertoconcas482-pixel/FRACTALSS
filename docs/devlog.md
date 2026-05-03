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

## 1 Maggio 2026 — Sessione di esplorazione e limiti

### Relazione zoom/iterazioni

Abbiamo ragionato sulla relazione tra livello di zoom e `maxiter`. La formula empirica adottata è:

```
zoom    = 3.5 / (xmax - xmin)
maxiter = base_quality * sqrt(2 * log2(zoom))
```

con `base_quality = 200`. La formula è stata integrata direttamente nel `main.cpp`: `maxiter` non è più hardcodato ma calcolato a runtime dal viewport corrente. Il programma stampa `zoom` e `maxiter` all'avvio per trasparenza.

### Esplorazione di zone del set

Sono stati eseguiti render su zone specifiche:

- **Coda della cardioide** — centro `(-1.786, 0.0)`, ampiezza `0.008`. Zona di giunzione cardioide/bulbo-2, ricca di filamenti. `zoom ≈ 437`, `maxiter ≈ 838`.
- **Frangia superiore cardioide** — centro `(-0.85, 0.13)`, ampiezza `0.10`. `zoom ≈ 35`, `maxiter ≈ 640`.
- **Spirale doppia** — centro `(-0.7568, 0.0670)`, ampiezza `0.004`. Coordinate ricavate da un explorer esterno. `zoom ≈ 875`, `maxiter ≈ 888`. Tempo di render a `24000×18000`: **178 secondi**. Questa zona non beneficia degli early exit cardioide/bulbo perché i punti sono quasi tutti esterni al set — ogni pixel esegue il loop iterativo completo.

### Limite di memoria — std::bad_alloc

Tentando di alzare la risoluzione a `44000×33000` (≈1.45 miliardi di pixel) per un render da ~10 minuti, il programma ha lanciato `std::bad_alloc`.

**Causa:** `fractal_el` pesa circa **32 byte** per elemento (16 byte per `std::complex<double>`, 4 per `int escapeiter`, 1 per `bool inside`, 3 per `r/g/b`, più padding). Il vettore `fractal_pl::data_` alloca tutti i pixel in memoria contemporaneamente:

```
44000 × 33000 × 32 byte ≈ 44 GB
```

La macchina ha 32 GB di RAM — allocazione impossibile.

**Limite pratico attuale:** la risoluzione massima gestibile è circa `24000×18000` (432M pixel × 32 byte ≈ 3.3 GB), con margine sufficiente per il resto del sistema.

**Strade per superare il limite (roadmap):**

1. **Eliminare `c` da `fractal_el`** — la coordinata complessa è ridondante: è calcolabile deterministicamente dalla posizione `(ix, iy)` nella griglia e dai parametri del viewport. Rimuoverla risparmia 16 byte per pixel, portando il peso a ~16 byte. La risoluzione massima raddoppierebbe.
2. **Rendering a strisce** — calcolare e scrivere su file una striscia orizzontale alla volta, tenendo in memoria solo `ny/N` righe per volta. Richiede una modifica alla pipeline ma elimina il limite di RAM in modo quasi completo.
3. **Compattare `fractal_el`** — sostituire `int escapeiter` con `uint16_t` (max 65535 iterazioni, sufficiente per zoom moderati) e rimuovere `inside` (deducibile da `escapeiter == maxiter`). Si arriverebbe a ~12 byte per elemento.

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

---

## 3 Maggio 2026 — Progettazione benchmark MAIN vs CARDIOID

### Obiettivo

Confrontare le prestazioni di `mandel_set::mandel_check` (branch **main**) con
`mandel_pipeline::mandel_check_cardioid` (branch **refactor/png-buffer-pipeline**).
Le due implementazioni applicano lo **stesso algoritmo matematico** ma con
architetture diverse:

- **main** — loop di visita del piano, check cardioide/bulbo-2 e iterazione pura
  sono tutti dentro un unico metodo di `mandel_set`.
- **refactor** — il loop di visita sta in `mandel_pipeline`, che delega il calcolo
  a funzioni statiche di `mandel_set` (`is_in_cardioid`, `is_in_period2_bulb`, `iterate`).

Il benchmark verifica se la separazione architetturale introduce overhead misurabile
a parità di logica computazionale.

### Schema CSV

Entrambe le branch producono un CSV con schema identico:

```
mode, nx, ny, maxiter, period_k, plane_ms, mandel_ms, total_ms
```

- `mode` — `MAIN` (branch main) oppure `CARDIOID` (branch refactor)
- `plane_ms` — tempo di costruzione del piano complesso
- `mandel_ms` — tempo di calcolo Mandelbrot (**metrica principale**)
- `total_ms` — somma dei due
- `period_k` — sempre `0` in questo confronto (nessun cycle-detection)

### Disegno sperimentale

**Centro fisso** — ricavato esplorando mandelbrot.site e convergendo progressivamente
sulla giunzione cardioide/bulbo-2, una delle zone computazionalmente più dense:

```
cx = -1.7864402575360145
cy = -9.677023626863956e-10   (≈ 0)
```

Scegliere un punto ad alta densità di bordo garantisce che `mandel_ms` non sia
falsato da zone vuote o puramente interne, rendendo i risultati comparabili
tra un livello di zoom e l'altro.

**Intorno per ogni zoom:**

```
width  = 3.5 / zoom
height = width * (ny / nx)      ← rispetta l'aspect ratio della risoluzione
xmin   = cx - width  / 2
xmax   = cx + width  / 2
ymin   = cy - height / 2
ymax   = cy + height / 2
```

**Calibrazione maxiter** — derivata dallo zoom con la stessa formula del renderer,
nessun valore arbitrario:

```
maxiter = max(200, floor(1000 * sqrt(2 * log2(max(zoom, 2)))))
```

### Risoluzioni

Proporzioni reali di ogni formato (non quadrate):

| Nome   | nx   | ny   | Contesto                  |
|--------|------|------|---------------------------|
| FHD    | 1920 | 1080 | Full HD, schermo standard |
| 2K     | 2560 | 1440 | QHD, monitor pro/gaming   |
| 4K     | 3840 | 2160 | UHD, schermo 4K           |
| A4 300 | 2480 | 3508 | Stampa A4 qualità pro     |
| A3 300 | 3508 | 4961 | Stampa A3 qualità pro     |

### Livelli di zoom

10 livelli logaritmici, ognuno con `maxiter` calcolato dalla formula:

| # | zoom      | maxiter | width (xmax-xmin) |
|---|-----------|---------|-------------------|
| 1 | 16        | 2828    | 2.1875e-01        |
| 2 | 64        | 3464    | 5.4688e-02        |
| 3 | 256       | 4000    | 1.3672e-02        |
| 4 | 1024      | 4472    | 3.4180e-03        |
| 5 | 4096      | 4898    | 8.5449e-04        |
| 6 | 16384     | 5291    | 2.1362e-04        |
| 7 | 65536     | 5656    | 5.3406e-05        |
| 8 | 524288    | 6164    | 6.6757e-06        |
| 9 | 2097152   | 6480    | 1.6689e-06        |
|10 | 134217728 | 7348    | 2.6077e-08        |

**Dimensione totale dataset:** 2 modalità × 5 risoluzioni × 10 zoom = **100 righe**.

### Coordinate originali da mandelbrot.site

Usate per ricavare il centro stabile per convergenza progressiva:

| z viewer | re                     | im                  |
|----------|------------------------|---------------------|
| 4        | -1.30078125            | -0.0693359375       |
| 10       | -1.778961181640625     | -0.0015411376953125 |
| 16       | -1.786264419555664     | -0.0000045299530029 |
| 21       | -1.7864390760660172    | -6.482e-7           |
| 27       | -1.7864402558188885    | -1.502e-8           |
| 31       | -1.7864402575360145    | -9.677e-10          |

### Stato implementazione

- [x] `BENCHMARK_MODE` aggiunto a `src/main.cpp` della branch **main**
      (commit `006cc43` — label `MAIN`, schema CSV identico alla refactor)
- [x] `BENCHMARK_MODE` già presente in `src/main.cpp` della branch
      **refactor/png-buffer-pipeline** (label `CARDIOID`)
- [ ] Aggiornare entrambi i `main.cpp` con le 5 risoluzioni reali
      e i 10 livelli di zoom descritti sopra
- [ ] Eseguire il benchmark su entrambe le branch e raccogliere i CSV
- [ ] Scrivere lo script Python di analisi (`analysis/plot_benchmark.py`)

### Come eseguire il benchmark

```bash
# 1. Scegliere la branch, impostare BENCHMARK_MODE 1, compilare in Release
git checkout main          # oppure: refactor/png-buffer-pipeline
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# 2. Lanciare e salvare il CSV
./mandelbrot > ../benchmark_main.csv      # oppure benchmark_cardioid.csv

# 3. Ripristinare BENCHMARK_MODE 0 dopo il benchmark
```

### Analisi prevista

Lo script Python di analisi dovrà:
- Leggere i due CSV e unirli
- Normalizzare `mandel_ms` per pixel totali (`nx * ny`) → `ms_per_pixel`
- Plottare `ms_per_pixel` vs `zoom` per ogni risoluzione
- Sovrapporre `MAIN` e `CARDIOID` sullo stesso grafico per confronto diretto
