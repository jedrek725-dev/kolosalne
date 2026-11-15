/**
 * Program: Kalkulator Liczb Kolosalnych
 *
 * Program realizuje operacje na "liczbach kolosalnych" zdefiniowanych
 * rekurencyjnie. Liczba kolosalna to lista cyfr, które same są liczbami
 * kolosalnymi. Wartość liczby to suma potęg dwójki o wykładnikach
 * będących wartościami jej cyfr.
 *
 * Program działa jako kalkulator stosowy obsługujący operacje:
 * - wczytywania i wypisywania liczb (reprezentacja tekstowa),
 * - duplikacji (:),
 * - potęgowania (^),
 * - dodawania (+),
 * - mnożenia (*).
 *
 * Zarządzanie pamięcią odbywa się poprzez licznik referencji, co pozwala
 * na efektywne współdzielenie struktury liczb (niezmienność danych).
 *
 * autor: Jędrzej Kamiński
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Początkowy rozmiar stosu kalkulatora.
 */
#define STOS_POJEMNOSC_POCZATKOWA 128

/**
 * Struktura reprezentująca liczbę kolosalną.
 * Jest to lista jednokierunkowa wykładników potęg dwójki.
 *
 * wykladnik: Wskaźnik na liczbę będącą wykładnikiem (dla składnika 
2^wykladnik).
 * NULL oznacza 0.
 * reszta:    Wskaźnik na kolejny składnik sumy (o mniejszym wykładniku).
 * licznik_ref: Licznik referencji do zarządzania pamięcią.
 */
typedef struct Kolosalna {
    struct Kolosalna *wykladnik;
    struct Kolosalna *reszta;
    unsigned int licznik_ref;
} Kolosalna;

/**
 * Struktura stosu kalkulatora.
 */
typedef struct {
    Kolosalna **dane;
    size_t rozmiar;
    size_t pojemnosc;
} Stos;

/**
 * Zwiększa licznik referencji obiektu.
 */
void zwieksz_ref(Kolosalna *k) {
    if (k) {
        k->licznik_ref++;
    }
}

/**
 * Zmniejsza licznik referencji. Gdy spadnie do 0, zwalnia pamięć.
 */
void zmniejsz_ref(Kolosalna *k) {
    if (k) {
        k->licznik_ref--;
        if (k->licznik_ref == 0) {
            zmniejsz_ref(k->wykladnik);
            zmniejsz_ref(k->reszta);
            free(k);
        }
    }
}

/**
 * Tworzy nowy węzeł listy liczb kolosalnych.
 * Przejmuje odpowiedzialność za referencje do `wykladnik` i `reszta`.
 */
Kolosalna *stworz_wezel(Kolosalna *wykladnik, Kolosalna *reszta) {
    Kolosalna *k = malloc(sizeof(Kolosalna));
    if (!k) {
        fprintf(stderr, "Błąd alokacji pamięci\n");
        exit(1);
    }
    k->wykladnik = wykladnik;
    k->reszta = reszta;
    k->licznik_ref = 1;

    if (wykladnik) zwieksz_ref(wykladnik);
    if (reszta) zwieksz_ref(reszta);

    return k;
}

/**
 * Porównuje dwie liczby kolosalne.
 * Zwraca: 1 gdy a > b, -1 gdy a < b, 0 gdy a == b.
 */
int porownaj(Kolosalna *a, Kolosalna *b) {
    if (a == b) return 0;
    if (a == NULL) return -1;
    if (b == NULL) return 1;

    /* Porównaj wykładniki najbardziej znaczących składników */
    int wynik = porownaj(a->wykladnik, b->wykladnik);
    if (wynik != 0) return wynik;

    /* Wykładniki równe -> porównaj resztę listy */
    return porownaj(a->reszta, b->reszta);
}

/**
 * Tworzy liczbę kolosalną reprezentującą 1 (czyli 2^0).
 */
Kolosalna *stworz_jeden(void) {
    return stworz_wezel(NULL, NULL);
}

/**
 * Dodaje dwie liczby kolosalne.
 * Zwraca nową liczbę.
 * Funkcja dba o to, by wynik był znormalizowany (posortowany i scalony).
 */
