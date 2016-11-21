#! /usr/bin/bash

rm finalizados.txt
rm nohup.out
rm restantes.txt
rm -rf resultados/
make
python3 scripts/gerar_configuracoes.py "$1"
python3 scripts/executa_experimentos.py build/robby
echo $! > pid