#include <iostream>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <fstream>

using namespace std;

// ====== PARAMETRY OBIEKTU REFERENCYJNEGO (wzorcowego) ======
const double K_REF = 1.0;
const double T_REF = 1.0;
const double XI_REF = 0.3;

// ====== FORMATOWANIE WYNIKÓW (np. do CSV) ======
string format(double value) {
    ostringstream oss;
    oss << fixed << setprecision(5) << value;
    string str = oss.str();
    for (char& c : str) {
        if (c == '.') c = ',';  // przecinek jako separator dziesiętny
    }
    return str;
}

// ====== FUNKCJE OBIEKTU WZORCOWEGO ======
double skokowa_h(double t) {
    double sqrt_part = sqrt(1 - XI_REF * XI_REF);
    double exp_part = exp(-XI_REF * t / T_REF);
    double omega = sqrt_part / T_REF;
    double phase = atan(sqrt_part / XI_REF);
    double sin_part = sin(omega * t + phase);
    return K_REF * (1 - (1.0 / sqrt_part) * exp_part * sin_part);
}

double impulsowa_g(double t) {
    double sqrt_part = sqrt(1 - XI_REF * XI_REF);
    double coef = K_REF / (T_REF * sqrt_part);
    double omega = sqrt_part / T_REF;
    return coef * exp(-XI_REF * t / T_REF) * sin(omega * t);
}

// ====== STRUKTURA OSOBNIKA ======
struct Osobnik {
    double K, T, xi;
    double J;

    Osobnik(double k, double t, double x)
        : K(k), T(t), xi(x), J(0.0) {}
};

// ====== LOSOWANIE LICZBY Z PRZEDZIAŁU ======
double losuj_z_przedzialu(double min, double max) {
    return min + (double(rand()) / RAND_MAX) * (max - min);
}

// ====== FUNKCJE MODELU (DLA OSOBNIKA) ======
double model_h(double t, double K, double T, double xi) {
    double sqrt_part = sqrt(1 - xi * xi);
    double exp_part = exp(-xi * t / T);
    double omega = sqrt_part / T;
    double phase = atan(sqrt_part / xi);
    double sin_part = sin(omega * t + phase);
    return K * (1 - (1.0 / sqrt_part) * exp_part * sin_part);
}

double model_g(double t, double K, double T, double xi) {
    double sqrt_part = sqrt(1 - xi * xi);
    double coef = K / (T * sqrt_part);
    double omega = sqrt_part / T;
    return coef * exp(-xi * t / T) * sin(omega * t);
}

// ====== OCENA JAKOŚCI OSOBNIKA ======
double ocen_J(Osobnik& o) {
    double suma = 0.0;
    for (double t = 0.0; t <= 12.0; t += 0.1) {
        double h_m = model_h(t, o.K, o.T, o.xi);
        double h_r = skokowa_h(t);
        double g_m = model_g(t, o.K, o.T, o.xi);
        double g_r = impulsowa_g(t);
        suma += pow(h_m - h_r, 2) + pow(g_m - g_r, 2);
    }
    o.J = suma;
    return suma;
}

// ====== GENEROWANIE OSOBNIKA LOSOWEGO ======
Osobnik generuj_osobnika(double Kmin, double Kmax, double Tmin, double Tmax, double ximin, double ximax) {
    double k = losuj_z_przedzialu(Kmin, Kmax);
    double t = losuj_z_przedzialu(Tmin, Tmax);
    double xi = losuj_z_przedzialu(ximin, ximax);
    return Osobnik(k, t, xi);
}

// ====== GENEROWANIE POPULACJI POCZĄTKOWEJ ======
vector<Osobnik> generuj_populacje(int rozmiar,
    double Kmin, double Kmax,
    double Tmin, double Tmax,
    double ximin, double ximax)
{
    vector<Osobnik> populacja;
    for (int i = 0; i < rozmiar; ++i) {
        Osobnik o = generuj_osobnika(Kmin, Kmax, Tmin, Tmax, ximin, ximax);
        ocen_J(o);
        populacja.push_back(o);
    }
    return populacja;
}

// ====== SELEKCJA RANKINGOWA ======
vector<Osobnik> selekcja_rankingowa(const vector<Osobnik>& populacja, int liczba_wybranych) {
    // 1. Kopiujemy i sortujemy populację według wartości J (rosnąco)
    vector<Osobnik> posortowana = populacja;
    sort(posortowana.begin(), posortowana.end(), [](const Osobnik& a, const Osobnik& b) {
        return a.J < b.J;
        });

    // 2. Wyliczamy prawdopodobieństwa rankingowe
    int N = posortowana.size();
    vector<double> prawdopodobienstwa(N);
    double suma_rang = (N * (N + 1)) / 2.0;

    for (int i = 0; i < N; ++i) {
        prawdopodobienstwa[i] = (N - i) / suma_rang;
    }

    // 3. Rozkład skumulowany
    vector<double> rozklad(N);
    rozklad[0] = prawdopodobienstwa[0];
    for (int i = 1; i < N; ++i) {
        rozklad[i] = rozklad[i - 1] + prawdopodobienstwa[i];
    }

    // 4. Losujemy osobników wg rozkładu
    vector<Osobnik> wybrani;
    for (int i = 0; i < liczba_wybranych; ++i) {
        double r = losuj_z_przedzialu(0.0, 1.0);
        for (int j = 0; j < N; ++j) {
            if (r <= rozklad[j]) {
                wybrani.push_back(posortowana[j]);
                break;
            }
        }
    }

    return wybrani;
}