Kolosalna *dodaj(Kolosalna *a, Kolosalna *b) {
    if (a == NULL) {
        zwieksz_ref(b);
        return b;
    }
    if (b == NULL) {
        zwieksz_ref(a);
        return a;
    }

    int cmp = porownaj(a->wykladnik, b->wykladnik);

    if (cmp > 0) {
        /* a > b: dodajemy b do reszty a */
        Kolosalna *nowa_reszta = dodaj(a->reszta, b);

        /* Sprawdzenie normalizacji po powrocie z rekurencji */
        if (nowa_reszta != NULL && porownaj(nowa_reszta->wykladnik, 
a->wykladnik) >= 0) {
            Kolosalna *glowa_a = stworz_wezel(a->wykladnik, NULL);
            Kolosalna *wynik = dodaj(glowa_a, nowa_reszta);
            zmniejsz_ref(glowa_a);
            zmniejsz_ref(nowa_reszta);
            return wynik;
        } else {
            Kolosalna *wynik = stworz_wezel(a->wykladnik, nowa_reszta);
            zmniejsz_ref(nowa_reszta);
            return wynik;
        }
    } else if (cmp < 0) {
        /* b > a: dodajemy a do reszty b */
        Kolosalna *nowa_reszta = dodaj(a, b->reszta);

        if (nowa_reszta != NULL && porownaj(nowa_reszta->wykladnik, 
b->wykladnik) >= 0) {
            Kolosalna *glowa_b = stworz_wezel(b->wykladnik, NULL);
            Kolosalna *wynik = dodaj(glowa_b, nowa_reszta);
            zmniejsz_ref(glowa_b);
            zmniejsz_ref(nowa_reszta);
            return wynik;
        } else {
            Kolosalna *wynik = stworz_wezel(b->wykladnik, nowa_reszta);
            zmniejsz_ref(nowa_reszta);
            return wynik;
        }
    } else {
        /* Wykładniki równe: 2^E + 2^E = 2^(E+1) */
        Kolosalna *jeden = stworz_jeden();
        Kolosalna *nowy_wykladnik = dodaj(a->wykladnik, jeden);
        zmniejsz_ref(jeden);

        /* Sumujemy reszty list (składniki < 2^E) */
        Kolosalna *suma_reszt = dodaj(a->reszta, b->reszta);

        /* Wynik to 2^(E+1) + suma_reszt */
        Kolosalna *wynik = stworz_wezel(nowy_wykladnik, suma_reszt);
        zmniejsz_ref(nowy_wykladnik);
        zmniejsz_ref(suma_reszt);

        return wynik;
    }
}

/**
 * Mnoży składnik 2^skalar przez liczbę b.
 */
Kolosalna *mnoz_skalar(Kolosalna *skalar, Kolosalna *b) {
    if (b == NULL) return NULL;

    Kolosalna *nowy_wykladnik = dodaj(skalar, b->wykladnik);
    Kolosalna *reszta = mnoz_skalar(skalar, b->reszta);

    Kolosalna *wynik = stworz_wezel(nowy_wykladnik, reszta);
    zmniejsz_ref(nowy_wykladnik);
    zmniejsz_ref(reszta);

    return wynik;
}

/**
 * Mnoży dwie liczby kolosalne.
 */
Kolosalna *mnoz(Kolosalna *a, Kolosalna *b) {
    if (a == NULL || b == NULL) return NULL;

    Kolosalna *czesc1 = mnoz_skalar(a->wykladnik, b);
    Kolosalna *czesc2 = mnoz(a->reszta, b);

    Kolosalna *wynik = dodaj(czesc1, czesc2);
    zmniejsz_ref(czesc1);
    zmniejsz_ref(czesc2);

    return wynik;
}

/**
 * Oblicza 2 do potęgi podanej jako argument.
 */
Kolosalna *potega_dwojki(Kolosalna *wykladnik) {
    return stworz_wezel(wykladnik, NULL);
}

/**
 * Parsuje liczbę kolosalną z wejścia.
 */
Kolosalna *czytaj_kolosalna(void) {
    Kolosalna *akumulator = NULL;

    while (true) {
        int c = getchar();

        if (c == '0') {
            return akumulator;
        } else if (c == '1') {
            Kolosalna *wykladnik = czytaj_kolosalna();
            Kolosalna *skladnik = stworz_wezel(wykladnik, NULL);
            zmniejsz_ref(wykladnik);

            Kolosalna *nowy_akumulator = dodaj(akumulator, skladnik);
            zmniejsz_ref(akumulator);
            zmniejsz_ref(skladnik);
            akumulator = nowy_akumulator;
        }
    }
}

