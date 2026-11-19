# Projekte im Repository-Root bauen und testen.

.DEFAULT_GOAL := help

VERILATOR_FLAGS := --quiet-build --quiet-stats --trace-vcd --cc --exe --build

.PHONY: help logic_basics test test_logic_basics test_mux test_half_adder test_full_adder test_add2 test_add4

help:
	@printf 'Targets:\n  logic_basics       Spade-Code übersetzen\n  test_mux           Multiplexer testen\n  test_half_adder    Halbaddierer testen\n  test_full_adder    Volladdierer testen\n  test_add2          2-Bit-Addierer testen\n  test_add4          4-Bit-Addierer testen\n  test              Alle Tests ausführen\n  test_logic_basics  Alias für test\n'

logic_basics:
	@cd logic_basics && swim build

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

test_mux test_half_adder test_full_adder test_add2 test_add4: logic_basics
	@cd logic_basics && ln -sfn "$$(verilator --getenv VERILATOR_ROOT)/include" build/verilator_include
	@cd logic_basics && verilator $(VERILATOR_FLAGS) \
		--top-module $(TOP) \
		--Mdir build/obj_$(TEST) \
		build/spade.sv "$(abspath logic_basics/test/$(TEST).cc)" && \
		./build/obj_$(TEST)/V$(TOP)

test: test_mux test_half_adder test_full_adder test_add2 test_add4

test_logic_basics: test
