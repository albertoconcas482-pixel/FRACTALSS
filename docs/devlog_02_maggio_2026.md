# Devlog — 2 maggio 2026

## Contesto

Siamo sulla branch `refactor/png-buffer-pipeline`. In questa sessione l'obiettivo
primario era capire se il **periodicity checking** migliora davvero le prestazioni
di `mandel_check`, e se il refactoring del `mandel_set` ha introdotto regressioni.

---

## Cosa abbiamo fatto

### 1. Aggiunto `period_k` al main e al benchmark

`main.cpp` ora espone `static constexpr int period_k{20}` come variabile visibile.
Il benchmark è stato esteso con un asse `period_k` (valori: 10, 20, 50, 100)
per misurare quanto K influisce sui tempi.

### 2. Aggiunto `mandel_check_cardioid` nel pipeline

Nuova funzione `mandel_pipeline::mandel_check_cardioid` che usa `iterate()` nudo
(senza cycle detection) come baseline pulita. Questo permette di isolare
l'overhead del period check rispetto al solo check cardioid+bulb2.

### 3. Benchmark con due modalità: CARDIOID e PERIOD

Il `main.cpp` in `BENCHMARK_MODE 1` esegue entrambe le modalità e produce un CSV
con colonna `mode`. Viewport fisso: `x∈[-0.8,-0.7], y∈[0.05,0.15]` (stesso del
benchmark originale nel PDF).

---

## Risultati

### Il refactoring non ha introdotto regressioni ✅

Confrontando `CARDIOID` attuale con la tabella 6 del PDF (cardioid+bulb2 inline,
Release mode, stessa macchina), i delta sono tutti entro ±5% — varianza normale
da run singolo. Il codice di `mandel_set` dopo il refactoring è equivalente.

### K è quasi irrilevante in questo viewport

Variando `period_k` da 10 a 100 su 10000×10000 maxiter=800:

| period_k | mandel_ms |
|----------|-----------|
| 10       | 9666      |
| 20       | 9611      |
| 50       | 9398      |
| 100      | 9432      |

Differenza massima: ~270 ms su 9600 ms totali (~3%). K non è una leva utile
in questo viewport.

### Il period check costa ~10% di overhead fisso

Overhead `PERIOD` vs `CARDIOID` (best K, tutti i maxiter e risoluzioni):

| maxiter | overhead medio |
|---------|----------------|
| 50      | +12.2%         |
| 100     | +10.4%         |
| 200     | +10.8%         |
| 400     | +10.4%         |
| 800     | +7.8%          |

L'overhead è **costante rispetto alla risoluzione** (~10% su tutte e 4),
ma **decresce al crescere di maxiter**. Interpretazione: a maxiter alto,
qualche punto interno comincia a beneficiare del cycle detection, riducendo
parzialmente il costo. A maxiter basso il check è puro overhead.

---

## Interpretazione

Il viewport `[-0.8, -0.7]` è una zona di bordo con pochi punti interni profondi.
Il periodicity check guadagna solo dove l'orbita entra davvero in un ciclo
(zone interne lontane dal bordo). In questo viewport il check scatta raramente
e il suo costo domina il guadagno.

---

## Idee aperte (da riprendere)

### Idea 1 — Attivare il period check solo sopra una soglia di maxiter

Poiché overhead e guadagno sono entrambi legati a maxiter, ha senso attivare
`iterate_with_period` solo quando `maxiter >= soglia`. Sotto la soglia si usa
`iterate` nudo. Esempio:

```cpp
constexpr int period_threshold{300}; // da calibrare

const int iter = (maxiter >= period_threshold)
    ? mandel_set::iterate_with_period(cr, ci, maxiter, period_k)
    : mandel_set::iterate(cr, ci, maxiter);
```

Domanda aperta: dove mettere `period_threshold` — nel `main` come costante
esplicita (visibile, calibrabile) o dentro il pipeline come decisione interna?

### Idea 2 — Testare su viewport con zona interna dominante

Tutti i benchmark finora usano lo stesso viewport di bordo. Prima di giudicare
definitivamente il period check, vale la pena misurarlo su una zona con molti
punti interni (es. centro del set, cardioide grande) dove il cycle detection
non è overhead puro.

### Idea 3 — Parallelizzazione

Il bottleneck è chiaramente `mandel_check` e scala quasi linearmente con i pixel.
La parallelizzazione con `std::thread` o OpenMP è la prossima ottimizzazione
naturale, indipendente da period check.

---

## Prossimo passo concreto

Decidere tra:
1. Testare period check su viewport interno prima di scartarlo
2. Aggiungere la soglia `period_threshold` e fare un ultimo benchmark
3. Passare direttamente alla parallelizzazione
