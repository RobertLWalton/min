min:
	latex min; makeindex min
min-design:
	latex min-design; makeindex min-design
min-design.ps:
	rm -f min-design.ps
	latex min-design
	makeindex min-design
	latex min-design
	makeindex min-design
	latex min-design
	makeindex min-design
	dvips -O '0in,1in' -o min-design.ps min-design.dvi
min-design.pdf:		min-design.ps
	rm -f min-design.pdf
	ps2pdf min-design.ps min-design.pdf
min-lexeme:
	latex min-lexeme; makeindex min-lexeme
