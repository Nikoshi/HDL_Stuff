# Build and test Spade projects from the repository root.

.DEFAULT_GOAL := help

test_mux: TOP = main
test_mux: TEST = mux

test_half_adder: TOP = half_adder_top
test_half_adder: TEST = half_adder

.PHONY: help logic_basics test_logic_basics

help:
	@printf 'Targets:\n  logic_basics       Build logic_basics\n  test_logic_basics  Run the Verilator testbench\n'

logic_basics:
	@cd logic_basics && swim build

.PHONY: test_mux test_half_adder

test_mux test_half_adder: logic_basics
	@cd logic_basics && verilator --quiet-build --quiet-stats --trace-vcd --cc --exe --build \
		--top-module $(TOP) \
		--Mdir build/obj_$(TEST) \
		build/spade.sv "$(abspath logic_basics/test/$(TEST).cc)" && \
		./build/obj_$(TEST)/V$(TOP)