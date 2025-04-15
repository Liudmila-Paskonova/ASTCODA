import torch
from torch import nn
import torch.nn.functional as F


class ASTCODA(nn.Module):
    def __init__(self,
                 kernel_size,
                 embedding_dim,
                 num_filters,
                 padding_idx,
                 num_labels,
                 num_classes,
                 vocab_size,
                 dropout=0.5,
                 pretrained_embeddings=None,
                 pretrained_domains=None,
                 freeze_embedding=False,
                 freeze_domains=False):
        super(ASTCODA, self).__init__()

        self.kernel_size = kernel_size
        self.embedding_dim = embedding_dim
        self.num_filters = num_filters
        self.padding_idx = padding_idx
        self.num_labels = num_labels
        self.num_classes = num_classes
        self.vocab_size = vocab_size
        self.dropout = nn.Dropout(dropout)
        self.num_domains = num_labels // num_classes

        # Pads the input tensor boundaries with a constant value.
        # padding (int, tuple) = (kernel_size - 1, 0)
        self.pad = nn.ConstantPad1d(self.kernel_size - 1, self.padding_idx)

        # Embedding layer
        # [emb_dim, vocab_size]
        if pretrained_embeddings is not None:
            self.embedding = nn.Embedding.from_pretrained(pretrained_embeddings,
                                                          freeze=freeze_embedding)
        else:
            self.embedding = nn.Embedding(num_embeddings=self.vocab_size,
                                          embedding_dim=self.embedding_dim,
                                          padding_idx=self.padding_idx,
                                          max_norm=1.0)

        # 1-D Convolution
        # [emb_dim * kernel_size, num_filters]
        self.conv = nn.Conv1d(1, self.num_filters, self.embedding_dim * self.kernel_size,
                              stride=self.embedding_dim, bias=True)

        # Matrix that represents domains
        # [num_filters, num_domains]
        if pretrained_domains is not None:
            pretrained_domains = F.normalize(pretrained_domains, p=2, dim=1)
            self.attention_domains = nn.Embedding.from_pretrained(pretrained_domains,
                                                                  freeze=freeze_domains)
        else:
            self.attention_domains = nn.Embedding(num_embeddings=self.num_domains,
                                                  embedding_dim=self.num_filters,
                                                  padding_idx=self.padding_idx,
                                                  max_norm=1.0)
            nn.init.xavier_uniform_(self.attention_domains.weight)

        # List of fc-layers: one for each domain
        # [num_filters, num_classes]
        self.fcs = nn.ModuleList([
            nn.Linear(self.num_filters, self.num_classes)
            for _ in range(self.num_domains)
        ])

        # Batch normalization
        # [num_filters]
        self.bn_conv = nn.BatchNorm1d(self.num_filters)

    def forward(self, inputs):
        data, domain = inputs

        # Make all sentences equal in length
        # [batch_size, seq_len]
        x = self.pad(data)

        # Apply embedding layer to each word
        # [batch, seq_len, emb_dim]
        embedded_x = self.embedding(x).float()

        # Reshape
        # [batch_size, 1, seq_len * embedding_dim]
        embedded_x = embedded_x.view(-1, 1, x.shape[1] * self.embedding_dim)

        # Apply all convolution filters in parallel
        # [batch_size, num_filters, seq_len]
        features = self.conv(embedded_x)

        # BatchNorm
        # [batch_size, num_filters, seq_len]
        activations = self.bn_conv(features)

        # ReLU
        # [batch_size, num_filters, seq_len]
        activations = F.relu(activations)

        # Dropout
        # [batch_size, num_filters, seq_len]
        activations = self.dropout(activations)

        # Choose domains
        # [batch_size, num_filters]
        domains = self.attention_domains(domain)

        # Get attention weights
        # [batch_size, seq_len]
        attention_weights = torch.einsum('bf,bfs->bs', domains, activations)

        # Normalize
        # [batch_size, seq_len]
        attention_weights = F.softmax(attention_weights, dim=1)

        # Compute the weighted sum
        # [batch_size, num_filters]
        result = torch.einsum('bs,bfs->bf', attention_weights, activations)

        # Get logits
        # [batch_size, num_domains]
        logits = torch.stack([
            self.fcs[domain[i]](result[i])
            for i in range(result.shape[0])
        ])

        return {'features': features,
                'activations': activations,
                'domains': domains,
                'logits': logits}

    def get_embeddings(self):
        return self.embedding.weight

    def get_filters(self):
        return self.conv.weight.squeeze(), self.conv.bias

    def get_attention_domains(self):
        return self.attention_domains.weight

    def get_fc_weights(self):
        return [(fc.weight.data, fc.bias.data) for fc in self.fcs]