// ====== KRZYŻOWANIE ARYTMETYCZNE ======
pair<Osobnik, Osobnik> krzyzowanie_arytmetyczne(const Osobnik& r1, const Osobnik& r2, double pk) {
    double los = losuj_z_przedzialu(0.0, 1.0);
    if (los > pk) {
        return { r1, r2 };
    }

    double a = losuj_z_przedzialu(0.0, 1.0);
    Osobnik d1(
        a * r1.K + (1 - a) * r2.K,
        a * r1.T + (1 - a) * r2.T,
        a * r1.xi + (1 - a) * r2.xi
    );
    Osobnik d2(
        (1 - a) * r1.K + a * r2.K,
        (1 - a) * r1.T + a * r2.T,
        (1 - a) * r1.xi + a * r2.xi
    );
    return { d1, d2 };
}


vector<Osobnik> krzyzuj_populacje(const vector<Osobnik>& wybrani, double pk) {
    vector<Osobnik> potomkowie;
    for (size_t i = 0; i + 1 < wybrani.size(); i += 2) {
        pair<Osobnik, Osobnik> dzieci = krzyzowanie_arytmetyczne(wybrani[i], wybrani[i + 1], pk);
        potomkowie.push_back(dzieci.first);
        potomkowie.push_back(dzieci.second);
    }
    if (wybrani.size() % 2 == 1)
        potomkowie.push_back(wybrani.back());
    return potomkowie;
}

// ====== MUTACJA RÓWNOMIERNA ======
void mutuj_osobnika(Osobnik& o, double pm, double Kmin, double Kmax, double Tmin, double Tmax, double ximin, double ximax) {
    if (losuj_z_przedzialu(0.0, 1.0) < pm) o.K = losuj_z_przedzialu(Kmin, Kmax);
    if (losuj_z_przedzialu(0.0, 1.0) < pm) o.T = losuj_z_przedzialu(Tmin, Tmax);
    if (losuj_z_przedzialu(0.0, 1.0) < pm) o.xi = losuj_z_przedzialu(ximin, ximax);
}


void mutuj_populacje(vector<Osobnik>& populacja, double pm, double Kmin, double Kmax, double Tmin, double Tmax, double ximin, double ximax) {
    for (auto& o : populacja)
        mutuj_osobnika(o, pm, Kmin, Kmax, Tmin, Tmax, ximin, ximax);
}

// ====== GŁÓWNA PĘTLA ALGORYTMU GENETYCZNEGO ======
Osobnik uruchom_algorytm_genetyczny(int liczba_iter, int N, double pk, double pm,
    double Kmin, double Kmax, double Tmin, double Tmax, double ximin, double ximax)
{
    vector<Osobnik> populacja = generuj_populacje(N, Kmin, Kmax, Tmin, Tmax, ximin, ximax);
    Osobnik najlepszy = populacja[0];

    for (int i = 0; i < liczba_iter; ++i) {
        vector<Osobnik> wybrani = selekcja_rankingowa(populacja, N);
        vector<Osobnik> potomkowie = krzyzuj_populacje(wybrani, pk);
        mutuj_populacje(potomkowie, pm, Kmin, Kmax, Tmin, Tmax, ximin, ximax);

        for (auto& o : potomkowie) {
            ocen_J(o);
            if (o.J < najlepszy.J) najlepszy = o;
        }
        populacja = potomkowie;
        cout << "Iteracja " << i + 1 << ", J=" << najlepszy.J << endl;
    }

    return najlepszy;
}

