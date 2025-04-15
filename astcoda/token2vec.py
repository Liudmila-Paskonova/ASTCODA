import argparse
import json
import os
from tqdm import tqdm
from gensim.models import Word2Vec
import logging

logging.basicConfig(
    format='%(asctime)s : %(levelname)s : %(message)s', level=logging.INFO)

parser = argparse.ArgumentParser(
    description="Train a Word2Vec model using parameters from a JSON config file."
)
parser.add_argument("config_path", type=str,
                    help="Path to the JSON configuration file")
args = parser.parse_args()

with open(args.config_path, "r") as config_file:
    config = json.load(config_file)

tokens_path = config.pop("tokens_path", None)
if tokens_path is None:
    raise ValueError(
        "The JSON configuration must contain a 'tokens_path' key.")

out_dir = config.pop("out_dir", ".")
model_filepath = os.path.join(out_dir, "token2vec.model")

if "embedding_dim" in config:
    config["vector_size"] = config.pop("embedding_dim")

token_lists = []
print("Start loading tokens...")
with open(tokens_path, "r") as file:
    for line in tqdm(file):
        tokens = line.strip().split()
        if len(tokens) > 1:
            token_lists.append(tokens[1:])
print("Finish loading tokens!")

model = Word2Vec(token_lists, **config)

os.makedirs(out_dir, exist_ok=True)
model.save(model_filepath)
print(f"Model saved to {model_filepath}")
