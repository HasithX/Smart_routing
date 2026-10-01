.PHONY: all run test profile clean help install

PYTHON := python3

all: run

help:
	@echo "Smart City Public Transit System - Commands"
	@echo "---------------------------------------------"
	@echo "make install   : Install project dependencies"
	@echo "make run       : Run the main interactive CLI application"
	@echo "make test      : Run unit tests"
	@echo "make profile   : Run cProfile & algorithmic performance benchmarks"
	@echo "make clean     : Remove temporary files and cache"

install:
	$(PYTHON) -m pip install -r requirements.txt --break-system-packages

run:
	$(PYTHON) main.py

test:
	$(PYTHON) -m unittest discover -s tests -p "test_*.py"

profile:
	$(PYTHON) -m src.profiling.profiler

clean:
	find . -type d -name "__pycache__" -exec rm -rf {} +
	find . -type f -name "*.pyc" -delete
	find . -type f -name "*.prof" -delete