// ====== ZAPIS DO PLIKU CSV ======
void testuj_10_razy_i_zapisz_do_csv(int liczba_iteracji, int rozmiar_populacji, double pk, double pm,
    double Kmin, double Kmax, double Tmin, double Tmax, double ximin, double ximax,
    const string& plik_statystyki, const string& plik_grafy)
{
    ofstream plik1(plik_statystyki); // plik ze statystykami
    ofstream plik2(plik_grafy);      // plik do wykresów

    if (!plik1.is_open() || !plik2.is_open()) {
        cerr << "Błąd otwarcia plików!" << endl;
        return;
    }

    // Nagłówki
    plik1 << "nr_testu;K;T;xi;J\n";
    plik2 << "nr_testu;t;h_real;h_model;g_real;g_model;J\n";

    double suma_J = 0.0, suma_K = 0.0, suma_T = 0.0, suma_xi = 0.0;

    for (int i = 1; i <= 10; ++i) {
        Osobnik najlepszy = uruchom_algorytm_genetyczny(
            liczba_iteracji, rozmiar_populacji, pk, pm,
            Kmin, Kmax, Tmin, Tmax, ximin, ximax
        );

        // Zapis do pliku statystyk
        plik1 << i << ";" << format(najlepszy.K) << ";" << format(najlepszy.T) << ";"
            << format(najlepszy.xi) << ";" << format(najlepszy.J) << "\n";

        // Zapis do pliku wykresów (dla każdego t)
        for (double t = 0.0; t <= 12.0; t += 0.1) {
            double h_r = skokowa_h(t);
            double h_m = model_h(t, najlepszy.K, najlepszy.T, najlepszy.xi);
            double g_r = impulsowa_g(t);
            double g_m = model_g(t, najlepszy.K, najlepszy.T, najlepszy.xi);

            plik2 << i << ";" << format(t) << ";"
                << format(h_r) << ";" << format(h_m) << ";"
                << format(g_r) << ";" << format(g_m) << ";"
                << format(najlepszy.J) << "\n";
        }

        suma_K += najlepszy.K;
        suma_T += najlepszy.T;
        suma_xi += najlepszy.xi;
        suma_J += najlepszy.J;

        cout << "Test " << i << " | J=" << najlepszy.J << " | K=" << najlepszy.K
            << " T=" << najlepszy.T << " xi=" << najlepszy.xi << endl;
    }

    // Średnie wartości
    double avg_K = suma_K / 10.0;
    double avg_T = suma_T / 10.0;
    double avg_xi = suma_xi / 10.0;
    double avg_J = suma_J / 10.0;

    // Dopisujemy średnią do pliku statystyk
    plik1 << "srednia;" << format(avg_K) << ";" << format(avg_T) << ";" << format(avg_xi) << ";" << format(avg_J) << "\n";

    plik1.close();
    plik2.close();

    cout << "\nŚREDNIE z 10 testów:\n";
    cout << "K = " << avg_K << ", T = " << avg_T << ", xi = " << avg_xi << ", J = " << avg_J << endl;
}


// ====== MAIN ======
#include <sstream> // додати на початку файлу

int main() {
    srand(time(0)); // inicjalizacja generatora rand()

    // ====== Domyślne wartości ======
    double Kmin = 0.5, Kmax = 2.0;
    double Tmin = 0.5, Tmax = 2.0;
    double ximin = 0.1, ximax = 0.9;
    int liczba_iteracji = 100;
    int rozmiar_populacji = 30;
    double pk = 0.7;
    double pm = 0.05;

    string linia;
    cout << "Podaj przedział dla K (min max) [domyślnie: 0.5 2.0]: ";
    getline(cin, linia);
    if (!linia.empty()) {
        stringstream ss(linia);
        ss >> Kmin >> Kmax;
    }

    cout << "Podaj przedział dla T (min max) [domyślnie: 0.5 2.0]: ";
    getline(cin, linia);
    if (!linia.empty()) {
        stringstream ss(linia);
        ss >> Tmin >> Tmax;
    }

    cout << "Podaj przedział dla xi (min max) [domyślnie: 0.1 0.9]: ";
    getline(cin, linia);
    if (!linia.empty()) {
        stringstream ss(linia);
        ss >> ximin >> ximax;
    }

    cout << "Podaj liczbę iteracji AG [domyślnie: 100]: ";
    getline(cin, linia);
    if (!linia.empty()) {
        liczba_iteracji = stoi(linia);
    }

    cout << "Podaj rozmiar populacji [domyślnie: 30]: ";
    getline(cin, linia);
    if (!linia.empty()) {
        rozmiar_populacji = stoi(linia);
    }

    cout << "Podaj prawdopodobieństwo krzyżowania (0 - 1) [domyślnie: 0.7]: ";
    getline(cin, linia);
    if (!linia.empty()) {
        pk = stod(linia);
    }

    cout << "Podaj prawdopodobieństwo mutacji (0 - 1) [domyślnie: 0.05]: ";
    getline(cin, linia);
    if (!linia.empty()) {
        pm = stod(linia);
    }

    // ====== Uruchomienie testów i zapis wyników ======
    testuj_10_razy_i_zapisz_do_csv(
        liczba_iteracji, rozmiar_populacji, pk, pm,
        Kmin, Kmax, Tmin, Tmax, ximin, ximax,
        "statystyki_10_testow.csv",
        "wyniki_modelu.csv"
    );

    return 0;
}




