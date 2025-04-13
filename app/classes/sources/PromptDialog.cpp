#include "PromptDialog.h"
#include <QVBoxLayout>
#include <QLabel>

PromptDialog::PromptDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Session name setup");
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel("Please enter your player name:", this);
    inputField = new QLineEdit(this);
    QPushButton* okButton = new QPushButton("OK", this);

    layout->addWidget(label);
    layout->addWidget(inputField);
    layout->addWidget(okButton);

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    setLayout(layout);
}

QString PromptDialog::getText() const {
    return inputField->text();
}
