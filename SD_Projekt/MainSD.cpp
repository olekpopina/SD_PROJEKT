include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>  // dla std::setw i std::setprecision

double stepResponse(double t, double K, double T, double xi) {
    if (xi <= 0 || xi >= 1) return 0.0;
    return K * (1 - (1.0 / sqrt(1 - xi * xi)) * exp(-xi * t / T) *
        sin(sqrt(1 - xi * xi) * t / T + atan(sqrt(1 - xi * xi) / xi)));
}

double impulseResponse(double t, double K, double T, double xi) {
    if (xi <= 0 || xi >= 1) return 0.0;
    return (K / (T * sqrt(1 - xi * xi))) *
        exp(-xi * t / T) * sin(sqrt(1 - xi * xi) * t / T);
}

int main() {
    double K = 2.0;
    double T = 1.0;
    double xi = 0.4;

    double t_max = 10.0;
    double dt = 0.1;

    std::ofstream file("output.csv");
    file << "t;h;g\n";  // separator œrednikowy (czytelny w Excel PL)

    std::cout << std::setw(8) << "t"
        << std::setw(15) << "h(t) (skok)"
        << std::setw(15) << "g(t) (impuls)" << std::endl;

    for (double t = 0.0; t <= t_max; t += dt) {
        double h = stepResponse(t, K, T, xi);
        double g = impulseResponse(t, K, T, xi);

        file << t << ";" << h << ";" << g << "\n";

        std::cout << std::setw(8) << std::fixed << std::setprecision(2) << t
            << std::setw(15) << std::setprecision(6) << h
            << std::setw(15) << std::setprecision(6) << g << std::endl;
    }

    file.close();
    std::cout << "\nPlik 'output.csv' zosta³ zapisany poprawnie." << std::endl;

    return 0;
}
