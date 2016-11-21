#! /usr/bin/env python3

from itertools import product
import sys

taxa_mut = [0.005, 0.010]
taxa_cruz = [0.95, 0.99]
tam_pop = [200, 400, 800]
max_iter_sem_melhoria = [10, 20]
taxa_pert = [0.5, 0.8]
operador_cruz = ['UM_PONTO', 'MULTIPLOS_PONTOS_2', 'MULTIPLOS_PONTOS_4',
                 'SEGMENTADO', 'UNIFORME']
metodo_selecao = ['TORNEIO_2', 'TORNEIO_4', 'ROLETA']
numero_geracoes = [500, 750, 1000]
numero_grupos = 10

combinacoes = list(product(taxa_mut, taxa_cruz, tam_pop, max_iter_sem_melhoria,
                           taxa_pert, operador_cruz, metodo_selecao,
                           numero_geracoes))
numero_combinacoes = len(combinacoes)
print('Numero de combinacoes: ', numero_combinacoes)

combinacoes_por_grupo = numero_combinacoes // numero_grupos
print('Combinacoes por grupo:', combinacoes_por_grupo)


def getid(c):
    return '.'.join(str(x) for x in c)


def comb2str(i, c):
    return str(i) + '-' + getid(c) + ' ' + ' '.join(str(x) for x in c)


id_grupo = int(sys.argv[1])
comeco_intervalo = id_grupo * combinacoes_por_grupo
fim_intervalo = (id_grupo + 1) * combinacoes_por_grupo

print('Intervalo: [%d, %d)' % (comeco_intervalo, fim_intervalo))

with open('restantes.txt', 'w') as f:
    for i, c in enumerate(combinacoes[comeco_intervalo:fim_intervalo],
                          comeco_intervalo):
        print(comb2str(i, c), file=f)

