// Prueba del pool generacional reutilizable. No necesita GL ni ventana.
#include "Core/Memory/HandlePool.h"

#include "Check.h"

#include <iostream>
#include <string>

namespace {

struct Particle {
    std::string name;
    int lifetime = 0;
};

void testLifecycleAndReuse() {
    HandlePool<Particle> pool(4);
    const auto spark = pool.emplace(Particle{"spark", 3});
    const auto smoke = pool.emplace(Particle{"smoke", 8});
    require(pool.size() == 2);
    require(pool.get(spark) != nullptr && pool.get(spark)->name == "spark");

    require(pool.erase(spark));
    require(!pool.contains(spark));
    require(pool.get(spark) == nullptr);
    require(!pool.erase(spark));  // borrar dos veces no corrompe la lista libre

    const auto flame = pool.emplace(Particle{"flame", 5});
    require(flame.index == spark.index);
    require(flame.generation != spark.generation);
    require(pool.get(flame) != nullptr && pool.get(flame)->name == "flame");
    require(pool.get(smoke) != nullptr && pool.get(smoke)->lifetime == 8);

    std::cout << "[HANDLE POOL] ciclo de vida, invalidez y reutilizacion correctos.\n";
}

void testClearInvalidatesEverything() {
    HandlePool<int> pool;
    const auto first = pool.emplace(10);
    const auto second = pool.emplace(20);
    pool.clear();
    require(pool.empty());
    require(!pool.contains(first));
    require(!pool.contains(second));

    const auto current = pool.emplace(30);
    require(pool.contains(current));
    require(!pool.contains(first));
    require(!pool.contains(second));
    require(pool.get(current) != nullptr && *pool.get(current) == 30);

    std::cout << "[HANDLE POOL] clear invalida handles anteriores correctamente.\n";
}

}  // namespace

int main() {
    testLifecycleAndReuse();
    testClearInvalidatesEverything();
    std::cout << "\nTodas las comprobaciones del HandlePool han pasado correctamente.\n";
    return 0;
}
