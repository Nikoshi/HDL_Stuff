# Build Spade projects from the repository root.

.PHONY: help logic_basics

help:
	@printf 'Targets:\n  logic_basics  Build the logic_basics project\n'

logic_basics:
	cd logic_basics && swim build
