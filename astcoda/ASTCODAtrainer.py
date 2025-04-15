from tqdm import tqdm
import torch
from torch import nn
from torch.optim import Adam
from sklearn.metrics import f1_score, precision_score, recall_score


class ASTCODAtrainer:
    def __init__(self, model, train_loader, valid_loader, test_loader, config):
        self.model = model
        self.train_loader = train_loader
        self.valid_loader = valid_loader
        self.test_loader = test_loader
        self.config = config
        self.device = torch.device(
            "cuda" if config.get("cuda", False) else "cpu")
        self.model.to(self.device)
        self.optimizer = Adam(self.model.parameters(), lr=config.get(
            "learning_rate", 1e-3), weight_decay=config.get("weight_decay", 1e-4))
        self.criterion = nn.CrossEntropyLoss()

    def _run_epoch(self, loader, desc="Evaluating"):
        epoch_loss = 0
        corrects = 0
        all_preds = []
        all_labels = []
        num_classes = len(self.config["domain_classes"])

        for batch in tqdm(loader, desc=desc):
            batch_x, batch_y = batch
            batch_x = torch.LongTensor(batch_x).to(self.device)
            batch_y = torch.LongTensor(batch_y).to(self.device)

            domain = batch_y // num_classes

            outputs = self.model((batch_x, domain))
            logits = outputs['logits']
            loss = self.criterion(logits, batch_y % num_classes)
            epoch_loss += loss.item()

            pred_labels = torch.max(logits, 1)[1]
            corrects += (pred_labels == (batch_y % num_classes)).sum().item()

            all_preds.extend(pred_labels.cpu().numpy())
            all_labels.extend((batch_y % num_classes).cpu().numpy())

        avg_loss = epoch_loss / len(loader)
        accuracy = corrects / (len(loader.dataset)) * 100
        f1 = f1_score(all_labels, all_preds, average="macro")
        precision = precision_score(all_labels, all_preds, average="macro")
        recall = recall_score(all_labels, all_preds, average="macro")

        return avg_loss, accuracy, f1, precision, recall

    def train_epoch(self):
        self.model.train()
        epoch_loss = 0
        corrects = 0
        all_preds = []
        all_labels = []
        num_classes = len(self.config["domain_classes"])

        for batch in tqdm(self.train_loader, desc="Training"):
            batch_x, batch_y = batch
            batch_x = torch.LongTensor(batch_x).to(self.device)
            batch_y = torch.LongTensor(batch_y).to(self.device)
            domain = batch_y // num_classes

            self.optimizer.zero_grad()
            outputs = self.model((batch_x, domain))
            logits = outputs['logits']
            loss = self.criterion(logits, batch_y % num_classes)
            loss.backward()
            self.optimizer.step()

            epoch_loss += loss.item()
            pred_labels = torch.max(logits, 1)[1]
            corrects += (pred_labels == (batch_y % num_classes)).sum().item()
            all_preds.extend(pred_labels.cpu().numpy())
            all_labels.extend((batch_y % num_classes).cpu().numpy())

        avg_loss = epoch_loss / len(self.train_loader)
        accuracy = corrects / (len(self.train_loader.dataset)) * 100
        f1 = f1_score(all_labels, all_preds, average="macro")
        precision = precision_score(all_labels, all_preds, average="macro")
        recall = recall_score(all_labels, all_preds, average="macro")

        return avg_loss, accuracy, f1, precision, recall

    def validate(self):
        self.model.eval()
        with torch.no_grad():
            return self._run_epoch(self.valid_loader, desc="Validating")

    def test(self):
        self.model.eval()
        with torch.no_grad():
            return self._run_epoch(self.test_loader, desc="Testing")
