# Coursework

## Requirements

**To build the project:**

- CMake >= 3.22
- C++ compiler to support C++23
- Eigen3 >= 3.4
- nlohmann_json

**To train models:**

- gensim
- tqdm
- numpy
- torch
- scikit-learn

## Getting started
Let's create a tool to detect vulnerabilities!

1. **Download and unzip pretrained embeddings and the following vulnerability dataset**

You may find the detailed information about these archives below in the corresponding section.
- [C pretrained embeddings](https://disk.yandex.ru/d/PRbuA3DZ3Zg9Mg)
- [the dataset](https://disk.yandex.ru/d/ZAr8GZPQEBo5zg)
  
Now you have the following folders:
``` bash
│  ...
├── VULNERABILITY_DETECTION
│  ├──  test
│  │  └── ...
│  └──  train
│  │  └── ...
│  ├──  valid
│  │  └── ...
│  └── labels.csv
│  ...
└── embeddings_c.bin
│  ...
```
2. **Extract path-tokens from the train, test and validation subsets**

Edit the ```extractor_preferences.json```

For example, 
- here the *path-token* is a path from the root to a leaf of an AST (```root_terminal``` option),
- ```masked_identifiers``` means that all the terminals-```identifiers``` are anonymised,
- ```ids_hash``` implies that all the submissions are represented in the format ```<node>_<node>_ ... _<node>_<terminal>```.
  
Besides, submissions with <= ```2000``` path-tokens are not taken into consideration.

``` json
{
    "threads": 1, # to run the extractor in multithreaded mode
    "minlen": 1, # minimum number of nodes that constitutes the path-token
    "maxsize": 2000, # maximum number of path-tokens within a submission
    "lang": "c", # submission's language
    "dir": "VULNERABILITY_DETECTION/test", # folder with source code
    "traversal": "root_terminal", # how to traverse AST
    "token": "masked_identifiers", # how tokens are represented
    "split": "ids_hash", # how to represent one submission
    "outdir": "data", # output folder. It will be created if not exist
    "mapping": "VULNERABILITY_DETECTION/labels.csv" # path to the file with labels
}
```
After you set these values, run:

``` bash
./build/bin/extract extractor_preferences.json
```

Now the newly created ```data``` folder must look like this:

``` bash
│  ...
├──  data
│  ├── test
│  │  ├── labels.txt
│  │  ├── mapping.json
│  │  ├── submissions.txt
│  │  └── tokens.txt
│  ├── train
│  │  ├── labels.txt
│  │  ├── mapping.json
│  │  ├── submissions.txt
│  │  └── tokens.txt
│  └── valid
│     ├── labels.txt
│     ├── mapping.json
│     ├── submissions.txt
│     └── tokens.txt
│  ...
```

- ```labels.txt``` is a file where each row is a line in a format ```<domain>_<class>```,
- ```mapping.json``` is a vocabulary hash-terminal,
- ```submissions.txt``` stores the list of submissions,
- in the ```tokens.txt``` there're sequences of path-tokens in the predefined ```split``` format.

``` bash
# labels.txt
0_1
0_0

# mapping.json
{
"9999449717833555151": "Successfully wrote %ld bytes to %s",
"9999579624554016231": "Total errors: %d",
...
}

# submissions.txt
gemini_pro-9350.c
falcon180b-29416.c

# tokens.txt
219_222_2_5464660988692016868 219_222_172_17625822189638355265 219_222_2_5464660988692016868 ...
```

3. **Create mutual vocabularies**

Change the ```model_preferences.json```. Values required at this stage:

```json
{
    "train_x": "data/train/tokens.txt", # path to train tokens
    "test_x": "data/test/tokens.txt", # path to test tokens
    "valid_x": "data/valid/tokens.txt", # path to validation tokens
    "train_y": "data/train/labels.txt", # path to train labels
    "test_y": "data/test/labels.txt", # path to test labels
    "valid_y": "data/valid/labels.txt", # path to validation labels
    "train_vocab": "data/train/mapping.json", # path to the train hash-to-terminal vocabulary
    "test_vocab": "data/test/mapping.json", # path to the test hash-to-terminal vocabulary
    "valid_vocab": "data/valid/mapping.json", # path to the validation hash-to-terminal vocabulary
    "train_sub": "data/train/submissions.txt", # path to train filenames
    "test_sub": "data/test/submissions.txt", # path to test filenames
    "valid_sub": "data/valid/submissions.txt", # path to validation filenames
    "token_to_idx": "data/token_to_idx.txt",  # path to the future token-to-idx vocabulary
    "label_to_idx": "data/label_to_idx.txt",  # path to the future label-to-idx vocabulary
    "domain_to_idx": "data/domain_to_idx.txt",  # path to the future domain-to-idx vocabulary
    "hash_to_terminal": "data/hash_to_terminal.json",  # path to the future common hash-to-terminal vocabulary
      ...
}
```

Then run:
``` bash
./build/bin/vocab model_preferences.json
```

Now you should see:
``` bash
│  ...
├──  data
│  ├── test
│  │  └── ...
│  ├── train
│  │  └── ...
│  ├── valid
│  │  └── ...
│  ├── domain_to_idx.txt
│  ├── hash_to_terminal.json
│  ├── label_to_idx.txt
│  └── token_to_idx.txt
│  ...
```

- Each line of the ```domain_to_idx.txt```/```labels_to_idx.txt```/```token_to_idx.txt``` file stores a unique domain/ label/ path-token respectively
- ```hash_to_terminal.json``` is a union of the previous train, test and validation vocabularies

## Useful links

### Pretrained embeddings

- [C](https://disk.yandex.ru/d/PRbuA3DZ3Zg9Mg) (14.7 GB)
- [C++](https://disk.yandex.ru/d/foor9dUAI349Mg) (28.4 GB)

### Datasets

```bash
├── test # 10%
├── train # 80%
├── valid # 10%
└── labels.csv
```

#### AI detection

[download](https://disk.yandex.ru/d/1a98iHPB-RB0kA)

| Subset   | Number of files |
| -------- | --------------- |
| Train    | 500283          |
| Valid    | 62535           |
| Test     | 62536           |

#### Vulnerability detection

This dataset is created from both versions of the [FormAI dataset](https://github.com/FormAI-Dataset) and is intended for vulnerability detection. The files are split in a 80-10-10 ratio for train, validation and test sets respectively. Each row of the main ```labels.csv``` file contains three columns, separated by commas. ```FILENAME``` is the filename of the submission, ```IS_VULNERABLE``` represents the submission status, ```LINES``` column defines the sequence of row numbers if present, separated by underscores, ```-1``` otherwise:

```
FILENAME,IS_VULNERABLE,LINES
FormAI_1.c,0,-1
FormAI_10.c,1,31_35_79_80_81_88
...
```
[Download](https://disk.yandex.ru/d/ZAr8GZPQEBo5zg)

| Subset   | Number of files |
| -------- | --------------- |
| Train    | 333300          |
| Valid    | 41662           |
| Test     | 41663           |

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

