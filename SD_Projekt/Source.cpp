//#include <iostream>
//#include <vector>
//#include <cmath>
//#include <iomanip>
//#include <sstream>
//#include <random>
//#include <algorithm>
//
//using namespace std;
//
//// Параметри AG
//const int POP_SIZE = 50;
//const int GENERATIONS = 100;
//const double CROSSOVER_RATE = 0.7;
//const double MUTATION_RATE = 0.1;
//const double K_MIN = 0.1, K_MAX = 5.0;
//const double T_MIN = 0.1, T_MAX = 5.0;
//const double XI_MIN = 0.01, XI_MAX = 0.99;
//
//struct Chromosome {
//    double K, T, xi;
//    double fitness;
//};
//
//// Вектор часових точок
//vector<double> t_vals;
//vector<double> h_obj, g_obj;
//
//// Форматування числа з точністю
//string format(double value) {
//    ostringstream oss;
//    oss << fixed << setprecision(5) << value;
//    string str = oss.str();
//    for (char& c : str) if (c == '.') c = ',';
//    return str;
//}
//
//// Функції моделі
//
//double skokowa_h(double t, double K, double T, double xi) {
//    double sqrt_part = sqrt(1 - xi * xi);
//    double exp_part = exp(-xi * t / T);
//    double omega = sqrt_part / T;
//    double phase = atan(sqrt_part / xi);
//    double sin_part = sin(omega * t + phase);
//    return K * (1 - (1.0 / sqrt_part) * exp_part * sin_part);
//}
//
//double impulsowa_g(double t, double K, double T, double xi) {
//    double sqrt_part = sqrt(1 - xi * xi);
//    double coef = K / (T * sqrt_part);
//    double omega = sqrt_part / T;
//    return coef * exp(-xi * t / T) * sin(omega * t);
//}
//
//// Випадкове число у межах [a, b]
//double randomInRange(double a, double b) {
//    static random_device rd;
//    static mt19937 gen(rd());
//    uniform_real_distribution<> dis(a, b);
//    return dis(gen);
//}
//
//// Обчислення функції якості (меньше — краще)
//double calculateFitness(const Chromosome& c) {
//    double error = 0.0;
//    for (size_t i = 0; i < t_vals.size(); ++i) {
//        double h_model = skokowa_h(t_vals[i], c.K, c.T, c.xi);
//        double g_model = impulsowa_g(t_vals[i], c.K, c.T, c.xi);
//        error += pow(h_model - h_obj[i], 2) + pow(g_model - g_obj[i], 2);
//    }
//    return error;
//}
//
//// Ініціалізація популяції
//vector<Chromosome> initializePopulation() {
//    vector<Chromosome> pop;
//    for (int i = 0; i < POP_SIZE; ++i) {
//        Chromosome c = {
//            randomInRange(K_MIN, K_MAX),
//            randomInRange(T_MIN, T_MAX),
//            randomInRange(XI_MIN, XI_MAX),
//            0.0
//        };
//        c.fitness = calculateFitness(c);
//        pop.push_back(c);
//    }
//    return pop;
//}
//
//// Відбір — турнірна селекція
//Chromosome tournamentSelection(const vector<Chromosome>& pop) {
//    Chromosome a = pop[rand() % POP_SIZE];
//    Chromosome b = pop[rand() % POP_SIZE];
//    return (a.fitness < b.fitness) ? a : b;
//}
//
//// Кросовер — однопунктовий
//pair<Chromosome, Chromosome> crossover(const Chromosome& p1, const Chromosome& p2) {
//    Chromosome c1 = p1, c2 = p2;
//    if (randomInRange(0, 1) < CROSSOVER_RATE) {
//        swap(c1.T, c2.T);
//    }
//    return { c1, c2 };
//}
//
//// Мутація — зміна параметра
//void mutate(Chromosome& c) {
//    if (randomInRange(0, 1) < MUTATION_RATE) c.K += randomInRange(-0.1, 0.1);
//    if (randomInRange(0, 1) < MUTATION_RATE) c.T += randomInRange(-0.1, 0.1);
//    if (randomInRange(0, 1) < MUTATION_RATE) c.xi += randomInRange(-0.05, 0.05);
//    c.K = min(max(c.K, K_MIN), K_MAX);
//    c.T = min(max(c.T, T_MIN), T_MAX);
//    c.xi = min(max(c.xi, XI_MIN), XI_MAX);
//}
//
//// Основна функція AG
//Chromosome runGA() {
//    vector<Chromosome> pop = initializePopulation();
//    for (int gen = 0; gen < GENERATIONS; ++gen) {
//        vector<Chromosome> new_pop;
//        while (new_pop.size() < POP_SIZE) {
//            Chromosome p1 = tournamentSelection(pop);
//            Chromosome p2 = tournamentSelection(pop);
//            pair<Chromosome, Chromosome> children = crossover(p1, p2);
//            Chromosome c1 = children.first;
//            Chromosome c2 = children.second;
//            mutate(c1); mutate(c2);
//            c1.fitness = calculateFitness(c1);
//            c2.fitness = calculateFitness(c2);
//            new_pop.push_back(c1);
//            if (new_pop.size() < POP_SIZE) new_pop.push_back(c2);
//        }
//        pop = new_pop;
//    }
//    return *min_element(pop.begin(), pop.end(), [](const Chromosome& a, const Chromosome& b) {
//        return a.fitness < b.fitness;
//        });
//}
//
//int main() {
//    // Генерація "реальних" характеристик для прикладу (можна замінити на файл)
//    for (double t = 0.0; t <= 10.0; t += 0.1) {
//        t_vals.push_back(t);
//        h_obj.push_back(skokowa_h(t, 1.0, 1.0, 0.3));
//        g_obj.push_back(impulsowa_g(t, 1.0, 1.0, 0.3));
//    }
//
//    Chromosome best = runGA();
//    cout << "Best found parameters:\n";
//    cout << "K = " << best.K << ", T = " << best.T << ", xi = " << best.xi << endl;
//    cout << "Fitness = " << best.fitness << endl;
//
//    return 0;
//}
