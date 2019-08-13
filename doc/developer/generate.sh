#!/bin/bash

rm -f blue-app-electroneum.pdf blue-app-electroneum.latex

pandoc -s --template=blue-app-electroneum.template -f rst+raw_tex -t latex --toc --toc-depth 4 -N -o blue-app-commands.pdf blue-app-commands.rst

#cp blue-app-electroneum.rst blue-app-electroneum.md-toc-depth
#pandoc -s --template=blue-app-electroneum.template  -t latex --toc -N -o blue-app-electroneum.md.pdf blue-app-electroneum.md

#rst2latex --latex-preamble='\usepackage{eufrak}' --input-encoding='utf8' blue-app-electroneum.rst blue-app-electroneum.latex;

#pdflatex -halt-on-error -output-format=pdf blue-app-electroneum.latex
