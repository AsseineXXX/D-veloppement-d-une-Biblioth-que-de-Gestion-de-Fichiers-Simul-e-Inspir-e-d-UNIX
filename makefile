CC=gcc
CFLAGS=-I.
DEPS = BIBLIO_PROJET_OS.h
OBJ = BIBLIO_PROJET_OS.o main.o 
TARGET = projetos
DOXYGEN_CONFIG = Doxyfile
DOXYGEN_OUTPUT_DIR = doc

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
.PHONY: clean

clean:
	rm -f $(OBJ) $(TARGET)

	
.PHONY: doc
doc:
	mkdir -p $(DOXYGEN_OUTPUT_DIR)
	doxygen $(DOXYGEN_CONFIG)

.PHONY: clean-doc	
clean-doc:
	rm -rf $(DOXYGEN_OUTPUT_DIR)
