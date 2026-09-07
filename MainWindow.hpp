#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Controller.hpp"
#include <QMainWindow>
#include <QTextEdit>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Format {
public:
    Format(QString command_string, bool is_audio) : command_string(command_string), is_audio(is_audio) {}
    QString command_string;
    bool is_audio;
};

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
#ifdef _WIN32
    const QString invoke_ytdlp = "yt-dlp.exe";
#else
    const QString invoke_ytdlp = "python3 yt-dlp";
#endif
    enum TAB { OUTPUT, SETTINGS, UPDATE };
    const std::unordered_map<QString, Format> formats = {
        {"Best", Format("best", false)},
        {"MP4", Format("mp4", false)},
        {"WebM", Format("webm", false)},
        {"MKV", Format("mkv", false)},
        {"Best (audio)", Format("bestaudio", true)},
        {"AAC (audio)", Format("aac", true)},
        {"MP3 (audio)", Format("mp3", true)},
        {"OGG (audio)", Format("ogg", true)},
        {"Opus (audio)", Format("opus", true)}
    };
    Ui::MainWindow* ui;

private slots:
	void on_download_button_clicked();

    void on_deno_check_button_clicked();

    void on_save_location_button_clicked();

    void on_compatibility_checkbox_checkStateChanged(const Qt::CheckState &arg1);

    void on_embed_metadata_checkbox_checkStateChanged(const Qt::CheckState &arg1);

    void on_format_combobox_currentTextChanged(const QString &arg1);

private:
    void handleResults(QString result);
    // void printProgress(std::string id, int percent, std::string speed);
    void handleOutputLine(QString line, QTextEdit* shell_output);
    void runCommandThread(QString command, TAB tab);
    void handleOutputLine_shell_output_download_tab(QString content);
    void handleOutputLine_shell_output_update_tab(QString content);
    void onControllerMatch(int number) const;
    QString getQualityFromSlider();
    FuzeHttp::Controller<MainWindow*>* controller;
    QSettings settings;
};
#endif // MAINWINDOW_H
