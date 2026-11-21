# C++-Tests

Die Testbenches sind analog zum Spade-Quellcode gruppiert:

- `foundations/`: Logikgatter, Addierer, Register, Zähler, FSM und Ripple-Carry-ALU
- `cpu/`: CPU-v1-Bausteine und `instruction6_model.hpp`
- `cpu_v2/`: CPU-v2-Bausteine, integrierte CPU und ISA-Demo

Tests werden weiterhin **im Repository-Root** mit den unveränderten
Make-Targets gestartet, z. B.:

```sh
make test_cpu_v2
make test_completed
make demo_cpu_v2
```

Das Makefile ordnet jedem Target den passenden C++-Quellpfad zu.
Die VCD-Dateien liegen unter `logic_basics/build/`.
