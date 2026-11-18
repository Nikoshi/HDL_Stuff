# Projekte im Repository-Root bauen und testen.

.DEFAULT_GOAL := help

VERILATOR_FLAGS := --quiet-build --quiet-stats --trace-vcd --cc --exe --build

.PHONY: help logic_basics test test_logic_basics test_mux test_half_adder

help:
	@printf 'Targets:\n  logic_basics       Spade-Code übersetzen\n  test_mux           Multiplexer testen\n  test_half_adder    Halbaddierer testen\n  test              Alle Tests ausführen\n  test_logic_basics  Alias für test\n'

logic_basics:
	@cd logic_basics && swim build

test_mux: TOP := main
test_mux: TEST := mux

test_half_adder: TOP := half_adder_top
test_half_adder: TEST := half_adder

test_mux test_half_adder: logic_basics
	@cd logic_basics && ln -sfn "$$(verilator --getenv VERILATOR_ROOT)/include" build/verilator_include
	@cd logic_basics && verilator $(VERILATOR_FLAGS) \
		--top-module $(TOP) \
		--Mdir build/obj_$(TEST) \
		build/spade.sv "$(abspath logic_basics/test/$(TEST).cc)" && \
		./build/obj_$(TEST)/V$(TOP)

test: test_mux test_half_adder

test_logic_basics: test
