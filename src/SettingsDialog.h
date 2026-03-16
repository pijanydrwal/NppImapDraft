// SettingsDialog.h
#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QString>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

private slots:
    void on_applyButton_clicked();
};

#endif // SETTINGS_DIALOG_H