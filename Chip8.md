# chip-8

## Memoria

Chip8 ha una memoria di 4096 byte, dalla locazione 0x000 e 0xFFF. I primi 512 byte (da 0x000 a 0x1FF) sono i byte in cui veniva originariamente caricato l'interprete, e non devono essere usati dai programmi. La maggior parte dei programmi inizia infatti all'indirizzo 0x200 (512).

## Registri

Chip8 ha 16 registri general-purpose da 8 bit ciascuno, a cui ci riferiamo con la notazione `Vx` (`x` è una cifra decimale, da 0 a F).

Esiste un registro speciale, chiamato`I`, che è un registro da 16 bit utilizzato per memorizzare indirizzi di memoria, quindi vengono solitamente usati solo i 12 bit più significativi (perché ha 2^12=4096 indirizzi disponibili)

N.B. Il registro `VF` non deve essere utilizzato da nessun programma, perché viene usato come flag da alcune istruzioni.

Chip8 ha anche due registri speciali da 8 bit per la gestione dei delay e dei suoni. Quando questi registri sono ad un valore diverso da zero, vengono automaticamente decrementati a 60Hz.

I registri "aggiuntivi" non accessibili dai programmi sono:

- `PC`: registro da 16 bit
- `SP`: registro da 8 bit che punta al livello più alto dello stack

Lo stack è un array di 16 valori da 16 bit, utilizzati per salvare gli indirizzi a cui l'interprete deve ritornare quando ha finito la subroutine corrente. Sono supportati fino a 16 livelli di subroutine nestate.

## Tastiera

Chip8 presenta una tastiera formata da 16 chiavi esadecimali, con questo layout.

``` text
1 2 3 C
4 5 6 D
7 8 9 E
A 0 B F
```

## Display

Il display implementato originalmente su Chip8 è un 64x32 picel monocromatico, in cui i pixel si contano da in alto a sinistra fino al basso a destra.

I dati vengono scritti sul display utilizzando gli sprite, che sono un gruppo di byte che sono una rappresentazione binaria del disegno desiderato. Gli sprite possono arrivare a 15 byte, per rappresentare uno sprite al massimo di 8x15.

Vediamo un esempio, rappresentiamo lo sprite del carattere "2".

| "2"  | Binary   | Hex  |
| ---- | ------   | ---  |
| **** | 11110000 | 0xF0 |
| *  * | 00010000 | 0x10 |
| **** | 00100000 | 0x20 |
|    * | 01000000 | 0x40 |
| **** | 10000000 | 0x80 |

Ogni byte rappresenta una riga dello sprite, e ogni riga è quindi formata da 8 pixel. Ogni bit specifica se il pixel è acceso o spento.

## Timer e suoni

Chip8 ha a disposizione due timer, uno per il delay e uno per i suoni.

### Delay timer

Il delay timer è attivato quando il delay time register (`DT`) ha un valore diverso da zero, e viene decrementato a una frequenza di 60Hz. Quando arriva a zero, si disattiva.

### Sound timer

Il sound timer funziona come il delay timer, ma fino a quando il registro sarà != 0 emetterà un suono.

## Ciclo di esecuzione

Come i soliti elaboratori, il ciclo che governa il funzionamento di Chip8 è formato da:

- Fetch dell'istruzione dell'indirizzo salvato in PC
- Decode dell'istruzione per capire cosa l'emulatore deve fare
- Execute l'istruzione

N.B. Un processore farebbe andare troppo velocemente l'emulatore per poter giocare; è necessario settare una frequenza più bassa (~1MHz).

### Fetch

La prima fase consiste nel leggere dall'indirizzo puntato da PC i successivi due byte, dato che un'istruzione è formata da 16 bit. Subito dopo bisogna incrementare PC di due.

### Decode

Le istruzioni di Chip8 vengono classificate con il primo mezzo byte, cioè con il primo numero esadecimale dell'istruzione.

### Execute

Semplicemente esegui l'istruzione, easy.

## Istruzioni

Le istruzioni sono lunghe 2 byte, e sono memorizzate partendo dal byte più significativo (big endian). Il primo byte di ogni istruzione dovrebbe essere quindi collocato in un indirizzo pari.

### Elenco delle istruzioni

#### Istruzioni di controllo

