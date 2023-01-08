all: gen_palette.exe gen_data.exe data

gen_palette.exe: gen_palette.cpp
	g++ $< -o $@ -lSDL2 -lSDL2_image

gen_data.exe: gen_data.cpp
	g++ $< -o $@ -lSDL2 -lSDL2_image

DATA_IN=$(shell find ../data/*.png)
DATA_OUT=$(DATA_IN:%.png=%.bin)
data: gen_data.exe $(DATA_OUT)

../data/%.bin: ../data/%.png
	gen_data.exe $< $@

.PHONY: data