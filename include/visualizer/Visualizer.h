#ifndef VISUALIZER_VISUALIZER_H
#define VISUALIZER_VISUALIZER_H

#include <QApplication>
#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QStringList>
#include <QPalette>
#include <QLabel>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QMap>

namespace visualizer
{
/// Main class for the visualization
class CodeViewer : public QWidget
{
    Q_OBJECT

  private:
    QPlainTextEdit *textEdit;
    /// buttons
    QPushButton *nextButton;
    QPushButton *prevButton;
    /// header
    QLabel *headerLabel;
    /// files
    QStringList fileList;
    /// Idx to control buttons behaviour
    int currentIndex;
    QString folderPath;
    QMap<QString, QList<int>> lineMap;
    QMap<QString, QString> labelMap;

  private slots:
    /// "Next" button
    void loadNextFile();

    /// "Prev" button
    void loadPrevFile();

  private:
    /// A function that reads csv with line numbers
    /// @param linePath path to a csv
    void loadLines(const QString &linePath);

    /// A function that reads csv with labels
    /// @param labelsPath path to a csv
    void loadLabels(const QString &labelsPath);

    /// A function that does all the magic!
    /// @param fileName path to a file
    void loadFile(const QString &fileName);

    /// A function that highlights suspicious lines in the given file
    /// @param fileName path to a file
    void highlightLines(const QString &fileName);

  public:
    explicit CodeViewer(const QString &folderPath, const QString &linePath, const QString &labelsPath,
                        QWidget *parent = nullptr);
};

}; // namespace visualizer

#endif
