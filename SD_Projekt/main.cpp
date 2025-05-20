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
        if (c == '.') c = ',';  // przecinek jako separator dziesiêtny
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

// ====== LOSOWANIE LICZBY Z PRZEDZIA£U ======
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

// ====== OCENA JAKOŒCI OSOBNIKA ======
double ocen_J(Osobnik& o) {
    double suma = 0.0;
    for (double t = 0.0; t <= 20.0; t += 0.1) {
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

// ====== GENEROWANIE POPULACJI POCZ¥TKOWEJ ======
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
    vector<Osobnik> posortowana = populacja;
    sort(posortowana.begin(), posortowana.end(), [](const Osobnik& a, const Osobnik& b) {
        return a.J < b.J;
        });

    vector<double> prawdopodobienstwa;
    int N = posortowana.size();
    double suma_rang = (N * (N + 1)) / 2.0;
    for (int i = 0; i < N; ++i) {
        prawdopodobienstwa.push_back((N - i) / suma_rang);
    }

    vector<double> rozklad(N);
    rozklad[0] = prawdopodobienstwa[0];
    for (int i = 1; i < N; ++i)
        rozklad[i] = rozklad[i - 1] + prawdopodobienstwa[i];

    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    vector<Osobnik> wybrani;
    for (int i = 0; i < liczba_wybranych; ++i) {
        double r = dis(gen);
        for (int j = 0; j < N; ++j) {
            if (r <= rozklad[j]) {
                wybrani.push_back(posortowana[j]);
                break;
            }
        }
    }

    return wybrani;
}

// ====== KRZY¯OWANIE ARYTMETYCZNE ======
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

// ====== G£ÓWNA PÊTLA ALGORYTMU GENETYCZNEGO ======
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
void zapisz_do_csv(const Osobnik& najlepszy, const string& filename) {
    ofstream plik(filename);
    if (!plik.is_open()) {
        cerr << "B³¹d otwarcia pliku!" << endl;
        return;
    }
    plik << "t;h_real;h_model;g_real;g_model\n";
    for (double t = 0.0; t <= 20.0; t += 0.1) {
        plik << format(t) << ";"
            << format(skokowa_h(t)) << ";" << format(model_h(t, najlepszy.K, najlepszy.T, najlepszy.xi)) << ";"
            << format(impulsowa_g(t)) << ";" << format(model_g(t, najlepszy.K, najlepszy.T, najlepszy.xi)) << "\n";
    }
    plik.close();
    cout << "Zapisano dane do pliku: " << filename << endl;
}

// ====== MAIN ======
int main() {
    double Kmin = 0.5, Kmax = 2.0;
    double Tmin = 0.5, Tmax = 2.0;
    double ximin = 0.1, ximax = 0.9;

    int liczba_iteracji = 100;
    int rozmiar_populacji = 30;
    double pk = 0.7; // prawdopodobieñstwo krzy¿owania
    double pm = 0.05; // prawdopodobieñstwo mutacji

    srand(time(0));

    Osobnik najlepszy = uruchom_algorytm_genetyczny(
        liczba_iteracji, rozmiar_populacji, pk, pm,
        Kmin, Kmax, Tmin, Tmax, ximin, ximax
    );

    cout << "\n=== NAJLEPSZY OSOBNIK ===\n";
    cout << "K = " << najlepszy.K << ", T = " << najlepszy.T << ", xi = " << najlepszy.xi << ", J = " << najlepszy.J << endl;

    zapisz_do_csv(najlepszy, "wyniki_modelu.csv");
    return 0;
}
