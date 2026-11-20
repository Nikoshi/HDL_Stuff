# Projekte im Repository-Root bauen und testen.

.DEFAULT_GOAL := help

VERILATOR_FLAGS := --quiet-build --quiet-stats --trace-vcd --cc --exe --build

SPADE_DIR := logic_basics
SPADE_BUILD := $(SPADE_DIR)/build
SPADE_SRC := $(SPADE_DIR)/src
SPADE_SOURCES := $(shell find $(SPADE_SRC) -type f -name '*.spade' | sort)
# Nur tatsaechliche Build-Eingaben als Abhaengigkeiten auffuehren.
# Aenderungen an diesem Makefile (z. B. neue Test-Targets) loesen damit
# keinen kompletten Neubau aus. Nach Aenderungen an Compiler-Flags oder
# Verilator-/Swim-Aufrufen explizit "make clean" verwenden.
TOOLCHAIN_INPUTS := flake.nix flake.lock $(SPADE_DIR)/swim.toml $(SPADE_DIR)/swim.lock
FOUNDATIONS_COMMON := $(SPADE_SRC)/main.spade $(SPADE_SRC)/foundations/main.spade
CPU_COMMON := $(SPADE_SRC)/main.spade $(SPADE_SRC)/cpu/main.spade

.PHONY: demo_cpu_v2 help logic_basics test test_logic_basics synth_alu4 compare_alu4 test_mux test_half_adder test_full_adder test_add2 test_add4 test_register1 test_register4 test_counter4 test_shift4 test_traffic_light test_alu4 test_alu4_native test_alu4_ripple test_accumulator4 test_decoder6 test_instruction_exec6 test_pc4 test_rom16x6 test_cpu4 test_completed clean

help:
	@printf 'Targets:\n  logic_basics       Spade-Code übersetzen\n  test_mux           Multiplexer testen\n  test_half_adder    Halbaddierer testen\n  test_full_adder    Volladdierer testen\n  test_add2          2-Bit-Addierer testen\n  test_add4          4-Bit-Addierer testen\n  test_register1     1-Bit-Register testen\n  test_register4     4-Bit-Register mit Enable testen\n  test_counter4      4-Bit-Zähler mit Enable testen\n  test_shift4        4-Bit-Schieberegister testen\n  test_traffic_light Ampel-Zustandsautomat testen\n  test_alu4         4-Bit-ALU testen\n  test_alu4_ripple  Ripple-Carry-Vergleichsvariante testen\n  test_alu4_native  Alias für test_alu4\n  test_accumulator4 4-Bit-Akkumulator testen\n  test_decoder6     6-Bit-Befehlsdecoder testen\n  test_instruction_exec6 6-Bit-Befehle ausführen (Emulatorvergleich)\n  test_pc4          4-Bit-Programmzähler testen\n  test_rom16x6      16x6-Bit-Programmspeicher testen\n  test_cpu4         Integrierte 4-Bit-CPU gegen Emulator testen\n  demo_cpu_v2       ISA-Probeprogramm im C++-Emulator ausfuehren\n  synth_alu4        ALU-Synthesebericht mit Yosys erzeugen\n  compare_alu4      Zellzahlen, Zelltypen und Logiktiefe vergleichen\n  test              Alle Tests\n  test_completed    Alle abgeschlossenen Tests\n  test_logic_basics  Alias für test\n  clean             Generierte Builds und VCD-Dateien entfernen\n'

# Swim erzeugt eine gemeinsame SystemVerilog-Datei. Die Stamp-Datei
# verhindert erneute Aufrufe ohne geänderte Spade-/Toolchain-Eingaben.
$(SPADE_BUILD)/.spade-build.stamp: $(SPADE_SOURCES) $(TOOLCHAIN_INPUTS)
	@cd $(SPADE_DIR) && swim build
	@test -f $(SPADE_BUILD)/spade.sv
	@cd $(SPADE_DIR) && ln -sfn "$$(verilator --getenv VERILATOR_ROOT)/include" build/verilator_include
	@touch $@

logic_basics: $(SPADE_BUILD)/.spade-build.stamp

# Räumt auch die von den Tests erzeugten VCD-Traces auf.
clean:
	rm -rf $(SPADE_BUILD)

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

# Jedes Testprogramm hat eine eigene ausführbare Build-Datei als echtes Make-Ziel.
# Nur seine Spade-Abhängigkeiten (und sein C++-Test) erzwingen einen Neubau.
# Die Test-Targets selbst bleiben PHONY: Sie starten die vorhandenen Binaries
# bei jedem make test erneut, ohne sie unnötig zu kompilieren.
COMPLETED_TEST_NAMES := mux half_adder full_adder add2 add4 register1 register4 counter4 shift4 traffic_light alu4 alu4_ripple accumulator4 decoder6 instruction_exec6 pc4 rom16x6 cpu4
TEST_NAMES := $(COMPLETED_TEST_NAMES)

