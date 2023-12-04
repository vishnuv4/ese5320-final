ENCODER_SOURCES = testbench.cpp sha256.cpp cdc.cpp decoder.cpp dedup.cpp main.h sha256.h lzw.cpp server.cpp server.h
COMPILER = g++
OUTPUT = executable
INPUT_FILE = Franklin.txt
CODED_FILE = en_out.txt
DECODED_FILE = decode_out.txt

.PHONY: run
run: all encode

.PHONY: all
all: $(ENCODER_SOURCES) $(DECODER_SOURCES)
	$(COMPILER) -o $(OUTPUT) $(ENCODER_SOURCES)

.PHONY: debug
debug: $(ENCODER_SOURCES) $(DECODER_SOURCES)
	$(COMPILER) -g -Wall -o $(OUTPUT) $(ENCODER_SOURCES)

.PHONY: clean
clean:
	rm $(OUTPUT) $(CODED_FILE) $(DECODED_FILE)

.PHONY: encode
encode:
	./$(OUTPUT)

.PHONY: diff
diff:
	diff --text $(DECODED_FILE) $(INPUT_FILE) 

.PHONY: help
help:
	@echo List of commands:
	@echo all
	@echo clean
	@echo encode
	@echo decode
	@echo diff

.PHONY: full
full: all encode decode diff
