#! /usr/bin/env python3

from itertools import product

taxa_mut = [0.005, 0.010]
taxa_cruz = [0.95, 0.99]
tam_pop = [200, 400, 800]
max_iter_sem_melhoria = [10, 20]
taxa_pert = [0.2, 0.5, 0.8]
operador_cruz = ['UM_PONTO', 'MULTIPLOS_PONTOS_2', 'MULTIPLOS_PONTOS_4',
                 'SEGMENTADO', 'UNIFORME']
metodo_selecao = ['TORNEIO_2', 'TORNEIO_4', 'ROLETA']

combinacoes = list(product(taxa_mut, taxa_cruz, tam_pop, max_iter_sem_melhoria,
                           taxa_pert, operador_cruz, metodo_selecao))
print('Numero de combinacoes: ', len(combinacoes))


def getid(c):
    return '.'.join(str(x) for x in c)


def comb2str(i, c):
    return str(i) + '-' + getid(c) + ' ' + ' '.join(str(x) for x in c)


with open('restantes.txt', 'w') as f:
    for i, c in enumerate(combinacoes):
        print(comb2str(i, c), file=f)

