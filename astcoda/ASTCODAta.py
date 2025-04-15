from tqdm import tqdm
import numpy as np
import torch
from torch.nn.utils.rnn import pad_sequence


class ASTCODAtaset:
    def __init__(self, tokens_file, labels_file, token_to_idx_file, label_to_idx_file):
        '''
            Build vocabularies
        '''
        self.tokens_list = []
        self.labels = []
        self.token_to_idx = {}
        self.label_to_idx = {}

        with open(tokens_file, "r", encoding="UTF-8") as f:
            for x in tqdm(f):
                x = x.replace('\x00', '').strip()
                self.tokens_list.append(x.split(u' '))
        with open(labels_file, "r", encoding="UTF-8") as f:
            for x in tqdm(f):
                x = x.replace('\x00', '').strip()
                self.labels.append(x.split('\n')[0])
        self.token_to_idx["@@PAD@@"] = 0
        self.token_to_idx["@@UNK@@"] = 1
        idx = 2
        with open(token_to_idx_file, "r", encoding="UTF-8") as f:
            for x in tqdm(f):
                x = x.replace('\x00', '').strip()
                self.token_to_idx[x.split('\n')[0]] = idx
                idx += 1
        idx = 0
        with open(label_to_idx_file, "r", encoding="UTF-8") as f:
            for x in tqdm(f):
                x = x.replace('\x00', '').strip()
                self.label_to_idx[x.split('\n')[0]] = idx
                idx += 1

    def __len__(self):
        '''
            Number of files in the dataset
        '''
        return len(self.tokens_list)

    def numericalize(self, tokens):
        '''
            Transform a sequence of AST-tokens to indices.
        '''
        indices = [self.token_to_idx.get(token) for token in tokens]
        return np.array(indices, dtype=np.int64)

    def __getitem__(self, idx):
        '''
            For a given idx get the tokens and the label for the file with this idx
        '''
        tokens = self.tokens_list[idx]
        label = self.labels[idx]
        label = self.label_to_idx.get(label)
        indices = self.numericalize(tokens)
        return torch.tensor(indices), torch.tensor(label, dtype=torch.int64)


class ASTCODAtaloader:
    def __init__(self, dataset, batch_size=1024, shuffle=True):
        self.dataset = dataset
        self.batch_size = batch_size
        self.shuffle = shuffle
        self.indices = np.arange(len(dataset))

    def __iter__(self):
        if self.shuffle:
            np.random.shuffle(self.indices)

        for start_idx in range(0, len(self.dataset), self.batch_size):
            batch_indices = self.indices[start_idx:start_idx + self.batch_size]
            batch_tokens, batch_labels = [], []
            for idx in batch_indices:
                token, label = self.dataset[idx]
                batch_tokens.append(token)
                batch_labels.append(label)

            batch_tokens = pad_sequence(
                batch_tokens, batch_first=True, padding_value=0)
            batch_labels = torch.stack(batch_labels)
            yield batch_tokens, batch_labels

    def __len__(self):
        return int(np.ceil(len(self.dataset) / self.batch_size))
