# Projekte im Repository-Root bauen und testen.

.DEFAULT_GOAL := help

VERILATOR_FLAGS := --quiet-build --quiet-stats --trace-vcd --cc --exe --build

.PHONY: help logic_basics test test_logic_basics synth_alu4 compare_alu4 test_mux test_half_adder test_full_adder test_add2 test_add4 test_register1 test_register4 test_counter4 test_shift4 test_traffic_light test_alu4 test_alu4_native test_alu4_ripple test_accumulator4

help:
	@printf 'Targets:\n  logic_basics       Spade-Code übersetzen\n  test_mux           Multiplexer testen\n  test_half_adder    Halbaddierer testen\n  test_full_adder    Volladdierer testen\n  test_add2          2-Bit-Addierer testen\n  test_add4          4-Bit-Addierer testen\n  test_register1     1-Bit-Register testen\n  test_register4     4-Bit-Register mit Enable testen\n  test_counter4      4-Bit-Zähler mit Enable testen\n  test_shift4        4-Bit-Schieberegister testen\n  test_traffic_light Ampel-Zustandsautomat testen\n  test_alu4         4-Bit-ALU testen\n  test_alu4_ripple  Ripple-Carry-Vergleichsvariante testen\n  test_alu4_native  Alias für test_alu4\n  test_accumulator4 4-Bit-Akkumulator testen\n  synth_alu4        ALU-Synthesebericht mit Yosys erzeugen\n  compare_alu4      Zellzahlen, Zelltypen und Logiktiefe vergleichen\n  test              Alle Tests ausführen\n  test_logic_basics  Alias für test\n'

logic_basics:
	@cd logic_basics && swim build

# Nur Analyse: generische Gatter nach Optimierung, keine FPGA-Flächenzahlen.
# Das Protokoll bleibt im ignorierten build/-Verzeichnis.
synth_alu4: logic_basics
	@cd logic_basics && yosys -Q -T -l build/alu4_synth.log -p \
		'read_verilog -sv build/spade.sv; synth -flatten -top alu4_top; stat'
	@printf 'Yosys-Protokoll: logic_basics/build/alu4_synth.log\n'

# Beide Varianten getrennt und identisch synthetisieren.
# ltp zählt Logikstufen des längsten Pfads, keine Zeit in Nanosekunden.
compare_alu4: logic_basics
	@cd logic_basics && yosys -Q -T -l build/alu4_ripple_compare.log -p \
		'read_verilog -sv build/spade.sv; synth -flatten -top alu4_ripple_top; stat; ltp -noff' > /dev/null
	@cd logic_basics && yosys -Q -T -l build/alu4_native_compare.log -p \
		'read_verilog -sv build/spade.sv; synth -flatten -top alu4_top; stat; ltp -noff' > /dev/null
	@printf '\nALU-Vergleich (generische Zellen):\n'
	@printf '  Ripple-Carry: '
	@cd logic_basics && awk -v top=alu4_ripple_top -f scripts/alu_cell_count.awk build/alu4_ripple_compare.log
	@printf '  Spade-Addition (+): '
	@cd logic_basics && awk -v top=alu4_top -f scripts/alu_cell_count.awk build/alu4_native_compare.log
	@printf '\nLaengster Logikpfad (Anzahl Stufen, keine ns):\n'
	@printf '  Ripple-Carry: '
	@cd logic_basics && awk -f scripts/alu_logic_depth.awk build/alu4_ripple_compare.log
	@printf '  Spade-Addition (+): '
	@cd logic_basics && awk -f scripts/alu_logic_depth.awk build/alu4_native_compare.log
	@printf '\nZelltypen in den Berichten: Abschnitte "=== alu4_* ===" nach "stat"\n'
	@printf 'Berichte: logic_basics/build/alu4_ripple_compare.log und alu4_native_compare.log\n'

test_mux: TOP := main
test_mux: TEST := mux

test_half_adder: TOP := half_adder_top
test_half_adder: TEST := half_adder

test_full_adder: TOP := full_adder_top
test_full_adder: TEST := full_adder

test_add2: TOP := add2_top
test_add2: TEST := add2

test_add4: TOP := add4_top
test_add4: TEST := add4

test_register1: TOP := register1_top
test_register1: TEST := register1

test_register4: TOP := register4_top
test_register4: TEST := register4

test_counter4: TOP := counter4_top
test_counter4: TEST := counter4

test_shift4: TOP := shift4_top
test_shift4: TEST := shift4

test_traffic_light: TOP := traffic_light_top
test_traffic_light: TEST := traffic_light

test_alu4: TOP := alu4_top
test_alu4: TEST := alu4

test_alu4_ripple: TOP := alu4_ripple_top
test_alu4_ripple: TEST := alu4_ripple

# Alter Target-Name bleibt als Alias erhalten; native Addition ist jetzt Standard.
test_alu4_native: test_alu4

test_accumulator4: TOP := accumulator4_top
test_accumulator4: TEST := accumulator4

test_mux test_half_adder test_full_adder test_add2 test_add4 test_register1 test_register4 test_counter4 test_shift4 test_traffic_light test_alu4 test_alu4_ripple test_accumulator4: logic_basics
	@cd logic_basics && ln -sfn "$$(verilator --getenv VERILATOR_ROOT)/include" build/verilator_include
	@cd logic_basics && verilator $(VERILATOR_FLAGS) \
		--top-module $(TOP) \
		--Mdir build/obj_$(TEST) \
		build/spade.sv "$(abspath logic_basics/test/$(TEST).cc)" && \
		./build/obj_$(TEST)/V$(TOP)

test: test_mux test_half_adder test_full_adder test_add2 test_add4 test_register1 test_register4 test_counter4 test_shift4 test_traffic_light test_alu4 test_alu4_ripple test_accumulator4

test_logic_basics: test
