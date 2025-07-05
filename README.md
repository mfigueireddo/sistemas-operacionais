# 🖥️ Sistemas Operacionais — Repositório de Projetos (2025.1)

Este repositório contém os arquivos e implementações desenvolvidas ao longo da disciplina de **Sistemas Operacionais**, incluindo laboratórios práticos e trabalhos de avaliação.

---

## 📁 Estrutura do Repositório

```
.
├── labs/        → Laboratórios práticos desenvolvidos em aula
├── t1/          → Trabalho 1: Simulação de gerenciamento de espaço aéreo
└── t2/          → Trabalho 2: Simulação de gerenciamento de memória virtual
```

---

## ✈️ Trabalho 1 — Gerenciamento de Espaço Aéreo

O primeiro trabalho simula o **gerenciamento de um espaço aéreo**, onde:

- Cada **processo** representa uma **aeronave**.
- O processo principal é responsável por gerenciar o funcionamento de todo o programa, incluindo:
  - Concessão de permissões de avanço/parada para evitar colisões.
  - Controle geral do estado do sistema.
- Há também uma **thread auxiliar** que permite ao usuário consultar o estado atual da simulação a qualquer momento.

Essa simulação visa demonstrar conceitos de concorrência, controle de processos e comunicação entre processos.

---

## 🧠 Trabalho 2 — Gerenciamento de Memória Virtual

O segundo trabalho simula o gerenciamento de **páginas na memória RAM** de um sistema operacional.

- `main.c` é responsável por:
  - Gerenciar todo o funcionamento do sistema.
  - Realizar o **escalonamento dos processos**.
  - Gerar arquivos com sequências de acesso a páginas para cada processo.

- `gmv.c` (Gerenciador de Memória Virtual):
  - Gerencia o uso da memória RAM.
  - Recebe requisições dos processos com:
    - O número da página.
    - A forma de acesso (leitura ou escrita).

- Cada processo possui sua **própria tabela de páginas**.

Foram implementados **4 algoritmos diferentes de substituição de páginas** para comparar a **eficiência** de cada um, usando os arquivos de acesso gerados pela `main`.

---

## 🛠️ Tecnologias Utilizadas

- Linguagem C
- POSIX Threads (pthread)
- Comunicação entre processos (pipes/signals)
- Manipulação de arquivos
- Simulação de escalonamento e gerenciamento de memória