TEST_TOP_mux := main
TEST_SRCS_mux := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/mux.spade
TEST_TOP_half_adder := half_adder_top
TEST_SRCS_half_adder := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/test_tops.spade $(SPADE_SRC)/foundations/adders.spade
TEST_TOP_full_adder := full_adder_top
TEST_SRCS_full_adder := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/test_tops.spade $(SPADE_SRC)/foundations/adders.spade
TEST_TOP_add2 := add2_top
TEST_SRCS_add2 := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/test_tops.spade $(SPADE_SRC)/foundations/adders.spade
TEST_TOP_add4 := add4_top
TEST_SRCS_add4 := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/test_tops.spade $(SPADE_SRC)/foundations/adders.spade
TEST_TOP_register1 := register1_top
TEST_SRCS_register1 := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/registers.spade
TEST_TOP_register4 := register4_top
TEST_SRCS_register4 := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/registers.spade
TEST_TOP_counter4 := counter4_top
TEST_SRCS_counter4 := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/counters.spade
TEST_TOP_shift4 := shift4_top
TEST_SRCS_shift4 := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/shift_registers.spade
TEST_TOP_traffic_light := traffic_light_top
TEST_SRCS_traffic_light := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/fsm.spade
TEST_TOP_alu4 := alu4_top
TEST_SRCS_alu4 := $(CPU_COMMON) $(SPADE_SRC)/cpu/alu.spade
TEST_TOP_alu4_ripple := alu4_ripple_top
TEST_SRCS_alu4_ripple := $(FOUNDATIONS_COMMON) $(SPADE_SRC)/foundations/alu_ripple.spade $(SPADE_SRC)/foundations/adders.spade
TEST_TOP_accumulator4 := accumulator4_top
TEST_SRCS_accumulator4 := $(CPU_COMMON) $(SPADE_SRC)/cpu/accumulator.spade $(SPADE_SRC)/cpu/alu.spade
TEST_TOP_decoder6 := decoder6_top
TEST_SRCS_decoder6 := $(CPU_COMMON) $(SPADE_SRC)/cpu/decoder6.spade
TEST_TOP_instruction_exec6 := instruction_exec6_top
TEST_SRCS_instruction_exec6 := $(CPU_COMMON) $(SPADE_SRC)/cpu/instruction_exec6.spade $(SPADE_SRC)/cpu/decoder6.spade $(SPADE_SRC)/cpu/accumulator.spade $(SPADE_SRC)/cpu/alu.spade $(SPADE_DIR)/test/instruction6_model.hpp
TEST_TOP_pc4 := pc4_top
TEST_SRCS_pc4 := $(CPU_COMMON) $(SPADE_SRC)/cpu/pc4.spade
TEST_TOP_rom16x6 := rom16x6_top
TEST_SRCS_rom16x6 := $(CPU_COMMON) $(SPADE_SRC)/cpu/rom16x6.spade
TEST_TOP_cpu4 := cpu4_top
TEST_SRCS_cpu4 := $(CPU_COMMON) $(SPADE_SRC)/cpu/cpu4.spade $(SPADE_SRC)/cpu/pc4.spade $(SPADE_SRC)/cpu/rom16x6.spade $(SPADE_SRC)/cpu/instruction_exec6.spade $(SPADE_SRC)/cpu/decoder6.spade $(SPADE_SRC)/cpu/accumulator.spade $(SPADE_SRC)/cpu/alu.spade $(SPADE_DIR)/test/instruction6_model.hpp

define TEST_RULE
$(SPADE_BUILD)/obj_$(1)/V$(TEST_TOP_$(1)): $(SPADE_DIR)/test/$(1).cc $(TEST_SRCS_$(1)) $(TOOLCHAIN_INPUTS) | $(SPADE_BUILD)/.spade-build.stamp
	@cd $(SPADE_DIR) && verilator $(VERILATOR_FLAGS) \
		--top-module $(TEST_TOP_$(1)) \
		--Mdir build/obj_$(1) \
		build/spade.sv "$(abspath $(SPADE_DIR)/test/$(1).cc)"
	@test -x $$@

test_$(1): $(SPADE_BUILD)/obj_$(1)/V$(TEST_TOP_$(1))
	@cd $(SPADE_DIR) && ./build/obj_$(1)/V$(TEST_TOP_$(1))
endef

$(foreach name,$(TEST_NAMES),$(eval $(call TEST_RULE,$(name))))

# Der alte Alias für die Standard-ALU bleibt erhalten.
test_alu4_native: test_alu4

test_completed: $(addprefix test_,$(COMPLETED_TEST_NAMES))
# Bis zur naechsten TDD-Uebung sind alle Tests abgeschlossen.
test: test_completed
test_logic_basics: test

# CPU-v2-ISA vor der Hardwareumsetzung pruefen: eigenstaendiges C++-Modell.
# Keine Swim-/Spade- oder Verilator-Abhaengigkeiten, alte Test-Binaries
# werden dadurch nicht erneut kompiliert.
$(SPADE_BUILD)/cpu_v2_isa_demo: $(SPADE_DIR)/test/cpu_v2_isa_demo.cc
	@mkdir -p $(SPADE_BUILD)
	$(CXX) -std=c++17 -O2 -Wall -Wextra -pedantic $< -o $@

demo_cpu_v2: $(SPADE_BUILD)/cpu_v2_isa_demo
	@$(SPADE_BUILD)/cpu_v2_isa_demo
