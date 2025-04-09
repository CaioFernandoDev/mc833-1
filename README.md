# Servidor e Cliente TCP

Projeto 1 da disciplina MC833.

## Compilação

Para compilar o projeto, basta executar o comando:

```bash
make
```

## Execução

Para executar o servidor, basta executar o comando em um terminal:

```bash
bin/servidor
```

Para executar o cliente, basta executar o comando em um novo terminal:

```bash
bin/cliente
```

## Definição do IP

O cliente irá solicitar o IP da máquina que está rodando o servidor. Por padrão, deixamos o 127.0.0.1, para o cenário na mesma máquina. Para testar em máquinas diferentes na mesma rede, usamos o comando para descobrir o IP do servidor:

```bash
hostname -I
```
