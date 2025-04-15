from ASTCODA import ASTCODA
from ASTCODAta import ASTCODAtaloader, ASTCODAtaset
from ASTCODAtrainer import ASTCODAtrainer

import argparse
import json
from gensim.models import KeyedVectors
import numpy as np
from sklearn.preprocessing import normalize
import torch
from tqdm import tqdm
import os


def save_embeddings(model, word2idx, file_path):
    embeddings = model.get_embeddings().detach().cpu().numpy().astype(np.float32)
    vocab_size = len(word2idx)
    emb_dim = embeddings.shape[1]

    with open(file_path, 'wb') as f:
        header = f"{vocab_size} {emb_dim}\n"
        f.write(header.encode('utf-8'))

        for word, idx in word2idx.items():
            word_part = word.encode('utf-8') + b' '
            f.write(word_part)
            f.write(embeddings[idx].tobytes())


def save_mat(file_path, tens):
    with open(file_path, "wb") as f:
        f.write(tens.detach().cpu().numpy().T.tobytes())


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Load config from a JSON file.")
    parser.add_argument("config_path", type=str,
                        help="Path to the config JSON file")
    args = parser.parse_args()

    with open(args.config_path, 'r') as fp:
        config = json.load(fp)

    print("Reading datasets...")
    # Create datasets
    train_dataset = ASTCODAtaset(
        config["train_x"], config["train_y"], config["token_to_idx"], config["label_to_idx"])
    valid_dataset = ASTCODAtaset(
        config["valid_x"], config["valid_y"], config["token_to_idx"], config["label_to_idx"])
    test_dataset = ASTCODAtaset(
        config["test_x"], config["test_y"], config["token_to_idx"], config["label_to_idx"])

    # Create dataloaders
    train_loader = ASTCODAtaloader(
        train_dataset, batch_size=config["batch_size"], shuffle=True)
    val_loader = ASTCODAtaloader(
        valid_dataset, batch_size=config["batch_size"], shuffle=False)
    test_loader = ASTCODAtaloader(test_dataset, batch_size=1, shuffle=False)

    print("Embeddings...", end=' ')
    # Add embeddings if needed
    if config["embeddings"] != "":
        print("yes")
        w2v = KeyedVectors.load_word2vec_format(
            config["embeddings"], binary=True)
        vocab_size = len(train_dataset.token_to_idx)
        embeddings = np.zeros((vocab_size, config["embedding_dim"]))
        embeddings[train_dataset.token_to_idx['@@UNK@@']] = normalize(
            np.random.uniform(-0.25, 0.25, (1, config["embedding_dim"])), norm='l2', axis=1)
        embeddings[train_dataset.token_to_idx['@@PAD@@']
                   ] = np.zeros((1, config["embedding_dim"]))
        for word, idx in tqdm(train_dataset.token_to_idx.items()):
            if word in w2v:
                embeddings[idx] = w2v[word]
        embeddings = torch.tensor(embeddings, dtype=torch.float64)

    else:
        print("no")

    # Set device
    print("Device...", end=" ")
    if (config["cuda"] == True):
        print("cuda:0")
        torch.cuda.set_device(0)
    else:
        print("cpu")

    model_name = f"model_k{config['kernel_size']}_nf{config['num_filters']}_e{config['embedding_dim']}"
    concrete_model_path = config["model_path"] + "/" + model_name
    log_path = concrete_model_path + "/logs"
    weights_path = concrete_model_path + "/weights"

    if not os.path.exists(config["model_path"]):
        os.makedirs(config["model_path"])
    if not os.path.exists(concrete_model_path):
        os.makedirs(concrete_model_path)
    if not os.path.exists(log_path):
        os.makedirs(log_path)
    if not os.path.exists(weights_path):
        os.makedirs(weights_path)

    print("Model:")
    model = ASTCODA(kernel_size=config["kernel_size"],
                    embedding_dim=config["embedding_dim"],
                    num_filters=config["num_filters"],
                    padding_idx=0,
                    num_labels=len(train_dataset.label_to_idx),
                    num_classes=len(config["domain_classes"]),
                    vocab_size=len(train_dataset.token_to_idx))
    print(model)
    with open(concrete_model_path + "/config.json", "w") as fp:
        json.dump(config, fp, indent=4)

    print("Training...")
    # Train
    trainer = ASTCODAtrainer(
        model, train_loader, val_loader, test_loader, config)

    print("\t".join(["Epoch", "TrLoss", "TrAcc", "TrF1", "TrPrec",
          "TrRec", "EvLoss", "EvAcc", "EvF1", "EvPrec", "EvRec", "BestEvAcc"]))

    metrics = {"train_loss": [], "train_acc": [], "train_f1": [], "train_prec": [], "train_rec": [
    ], "eval_loss": [], "eval_acc": [], "eval_f1": [], "eval_prec": [], "eval_rec": [], "best": -1}
    best_val_loss = float('inf')
    # early stopping
    patience = 3
    patience_counter = 0
    for epoch in range(config["num_epochs"]):
        train_loss, train_acc, train_f1, train_prec, train_rec = trainer.train_epoch()
        eval_loss, eval_acc, eval_f1, eval_prec, eval_rec = trainer.validate()
        metrics["train_loss"].append(train_loss)
        metrics["train_acc"].append(train_acc)
        metrics["train_f1"].append(train_f1)
        metrics["train_prec"].append(train_prec)
        metrics["train_rec"].append(train_rec)

        metrics["eval_loss"].append(eval_loss)
        metrics["eval_acc"].append(eval_acc)
        metrics["eval_f1"].append(eval_f1)
        metrics["eval_prec"].append(eval_prec)
        metrics["eval_rec"].append(eval_rec)

        if eval_acc > metrics["best"]:
            metrics["best"] = eval_acc
            torch.save(model, concrete_model_path + "/model.pt")

        if eval_loss < best_val_loss:
            best_val_loss = eval_loss
            patience_counter = 0
        else:
            patience_counter += 1

        print(
            f"{epoch}\t{train_loss:.5f}\t{train_acc:.2f}\t{train_f1:.2f}\t{train_prec:.2f}\t{train_rec:.2f}\t{eval_loss:.5f}\t{eval_acc:.2f}\t{eval_f1:.2f}\t{eval_prec:.2f}\t{eval_rec:.2f}\t{metrics['best']:.2f}")

        if (patience_counter >= patience):
            print("Early stopping triggered")
            break

    with open(log_path + "/metrics.json", "w") as fp:
        json.dump(metrics, fp, indent=4)

    print("Testing...")

    print("\t".join(["Test Acc", "Test F1", "Test Precision", "Test Recall"]))
    loss, acc, f1, prec, rec = trainer.test()
    print(f"{acc:.4f}\t{f1:.4f}\t{prec:.4f}\t{rec:.4f}")

    print("Saving weights...")
    # conv
    # [num_filters, kernel_size * emb_dim]
    # [num_filters]
    convs, bias = model.get_filters()
    # domains
    # [num_domains, num_filters]
    at_dom = model.get_attention_domains()
    # embeddings
    # [vocab_size, emb_dim]
    emb = model.get_embeddings()
    # bn_conv
    # [num_filters]
    alpha = model.bn_conv.weight
    # [num_filters]
    beta = model.bn_conv.bias
    # [num_filters]
    mean = model.bn_conv.running_mean
    # [num_filters]
    var = model.bn_conv.running_var
    # fc layer
    lst = model.get_fc_weights()
    w_lst = []
    b_lst = []
    for elem in lst:
        w, b = elem
        w_lst.append(w)
        b_lst.append(b)
    # [num_domains * num_classes, num_filters]
    fc_mat = torch.cat(w_lst, dim=0)
    # [num_domains * num_classes]
    fc_bias = torch.cat(b_lst, dim=0)

    save_embeddings(model, train_dataset.token_to_idx,
                    weights_path + "/embeddings.bin")

    save_mat(f"{weights_path}/conv_matrix.bin", convs)
    save_mat(f"{weights_path}/conv_bias.bin", bias)
    save_mat(f"{weights_path}/attention_domains.bin", at_dom)
    save_mat(f"{weights_path}/bn_alpha.bin", alpha)
    save_mat(f"{weights_path}/bn_beta.bin", beta)
    save_mat(f"{weights_path}/bn_mean.bin", mean)
    save_mat(f"{weights_path}/bn_var.bin", var)
    save_mat(f"{weights_path}/fc_matrices.bin", fc_mat)
    save_mat(f"{weights_path}/fc_biases.bin", fc_bias)
