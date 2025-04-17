#include <iostream>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

const double K = 1.0;
const double T = 1.0;
const double xi = 0.3;


string format(double value) {
    ostringstream oss;
    oss << fixed << setprecision(5) << value;
    string str = oss.str();
    for (char& c : str) {
        if (c == '.') c = ','; 
    }
    return str;
}

double skokowa_h(double t) {
    double sqrt_part = sqrt(1 - xi * xi);
    double exp_part = exp(-xi * t / T);
    double omega = sqrt_part / T;
    double phase = atan(sqrt_part / xi);
    double sin_part = sin(omega * t + phase);
    double response = K * (1 - (1.0 / sqrt_part) * exp_part * sin_part);
    return response;
}

double impulsowa_g(double t) {

    double sqrt_part = sqrt(1 - xi * xi);
    double coef = K / (T * sqrt_part);
    double omega = sqrt_part / T;
    return coef * exp(-xi * t / T) * sin(omega * t);
}

int main() {
    cout << "t;h(t);g(t)\n";
    for (double t = 0.0; t <= 5.0; t += 0.1) {
        cout << format(t) << ";" << format(skokowa_h(t)) << ";" << format(impulsowa_g(t)) << "\n";
    }
    return 0;
}
