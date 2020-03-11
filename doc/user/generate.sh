#!/bin/bash

rm -f bolos-app-electroneum.pdf bolos-app-electroneum.latex
# To get this to work install:
# sudo apt-get install pandoc texlive-latex-base texlive-fonts-recommended texlive-latex-recommended
pandoc -s --variable graphics --template=bolos-app-electroneum.template -f rst+raw_tex+line_blocks+citations -t latex --toc -N -o bolos-app-electroneum.pdf bolos-app-electroneum.rst
