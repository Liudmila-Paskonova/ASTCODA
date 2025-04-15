# ASTCODA - Abstract Syntax Tree Convolutions Operating on Domain Attention

**ASTCODA (Abstract Syntax Tree Convolutions Operating on Domain Attention)** is a framework for automatic conditional feature extraction from source code grouped by domain.

## Requirements

**To build the project:**

- CMake >= 3.22
- C++ compiler to support C++23
- Eigen3 >= 3.4
- nlohmann_json
- Qt6

**To train models:**

- gensim
- tqdm
- numpy
- torch
- scikit-learn

## Directory tree

``` bash
├── astcoda # Files to train ASTCODA model
│  ├── ASTCODA.py # main model
│  ├── ASTCODAta.py # datasets and dataloaders
│  ├── ASTCODAtrainer.py # functions for training and evaluations
│  ├── token2vec.py # functions for training and evaluations
│  └── train.py # training unit
├── docs # Information and tutorials
├── include # Headers
│  ├── extractor # ThreadPool, TreeSitter...
│  ├── model  # Eigen3 ...
│  ├── support  # helper functions ...
│  ├── visualizer  # Qt6 ...
│  ├── vocab  # nlohmann_json...
│  └── CMakeLists.txt  
├── lib # Source files
│  ├── extractor # ThreadPool, TreeSitter...
│  ├── model  # Eigen3 ...
│  ├── support  # helper functions ...
│  ├── visualizer  # Qt6 ...
│  ├── vocab  # nlohmann_json...
│  └── CMakeLists.txt  
├── tools # Tools to run
│  ├── CMakeLists.txt 
│  ├── evaluate.cpp # generates the list of "suspicious" lines
│  ├── extract.cpp # extracts sequences of AST-tokens
│  ├── visualize.cpp # shows the retrieved "suspicious" lines in program code
│  └── vocabs.cpp # combine information about training, validation and test datasets
├── tree-sitter # Parser
├── tree-sitter-c 
├── ...
├── .clang-format
├── .gitattributes
├── .gitmodules
├── CMakeLists.txt
├── embeddings_preferences.json # Parameters for training embeddings
├── extractor_preferences.json # Extractor parameters
├── model_preferences.json # Parameters for training ASTCODA model
├── README.md
└── test_preferences.json # Parameters for evaluation
```

## Getting started

Clone and build the repository:

``` bash
git clone git@github.com:Liudmila-Paskonova/ASTCODA.git
mkdir build && cd build
cmake ..
make
cd ..
```

Start with [this step-by-step tutorial](docs/ai_example.md) to train a model that distinguishes between human-written and AI-generated submissions.

## Useful links

### Pretrained embeddings

- [C](https://disk.yandex.ru/d/PRbuA3DZ3Zg9Mg) (14.7 GB)
- [C++](https://disk.yandex.ru/d/foor9dUAI349Mg) (28.4 GB)

### Datasets

General structure:

``` bash
├── test # 10%
├── train # 70%
├── valid # 20%
└── labels.csv
```

Files are split in a 70-20-10 ratio for train, validation and test sets respectively. Each row of the main ```labels.csv``` file contains two (three) columns, separated by commas. The first one is a submission's filename, the second represents labels in format ```<domain>_<class>```, the optional third column defines the sequence of "suspicious" row numbers if present, separated by underscores, ```-1``` otherwise, e.g:

``` bash
FILENAME,IS_VULNERABLE,LINES
FormAI_1.c,0,-1
FormAI_10.c,1,31_35_79_80_81_88
...
```

All the datasets are balanced.

#### AI code detection

This dataset is a mixture of [FormAI dataset](https://github.com/FormAI-Dataset) (AI-generated submissions) and [SARD](https://samate.nist.gov/SARD/) (human-written programs).

**Classes:** ```AI```, ```Human```

**Number of domains:** 1

| Subset   | Number of files |
| -------- | --------------- |
| Train    | 269749          |
| Valid    | 77070           |
| Test     | 38535           |

[Download](https://disk.yandex.ru/d/On0Ib68cZ41GPQ)

#### Vulnerability detection

This dataset is created from both versions of the [FormAI dataset](https://github.com/FormAI-Dataset) and is intended for vulnerability detection.

**Classes:** ```Vulnerable```, ```Good```

**Number of domains:** 1

| Subset   | Number of files |
| -------- | --------------- |
| Train    | 235566          |
| Valid    | 67304           |
| Test     | 33653           |

[Download](https://disk.yandex.ru/d/yZh7f34CrzKr3g)

#### Error localization in student programming submissions

This dataset is consisted of 500'000 submissions to 30 problems from [Project CodeNet](https://github.com/IBM/Project_CodeNet).

**Classes:** ```OK``` (AC), ```PT``` (WA, TLE, MLE, OLE, RE, PE)

**Number of domains:** 30

| Subset   | Number of files |
| -------- | --------------- |
| Train    | 314965          |
| Valid    | 89990           |
| Test     | 44995           |

[Download](https://disk.yandex.ru/d/XsKXpg30JqeM3g)

### Pretrained models

#### AI code detection

| Parameter             |Value   |
| ----------------------| ------ |
| pretrained embeddings | None   |
| embeddings_dim        | 384    |
| kernel_size           | 15     |
| num_filters           | 128    |
| dropout               | 0.5    |
| weight_decay          | 0.0001 |
| learning_rate         | 0.0001 |
| minlen                | 1      |

[Download model]()

[Download weights]()

#### Vulnerability detection

| Parameter             |Value   |
| ----------------------| ------ |
| pretrained embeddings | None   |
| embeddings_dim        | 384    |
| kernel_size           | 15     |
| num_filters           | 128    |
| dropout               | 0.5    |
| weight_decay          | 0.0001 |
| learning_rate         | 0.0001 |
| minlen                | 1      |

[Download model](https://disk.yandex.ru/d/R2fwuLRxitKySQ)

[Download weights](https://disk.yandex.ru/d/fmGRctk7Kd-F7w)

#### Error localization in student programming submissions

| Parameter             |Value   |
| ----------------------| ------ |
| pretrained embeddings | None   |
| embeddings_dim        | 384    |
| kernel_size           | 15     |
| num_filters           | 128    |
| dropout               | 0.5    |
| weight_decay          | 0.0001 |
| learning_rate         | 5e-05  |
| minlen                | 5      |

[Download model](https://disk.yandex.ru/d/PEfnCQSSjzG42A)

[Download weights](https://disk.yandex.ru/d/hPsUUTbfnYgf6Q)