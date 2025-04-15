# Train embeddings

## Train from scratch

Edit ```embeddings_preferences.json```:

``` json
{
    "tokens_path": "example/train/tokens.txt",
    "embedding_dim": 384,
    "window": 15,
    "min_count": 1,
    "epochs": 10,
    "hs": 0,
    "workers": 1,
    "sg": 1,
    "negative": 15,
    "sample": 0.00006,
    "seed": 42,
    "out_dir": "example/token2vec"
}
```

and run:

``` bash
python3 astcoda/token2vec.py embeddings_preferences.json
```

## Use pretrained embeddings

[Download pretrained embeddings](../README.md#pretrained-embeddings), unzip them and modify the ```embeddings``` section in ```model_preferences.json```, e.g.

``` json
{ 
 "embeddings": "embeddings_c.bin"
}
```
