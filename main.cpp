#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <numeric>
#include <random>
#include <chrono>
#include <fstream>

// Constantes globales
const unsigned int GENERACIONES = 20;
const unsigned int POBLACION_INICIAL = 10000;

const unsigned int MAX_HIJOS = 12;
const double lambda = 4.75; // punto medio de la distribución de Poisson

const double porcentajeHereditario = 0.7;

// Variables de acceso globales
unsigned int POBLACION_ACTUAL;
unsigned int HOMBRES;
unsigned int MUJERES;


// Clase Persona
struct Persona {
    bool hombre; // true = hombre, false = mujer
    double atraccionHombre; // porcentaje de atracción a hombres
    double atraccionMujer; // porcentaje de atracción a mujeres
    bool emparejado; // booleano que nos indica si en un encuentro está ya emparejada la persona o no
};

// Precálculo del factorial hasta MAX_HIJOS
int factorial[MAX_HIJOS + 1];

void precalculoFactorial() {
    factorial[0] = 1;
    for (int i = 1; i <= MAX_HIJOS; i++)
        factorial[i] = i * factorial[i-1];
}

// Precálculo de la distribución de Poisson para el número de hijos
std::discrete_distribution<int> distribucionPoisson;
std::random_device rd;
std::mt19937 gen(rd());

void precalculoPoisson() {

    precalculoFactorial();

    // La fórmula de Poisson es:
    // P(X = k) = (lambda^k * e^-k) / k!

    std::vector<double> pesos(MAX_HIJOS + 1);

    for (int k = 0; k <= MAX_HIJOS; k++) {
        pesos[k] = (std::pow(lambda, k) * std::exp(-lambda)) / factorial[k];
    }

    // Creamos la distribución
    distribucionPoisson = std::discrete_distribution(pesos.begin(), pesos.end());
}

// Función que devuelve un double aleatorio entre 0 y 1
double randomDouble() {
    return ((double)rand() / (double)RAND_MAX);
}

// Función que devuelve una permutación aleatoria de 0 a n-1
std::vector<int> permutacionAleatoria(int n) {
    std::vector<int> permutacion(n);
    std::iota(permutacion.begin(), permutacion.end(), 0);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(permutacion.begin(), permutacion.end(), std::default_random_engine(seed));

    return permutacion;
}

// Función que calcula los encuentros y devuelve la lista de parejas actualizada
void calcularEncuentros(std::vector<Persona>& generacionActual, std::vector<std::pair<Persona, Persona>>& parejas) {

    // Tomamos una permutación aleatoria y a partir de ella realizamos los encuentros
    std::vector<int> permutacion = permutacionAleatoria(POBLACION_ACTUAL);

    // Ahora para cada par de índices, vamos a realizar los encuentros
    for (int i = 0; i+1 < POBLACION_ACTUAL; i += 2) {

        Persona& A = generacionActual[permutacion[i]];
        Persona& B = generacionActual[permutacion[i+1]];

        // Si alguno está ya emparejado, nos lo saltamos
        if (A.emparejado || B.emparejado) continue;

        // La probabilidad de que sean emparejados es el 
        // producto de los porcentajes cruzados
        double porcentajeAB = (B.hombre ? A.atraccionHombre : A.atraccionMujer);
        double porcentajeBA = (A.hombre ? B.atraccionHombre : B.atraccionMujer);

        double probabilidadEmparejamiento = porcentajeAB * porcentajeBA;

        // Ahora comprobamos si hay emparejamiento
        if (randomDouble() < probabilidadEmparejamiento) {
            // Hay emparejamiento, lo guardamos y actualizamos datos
            parejas.emplace_back(A, B);
            A.emparejado = B.emparejado = true;
        }
    }
}

