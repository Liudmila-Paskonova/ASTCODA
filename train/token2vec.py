import argparse
from tqdm import tqdm
from gensim.models import Word2Vec
import logging
logging.basicConfig(
    format='%(asctime)s : %(levelname)s : %(message)s', level=logging.INFO)
parser = argparse.ArgumentParser(
    description="Process a file and extract lists of tokens.")
parser.add_argument("file_path", type=str, help="Path to tokens")

args = parser.parse_args()

file_path = args.file_path

token_lists = []

print("Start loading tokens...")
with open(file_path, "r") as file:
    for line in tqdm(file):
        tokens = line.strip().split()
        if len(tokens) > 1:
            token_lists.append(tokens[1:])
print("Finish loading tokens!")

model = Word2Vec(token_lists, vector_size=384, window=15, min_count=1,
                 epochs=10, hs=0, workers=60, sg=1, negative=15, sample=6e-5, seed=42)

model.save("token2vec.model")
