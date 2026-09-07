#include "MainWindow.hpp"
#include "RunCommand.hpp"
#include "ui_MainWindow.h"
#include <cstdlib>
#include <QFileDialog>
#include <QThread>
#include <QScrollBar>
#include <QIcon>

// ------ GLOBAL ------

// void MainWindow::printProgress(std::string id, int percent, std::string speed) {
void printProgress(MainWindow* state, std::string id, int percent, std::string speed) {
    state->ui->progress_bar->setValue(percent);
    // std::cout << "[printProgress] command_string: " << state->formats.at("Best").command_string.toStdString() << std::endl;
    std::cout << "[printProgress] id: " << id << ", percent: " << percent << ", speed: " << speed << std::endl;
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , controller(new FuzeHttp::Controller<MainWindow*>()) {
    ui->setupUi(this);
#ifdef _WIN32
    const QString os_text = "Windows";
#else
    const QString os_text = "Unix";
#endif
    ui->os_label->setText("Platform: " + os_text);
    ui->compatibility_checkbox->setCheckState(
        this->settings.value("compatibility_mode").toBool() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->embed_metadata_checkbox->setCheckState(
        this->settings.value("embed_metadata").toBool() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->save_location_input->setText(this->settings.value("save_location").toString());
    controller->addPattern(printProgress, "[progress]", std::string(""), (int)0, std::string("")); 
}

MainWindow::~MainWindow() {
    delete controller;
	delete ui;
}

void MainWindow::handleResults(QString result) {
	qDebug() << result << Qt::endl << "Thread finished!";
}

void MainWindow::handleOutputLine_shell_output_download_tab(QString content) {this->handleOutputLine(content, ui->shell_output_download_tab); }
void MainWindow::handleOutputLine_shell_output_update_tab(QString content) {this->handleOutputLine(content, ui->shell_output_update_tab); }

void MainWindow::handleOutputLine(QString line, QTextEdit* shell_output) {
    controller->matchPathAndExecute(this, line.toStdString());
    QScrollBar* scroll_bar;
    scroll_bar = shell_output->verticalScrollBar();
    bool scrollbar_at_bottom = (scroll_bar->value() >= (scroll_bar->maximum() - 20));
    qDebug() << line;
        shell_output->append(line);
    if (scrollbar_at_bottom) {
        shell_output->verticalScrollBar()->setValue(shell_output->verticalScrollBar()->maximum());
    }
}

void MainWindow::runCommandThread(QString command, TAB tab) {
    RunCommand* workerThread = new RunCommand(this, command);
    connect(workerThread, &RunCommand::resultReady, this, &MainWindow::handleResults);
    if (tab == TAB::OUTPUT)
        connect(workerThread, &RunCommand::outputLine, this, &MainWindow::handleOutputLine_shell_output_download_tab);
    else
        connect(workerThread, &RunCommand::outputLine, this, &MainWindow::handleOutputLine_shell_output_update_tab);
    connect(workerThread, &RunCommand::finished, workerThread, &QObject::deleteLater);
	workerThread->start();
}


// ------ DOWNLOAD ------

void MainWindow::on_download_button_clicked() {
    QString command =
        this->invoke_ytdlp +
        " --progress-delta 0.73"
        // " --progress-template [progress] %(info.id)s_%(progress._percent)s_%(progress._speed_str)s_%(progress.eta)s";
        " --progress-template [progress]+%(info.id)s+%(progress._percent)s+%(progress._speed_str)s -o \"%(title).60s [%(id)s].%(ext)s\" ";

    // Determine format
    Format format = this->formats.at(ui->format_combobox->currentText());
    if (format.is_audio) {
        command += " -x";
        if (format.command_string != "bestaudio")
            command += " --audio-format " + format.command_string;
    }
    else {
        if (format.command_string == "best") {
            if (this->getQualityFromSlider() != "best")
                command += " -S \"height:" + this->getQualityFromSlider() + "\"";
        }
        else {
            command += " --merge-output-format " + format.command_string;
            QString quality = this->getQualityFromSlider();
            if (quality != "best")
                command += " --format \"bv*[height<=" + quality + "]+ba/b[height<=" + quality + "]\"";
        }

        if (ui->compatibility_checkbox->isChecked())
            command += " --compat-opt prefer-vp9-sort";
    }

    if (ui->embed_metadata_checkbox->isChecked())
        command += " --embed-metadata";

    // Determine in which directory the video will be saved
    QString save_location;
    if (ui->save_location_input->text().isEmpty())
        command += " -P .";
    else
        command += " -P \"" + ui->save_location_input->text() + '"';

    command += " " + ui->video_url_input->text();

    runCommandThread(command, TAB::OUTPUT);

    ui->progress_bar->setValue(0);
}

QString MainWindow::getQualityFromSlider() {
    const std::array<QString, 8> slider_values_as_string = {"144", "240", "360", "480", "720", "1080", "1440", "best"};
    return slider_values_as_string[ui->quality_slider->value()];
}

void MainWindow::on_save_location_button_clicked() {
    QFileDialog folder_chooser(this);
    folder_chooser.setFileMode(QFileDialog::Directory);
    folder_chooser.setOption(QFileDialog::ShowDirsOnly);
    QString save_location = folder_chooser.getExistingDirectory(this,
                                                                tr("Select a folder, into which videos will be saved"),
                                                                this->settings.value("save_location").toString(),
                                                                QFileDialog::ShowDirsOnly |
                                                                QFileDialog::DontResolveSymlinks);
    if (save_location != "") {
        ui->save_location_input->setText(save_location);
        this->settings.setValue("save_location", save_location);
    }
}


// ------ UPDATE ------

void MainWindow::on_deno_check_button_clicked() {
    QString command = "deno --version";
    runCommandThread(command, TAB::UPDATE);
}

void MainWindow::on_compatibility_checkbox_checkStateChanged(const Qt::CheckState &arg1) {
    this->settings.setValue("compatibility_mode", arg1);
}

void MainWindow::on_embed_metadata_checkbox_checkStateChanged(const Qt::CheckState &arg1) {
    this->settings.setValue("embed_metadata", arg1);
}

void MainWindow::on_format_combobox_currentTextChanged(const QString &arg1) {
    if (this->formats.contains(arg1)) {
        ui->resolution_groupbox->setEnabled(this->formats.at(arg1).is_audio == false);
    }
}

