Hwsem Basic Test

 - Tests the basic HWSEM API: initialization, lock, trylock, unlock, and
edge cases (invalid and boundary master IDs, wrong-master trylock,
same master ID across multiple HWSEM instances).

Hwsem Shared Peripheral Test

 - Tests sharing of a peripheral, such as an LED, between the cores of a
multi-core SoC. The test uses the default LED node declared in the
devicetree (led0) and toggles it. When multiple cores try to access the
LED at the same time, the core that has acquired the HWSEM owns the LED
and the other cores wait. Once the HWSEM is released, another core can
acquire it.