/**
 * Wypisuje liczbę kolosalną w formacie tekstowym.
 */
void pisz_kolosalna(Kolosalna *k) {
    while (k != NULL) {
        putchar('1');
        pisz_kolosalna(k->wykladnik);
        k = k->reszta;
    }
    putchar('0');
}

/**
 * Inicjalizuje stos.
 */
void stos_init(Stos *s) {
    s->dane = malloc(STOS_POJEMNOSC_POCZATKOWA * sizeof(Kolosalna *));
    if (!s->dane) {
        fprintf(stderr, "Błąd alokacji pamięci\n");
        exit(1);
    }
    s->rozmiar = 0;
    s->pojemnosc = STOS_POJEMNOSC_POCZATKOWA;
}

/**
 * Wkłada liczbę na stos.
 */
void stos_odloz(Stos *s, Kolosalna *k) {
    if (s->rozmiar == s->pojemnosc) {
        s->pojemnosc *= 2;
        s->dane = realloc(s->dane, s->pojemnosc * sizeof(Kolosalna *));
        if (!s->dane) {
            fprintf(stderr, "Błąd realokacji stosu\n");
            exit(1);
        }
    }
    s->dane[s->rozmiar++] = k;
}

/**
 * Zdejmuje liczbę ze stosu.
 */
Kolosalna *stos_zdejmij(Stos *s) {
    if (s->rozmiar == 0) {
        fprintf(stderr, "Błąd: pusty stos\n");
        exit(1);
    }
    return s->dane[--s->rozmiar];
}

/**
 * Czyści stos i zwalnia pamięć.
 */
void stos_czysc(Stos *s) {
    while (s->rozmiar > 0) {
        zmniejsz_ref(s->dane[--s->rozmiar]);
    }
    free(s->dane);
}

/**
 * Obsługuje polecenie wypisywania liczby (.).
 */
void obsluz_wypisywanie(Stos *s) {
    Kolosalna *a = stos_zdejmij(s);
    pisz_kolosalna(a);
    putchar('\n');
    zmniejsz_ref(a);
}

/**
 * Obsługuje polecenie duplikacji liczby (:).
 */
void obsluz_duplikacje(Stos *s) {
    Kolosalna *a = stos_zdejmij(s);
    zwieksz_ref(a);
    stos_odloz(s, a);
    zwieksz_ref(a);
    stos_odloz(s, a);
    zmniejsz_ref(a);
}

/**
 * Obsługuje polecenie potęgowania (^).
 */
void obsluz_potegowanie(Stos *s) {
    Kolosalna *a = stos_zdejmij(s);
    Kolosalna *wynik = potega_dwojki(a);
    stos_odloz(s, wynik);
    zmniejsz_ref(a);
}

/**
 * Obsługuje polecenie dodawania (+).
 */
void obsluz_dodawanie(Stos *s) {
    Kolosalna *b = stos_zdejmij(s);
    Kolosalna *a = stos_zdejmij(s);
    Kolosalna *wynik = dodaj(a, b);
    stos_odloz(s, wynik);
    zmniejsz_ref(a);
    zmniejsz_ref(b);
}

/**
 * Obsługuje polecenie mnożenia (*).
 */
void obsluz_mnozenie(Stos *s) {
    Kolosalna *b = stos_zdejmij(s);
    Kolosalna *a = stos_zdejmij(s);
    Kolosalna *wynik = mnoz(a, b);
    stos_odloz(s, wynik);
    zmniejsz_ref(a);
    zmniejsz_ref(b);
}

/**
 * Główna funkcja programu.
 */
int main(void) {
    Stos stos;
    stos_init(&stos);

    int c;
    while ((c = getchar()) != EOF) {
        if (isspace(c)) continue;

        if (c == '0' || c == '1') {
            ungetc(c, stdin);
            Kolosalna *liczba = czytaj_kolosalna();
            stos_odloz(&stos, liczba);
        } else if (c == '.') {
            obsluz_wypisywanie(&stos);
        } else if (c == ':') {
            obsluz_duplikacje(&stos);
        } else if (c == '^') {
            obsluz_potegowanie(&stos);
        } else if (c == '+') {
            obsluz_dodawanie(&stos);
        } else if (c == '*') {
            obsluz_mnozenie(&stos);
        }
    }

    stos_czysc(&stos);
    return 0;
}