- `0nnn` - SYS addr: jump alla routine macchina che si trova all'indirizzo `nnn`.
- `00EE` - RET: ritorna da una subroutine; l'interprete setta il program counter all'indirizzo salvato in cima allo stack, e poi decrementa lo stack pointer
- `1nnn` - JP addr: jump all'indirizzo `nnn`; l'interprete sovrascrive `PC` con il valore specificato
- `2nnn` - CALL addr: chiama la subroutine all'indirizzo `nnn`; l'interprete incrementa lo stack pointer, poi scrive il PC in cima allo stack e sovrascrive il registro con `nnn`
- `3xkk` - SE Vx, byte: skippa l'istruzione successiva se `Vx=kk`; l'interprete compara il registro a `Vx` to `kk` e, se uguali, incrementa il program counter di 2
- `4xkk` - SNE Vx, byte: skippa l'istruzione successiva se `Vx!=kk`; l'interprete compara il registro `Vx` con `kk` e, se non sono uguali, incrementa il program counter di 2
- `5xy0` - SE Vx, Vy: skippa l'istruzione successiva se `Vx` e `Vy` sono uguali

#### Operazioni con registri

- `6xkk` - LD Vx, byte: setta il valore `kk` dentro `Vx`
- `7xkk` - ADD Vx, byte: setta `Vx=Vx+kk`
- `8xy0` - LD Vx, Vy: salva il valore di `Vy` dentro `Vx`
- `8xy1` - OR Vx, Vy: fa l'operazione di OR tra i valori di Vx e Vy, salvando il risultato in Vx
- `8xy2` - AND Vx, Vy: fa l'operazione di AND tra i valori di Vx e Vy e salva il risultato in Vx
- `8xy3` - XOR Vx, Vy
- `8xy4` - ADD Vx, Vy: i valori di Vx e Vy sono sommati e, se il risultato generato è più grande di 8 bit, VF viene settato a `1`, altrimenti `0`, salvando solamente gli 8 bit meno significativi in `Vx`
- `8xy5` - Sub Vx, Vy: se Vx > Vy, `VF` viene settato a `1`, altrimenti `0`, poi sottrae `Vy` a `Vx`, salvando il risultato in `Vx`
- `8xy6` - SHR Vx {, Vy}: fa uno shift a destra
- `8xy7` - SUBN Vx, Vy: come SUB, ma fa l'operazione `Vy-Vx`
- `8xyE` - SHL Vx {, Vy}: fa uno shift a sinistra
- `9xy0` - SNE Vx, Vy: skippa l'istruzione successiva se `Vx!=Vy`
- `Annn` - LD I, addr: salva all'interno di `I` il valore `nnn`
- `Bnnn` - JP V0, addr: salta all'indirizzo `nnn+V0`
- `Cxkk`: salva in `Vx` un byte random AND `kk`

#### Istruzioni grafiche

- `00E0` - CLS: pulisce lo schermo
- `Dxyn`: DRW Vx, Vy, nibble: mostra a schermo uno sprite di dimensione `n` che inizia all'indirizzo di memoria I posizionandolo a `(Vx, Vy)`, e settando `VF=collision`; l'interprete legge gli `n` byte dalla memoria, iniziando dall'indirizzo contenuto nel registro I. Questi byte vengono mostrati a schermo sulle coordinate `(Vx,Vy)`; viene fatto uno XOR tra gli sprite e i byte presenti sullo schermo: se questo causa la cancellazione di almeno un pixel, `VF` viene settato a `1`, altrimenti `0`. Se lo sprite esce in qualche modo dallo schermo, deve comparire dal lato opposto dello schermo
- `Fx29` - LD F, Vx: setta nel registro I il valore dell'indirizzo dello sprite corrispondente alla cifra contenuta in `Vx`

#### Istruzioni IO

- `Ex9E` - SKP Vx: skippa la prossima istruzione se viene premuta il tasto la cui chiave corrisponde al valore di `Vx`; l'emulatore controlla se il tasto corrispondente a `Vx` è in stato di down, e se sì allora PC viene incrementato di 2
- `ExA1` - SKNP Vx: skippa la prossima istruzione se non è premuto il tasto `Vx`
- `Fx07` - LD Vx, DT: mette in `Vx` il valore contenuto in `DT` (delay timer)
- `Fx0A` - LD Vx, K: aspetta che un tasto venga premuto, e poi salva il valore della chiave premuta in Vx
- `Fx15` - LD DT, Vx: setta il delay timer `DT` al valore contenuto in `Vx`
- `Fx18` - LD ST, Vx: setta il valore del sound timer `ST` al valore di `Vx`

#### Istruzioni con registro I

- `Fx1E` - ADD I, Vx: setta il valore di I a `I+Vx`
- `Fx33` - LD B, Vx: salva la rappresentazione BCD del valore contenuto in `Vx` negli indirizzi di memoria I, I+1 e I+2
- `Fx55` - LD [I], Vx: salva i valori nei registri da `V0` a `Vx` negli indirizzi di memoria partendo dall'indirizzo contenuto in I
- `Fx65`: LD Vx, [I]: salva nei registri da `V0` a `Vx` i valori contenuti dall'indirizzo contenuto in I