// Función que calcula la siguiente generación dada la actual
void calcularGeneracion(std::vector<Persona>& generacionActual) {

    // Primero nos guardamos algunos datos relevantes
    POBLACION_ACTUAL = generacionActual.size();
    
    HOMBRES = 0;
    for (Persona p : generacionActual) 
        HOMBRES += (p.hombre);
    

    MUJERES = POBLACION_ACTUAL - HOMBRES;

    // Ahora vamos a realizar rondas de encuentros
    const unsigned int ENCUENTROS = POBLACION_ACTUAL / 100;

    std::vector<std::pair<Persona, Persona>> parejas;

    for (int i = 0; i < ENCUENTROS; i++) 
        calcularEncuentros(generacionActual, parejas);
    
    // Ya teniendo parejas formadas, vamos a calcular la siguiente generación.
    // Para ello, vamos a utilizar una distribución de Poisson truncada,
    // llamando a la distribución precalculada con distribucionPoisson(gen).

    std::vector<Persona> nuevaGeneracion;

    for (auto& [A, B] : parejas) {

        // Evitamos que las parejas del mismo sexo tengan hijos
        if (A.hombre == B.hombre) continue;

        unsigned int numHijos = distribucionPoisson(gen);

        for (int i = 0; i < numHijos; i++) {
            Persona hijo;
            hijo.hombre = (randomDouble() < 0.5);

            double atraccionHeredadaHombre = (A.atraccionHombre + B.atraccionHombre) / 2;
            double atraccionHeredadaMujer = (A.atraccionMujer + B.atraccionMujer) / 2;

            // Introducimos aleatoriedad en el hijo
            double atraccionAleatoriaGeneral = randomDouble();
            double atraccionAleatoriaHombre = atraccionAleatoriaGeneral * randomDouble();
            double atraccionAleatoriaMujer = atraccionAleatoriaGeneral - atraccionAleatoriaHombre;

            hijo.atraccionHombre = atraccionHeredadaHombre * porcentajeHereditario + atraccionAleatoriaHombre * (1 - porcentajeHereditario);
            hijo.atraccionMujer = atraccionHeredadaMujer * porcentajeHereditario + atraccionAleatoriaMujer * (1 - porcentajeHereditario);
            
            hijo.emparejado = false;

            nuevaGeneracion.push_back(hijo);
        }
    }


    generacionActual = nuevaGeneracion;
}

int main() {

    srand(time(NULL));

    precalculoPoisson();

    // Abrimos un archivo CSV para guardar los datos
    std::ofstream archivoCSV("simulacion.csv");
    archivoCSV << "Generacion,Poblacion,AtraccionMediaHombreAHombre,AtraccionMediaHombreAMujer,AtraccionMediaMujerAHombre,AtraccionMediaMujerAMujer\n";

    // Calculamos la generación 0 con atributos aleatorios
    std::vector<Persona> generacionActual(POBLACION_INICIAL);
    double sumaAtrHH = 0, sumaAtrHM = 0, sumaAtrMH = 0, sumaAtrMM = 0;

    for (Persona& p : generacionActual) {
        p.hombre = (randomDouble() < 0.5);
        
        double atraccionGeneral = randomDouble();
        p.atraccionHombre = atraccionGeneral * randomDouble();
        p.atraccionMujer = atraccionGeneral - p.atraccionHombre;

        p.emparejado = false;

        HOMBRES += (p.hombre);

        sumaAtrHH += (p.hombre ? p.atraccionHombre : 0);
        sumaAtrHM += (p.hombre ? p.atraccionMujer : 0);
        sumaAtrMH += (!p.hombre ? p.atraccionHombre : 0);
        sumaAtrMM += (!p.hombre ? p.atraccionMujer : 0);
    }

    MUJERES = POBLACION_INICIAL - HOMBRES;

    // Guardamos los datos de la generación cero
    archivoCSV << "0, " << POBLACION_INICIAL << ","
               << sumaAtrHH / HOMBRES << ","
               << sumaAtrHM / HOMBRES << ","
               << sumaAtrMH / MUJERES << ","
               << sumaAtrMM / MUJERES << "\n";

    // Ahora calculamos hasta el número de generaciones establecido
    std::cout << "Simulando generaciones...\n";

    for (int i = 0; i < GENERACIONES; i++) {
        
        // Calculamos la siguiente generación
        calcularGeneracion(generacionActual);

        // Calculamos las estadísticas de la nueva generación

        sumaAtrHH = 0;
        sumaAtrHM = 0;
        sumaAtrMH = 0;
        sumaAtrMM = 0;
        for (const Persona& p : generacionActual) {
            sumaAtrHH += (p.hombre ? p.atraccionHombre : 0);
            sumaAtrHM += (p.hombre ? p.atraccionMujer : 0);
            sumaAtrMH += (!p.hombre ? p.atraccionHombre : 0);
            sumaAtrMM += (!p.hombre ? p.atraccionMujer : 0);
        }

        double mediaHH = (HOMBRES ? sumaAtrHH / HOMBRES : 0);
        double mediaHM = (HOMBRES ? sumaAtrHM / HOMBRES : 0);
        double mediaMH = (MUJERES ? sumaAtrMH / MUJERES : 0);
        double mediaMM = (MUJERES ? sumaAtrMM / MUJERES : 0);
        
        // Escribimos en el CSV
        archivoCSV << i+1 << "," << POBLACION_ACTUAL << ","
                   << mediaHH << "," << mediaHM << "," 
                   << mediaMH << "," << mediaMM << "\n";

        std::cout << "Generación " << i+1 << ": " << POBLACION_ACTUAL << " habitantes.\n";
    }

    archivoCSV.close();
    std::cout << "Simulación terminada. Datos en 'simulacion.csv'.\n";
}