# C Speedtest CLI

Apskaičiuokite ir įvertinkite tinklo greitį tiesiai iš komandinės eilutės! Ši programa yra lengvas, **C kalba** parašytas įrankis, kuris naudoja `libcurl` tinklo užklausoms valdyti bei `cJSON` duomenų apdorojimui.

## 🛠️ Funkcijos

* **Automatinė geolokacija**: Automatiškai nustato esamą tinklo vietovę.
* **Optimaliausio serverio paieška**: Randa geriausią matavimo serverį pagal esamą arba rankiniu būdu nurodytą vietovę.
* **Atsiuntimo (Download) sparta**: Galimybė matuoti atsisiuntimo greitį naudojant standartinius arba pasirinktus testinius failus.
* **Išsiuntimo (Upload) sparta**: Galimybė matuoti išsiuntimo greitį naudojant `Ookla` arba kitus HTTP/HTTPS serverius.
* **Pilnas automatinis testas**: Galimybė atlikti visą matavimo ciklą (serverio paiešką, išsiuntimą ir atsiuntimą) vienu komandos paleidimu.

## 🧰 Reikalavimai ir bibliotekos

Norėdami sukompiliuoti ir paleisti šią programą, įsitikinkite, kad jūsų sistemoje yra įdiegtos šios bibliotekos:

* **`libcurl`**: Naudojama HTTP/HTTPS duomenų siuntimui ir gavimui.
* **`cJSON`**: Naudojama JSON formato atsakymų (pvz., serverių sąrašų) apdorojimui.
* **C standartinė biblioteka / POSIX**: Reikalinga `unistd.h` komandinės eilutės argumentų apdorojimui (`getopt`).

## 🔨 Projekto kompiliavimas

Sukompiliuokite projektą naudodami Makefile:

```bash
make
```

## 🚀 Naudojimas

```bash
./build/main [PARINKTYS]
```

### Galimos parinktys

| Raktas | Argumentas | Aprašymas |
| :--- | :--- | :--- |
| `-a` | *Nėra* | **Automatizuotas testas**: Automatiškai parenka geriausią serverį, atlieka išsiuntimo ir atsiuntimo matavimus. |
| `-d` | *Nėra* | Atlieka standartinį atsiuntimo (download) testą. |
| `-D` | `<url>` | Atlieka atsiuntimo testą naudojant nurodytą failo URL(url pateikti taip kaip json faile pvz.: speed-kaunas.telia.lt:8080). |
| `-u` | *Nėra* | Atlieka standartinį išsiuntimo (upload) testą. |
| `-U` | `<url>` | Atlieka išsiuntimo testą į nurodytą serverio URL(url pateikti taip kaip json faile pvz.: speed-kaunas.telia.lt:8080). |
| `-l` | *Nėra* | Randa ir išveda geriausią serverį pagal automatiškai nustatytą vietovę. |
| `-L` | `<vietovė>` | Randa ir išveda geriausią serverį nurodytai vietovei (pvz., `"Lithuania"`). |

## 💡 Pavyzdžiai

#### 1. Atlikti pilną automatinį greičio testą:

```bash
./speedtest -a
```

#### 2. Išbandyti atsiuntimo greitį iš konkretaus URL:

```bash
./speedtest -D speed-kaunas.telia.lt:8080
```

#### 3. Rasti geriausią serverį konkrečiai vietovei:

```bash
./speedtest -L Lithuania
```

#### 4. Atlikti tik išsiuntimo ir atsiuntimo testus:

```bash
./speedtest -u -d
```
