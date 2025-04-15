#include "visualizer/Visualizer.h"

void
visualizer::CodeViewer::loadNextFile()
{
    currentIndex = (currentIndex + 1) % fileList.size();
    loadFile(fileList[currentIndex]);
}

void
visualizer::CodeViewer::loadPrevFile()
{
    currentIndex = (currentIndex - 1 + fileList.size()) % fileList.size();
    loadFile(fileList[currentIndex]);
}

void
visualizer::CodeViewer::loadLines(const QString &linePath)
{
    QFile file(linePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Warning", "Could not open labels file: " + linePath);
        return;
    }

    QTextStream in(&file);
    QString line;
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList parts = line.split(' ');
        if (parts.size() < 2) {
            continue;
        }

        QString fileName = parts[0];

        QList<int> lineNumbers;
        for (int i = 1; i < parts.size(); ++i) {
            int lnum = parts[i].toInt();
            lineNumbers.append(lnum);
        }

        lineMap[fileName] = lineNumbers;
    }
    file.close();
}

void
visualizer::CodeViewer::loadLabels(const QString &labelsPath)
{
    // Read the labels file and parse the data
    QFile file(labelsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Warning", "Could not open labels file: " + labelsPath);
        return;
    }

    QTextStream in(&file);
    QString line;
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() < 2) {
            continue;
        }

        QString fileName = parts[0];
        QString label = parts[1];

        labelMap[fileName] = label;
    }
    file.close();
}

void
visualizer::CodeViewer::loadFile(const QString &fileName)
{
    // Correct the header
    auto parts = labelMap[fileName].split('_');
    auto domain = parts[0];
    auto status = parts[1];
    headerLabel->setText("File: " + fileName + ", domain: " + domain + ", status: " + status);

    QDir directory(folderPath);
    QString fullPath = directory.absoluteFilePath(fileName);

    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Warning", "Could not open " + fullPath);
        return;
    }

    // Read the file
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    textEdit->setPlainText(content);
    setWindowTitle(fileName);

    // Highlight lines based on the filename info
    highlightLines(fileName);
}

void
visualizer::CodeViewer::highlightLines(const QString &fileName)
{

    if (!lineMap.contains(fileName)) {
        return;
    }

    QList<int> linesToHighlight = lineMap[fileName];

    // if there're no erroneous lines -> do nothing
    if (linesToHighlight.contains(0)) {
        return;
    }

    QTextCursor cursor = textEdit->textCursor();
    QTextCharFormat format;
    format.setBackground(Qt::red);

    // Highlight lines line by line
    int lineNumber = 1; // 1-based index
    cursor.movePosition(QTextCursor::Start);
    while (cursor.movePosition(QTextCursor::Down)) {
        if (linesToHighlight.contains(lineNumber)) {
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor.setCharFormat(format);
        }
        lineNumber++;
    }
}

visualizer::CodeViewer::CodeViewer(const QString &folderPath, const QString &linePath, const QString &labelsPath,
                                   QWidget *parent)
    : QWidget(parent), currentIndex(0), folderPath(folderPath)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Header
    headerLabel = new QLabel("", this);
    QFont headerFont = headerLabel->font();
    headerFont.setBold(true);
    headerFont.setPointSize(12);
    headerLabel->setFont(headerFont);
    layout->addWidget(headerLabel);

    // Text viewer
    textEdit = new QPlainTextEdit(this);
    textEdit->setReadOnly(true);

    QPalette palette = textEdit->palette();
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Text, Qt::black);
    textEdit->setPalette(palette);

    layout->addWidget(textEdit);

    // Buttons layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    prevButton = new QPushButton("Prev", this);
    nextButton = new QPushButton("Next", this);

    prevButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    nextButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(nextButton);

    layout->addLayout(buttonLayout);

    // Connect buttons
    connect(prevButton, &QPushButton::clicked, this, &CodeViewer::loadPrevFile);
    connect(nextButton, &QPushButton::clicked, this, &CodeViewer::loadNextFile);

    // Load lines and labels
    loadLines(linePath);
    loadLabels(labelsPath);

    QDir directory(folderPath);
    fileList = directory.entryList(QDir::Files, QDir::Name);
    if (fileList.isEmpty()) {
        QMessageBox::critical(this, "Error", "No files found in the folder: " + folderPath);
        nextButton->setEnabled(false);
        return;
    }

    loadFile(fileList[currentIndex]);
}
