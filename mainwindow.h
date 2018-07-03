#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mainwindowviewmodel.h"
#include "menuvariables.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_envComboBox_currentTextChanged(const QString &arg1);

    void on_plannerComboBox_currentTextChanged(const QString &arg1);

    void on_start_x_editingFinished();

    void on_start_y_editingFinished();

    void on_goal_x_editingFinished();

    void on_goal_y_editingFinished();

    void on_rangeLineEdit_editingFinished();

    void on_goalBiasLineEdit_editingFinished();

private:
    void pointsEditingFinished();
    MenuVariables m_menu_variables;
    Ui::MainWindow *ui;
    MainWindowViewModel m_view_model;
};

#endif // MAINWINDOW_H
