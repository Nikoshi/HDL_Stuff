# Build and test Spade projects from the repository root.

.DEFAULT_GOAL := help
.PHONY: help logic_basics test_logic_basics

help:
	@printf 'Targets:\n  logic_basics       Build logic_basics\n  test_logic_basics  Run the Verilator testbench\n'

logic_basics:
	@cd logic_basics && swim build

test_logic_basics: logic_basics
	@cd logic_basics && ln -sfn "$$(verilator --getenv VERILATOR_ROOT)/include" build/verilator_include
	@cd logic_basics && verilator --quiet-build --quiet-stats --trace-vcd --cc --exe --build \
		--top-module main \
		--Mdir build/obj_mux \
		build/spade.sv "$(abspath logic_basics/test/mux.cc)" && \
		./build/obj_mux/Vmain
