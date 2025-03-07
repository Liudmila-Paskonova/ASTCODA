# Coursework

**Pretrained embeddings**
- [C](https://disk.yandex.ru/d/PRbuA3DZ3Zg9Mg) (14.7 GB)
- [C++](https://disk.yandex.ru/d/foor9dUAI349Mg) (28.4 GB)

## Directory tree
``` bash
├── build # binaries and makefile data
├── c250k_model # folder with downloaded pretrained model
├── data # predictions
│  ├──  PROBLEM_NAME
│  │  ├── FILENAME
│  │  │  ├── FILENAME.contexts # extractor job
│  │  │  ├── FILENAME.contexts.num_examples # by default all C_n^2 contexts
│  │  │  ├── FILENAME.contexts.vectors # code2vec job
│  │  │  ├── FILENAME.contexts.vectors.clusters # clustering job
│  │  │  ├── FILENAME.html # result file to visualize
│  │  │  └── FILENAME.png # AST
│  │  └── ...
│  └── ...
├── examples # use to evaluate model
│  ├──  PROBLEM_NAME
│  │  ├── test # folder with files
│  │  │  ├── FILENAME.EXTENSION 
│  │  │  └── ...
│  │  ├── clusters.sav # trained kmeans model
│  │  ├── README.md # problem statement
│  │  └── solutions_pt.csv # info about pt clusters
│  └── ...
├── extractor # tool to extract contexts from files
│  ├── CMakeLists.txt
│  ├── extractor.h # base classes to extract contexts
│  ├── main.cpp
│  ├── threadPool.h # some parallel stuff
│  └── treeSitter.h # C++ binding on tree-sitter
├── frontend
│  ├──  ast # tool to draw syntax tree
│  │  ├── CMakeLists.txt
│  │  └── main.cpp
│  └── error_detection # tool to detect "error" clusters, generates html
│     ├── CMakeLists.txt
│     ├── highlight.h # several classes for html generation
│     ├── main.cpp
│     └── predict.py # generates csv files for visualizer
├── model # code2vec model files
├── tree-sitter # submodule
├── tree-sitter-c # submodule
├── .clang-format
├── .gitattributes
├── .gitmodules
├── build.sh # script to configure cmake
├── CMakeLists.txt
├── README.md
└── run.sh # script to predict error clusters
```

