#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QVector>
#include <QJsonObject>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QStringList>
#include <QHBoxLayout>

class QLineEdit;
class QShortcut;
class QStackedWidget;

struct Track {
    QString name;
    QString filePath;
    qint64  durationMs = 0;
    bool    favorite   = false;

    QString artist;
    QString title;
    QPixmap cover;
    QStringList playlists;

    QJsonObject toJson() const;
    static Track fromJson(const QJsonObject &obj);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void togglePlay();
    void playNext();
    void playPrevious();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void onMetaDataChanged();

    void addTracks();
    void removeTrack(int index);
    void toggleFavorite(int index);
    void toggleFavoriteCurrent();
    void setFilterAll();
    void setFilterFavorites();
    void setFilterPlaylist(const QString &name);
    void onPlaylistItemClicked(QListWidgetItem *item);

    void toggleSettings();
    void onVolumeChanged(int value);

    void onMinimizeWindow();
    void onCloseWindow();

    void onSeekClicked(int pos);
    void cyclePlaybackMode();
    void openTrackEditor();
    void closeTrackEditor();
    void applyTrackEditor();
    void pickTrackCover();

    void onPlaylistActionClicked();
    void toggleTrackInPlaylist(int index);
    void onDeletePlaylistClicked();
    void onConfirmDeletePlaylist();
    void onCreatePlaylistConfirmed();
    void onRenamePlaylistClicked();
    void onRenamePlaylistConfirmed();
    void cancelAddToPlaylist();
    void onSystemVolumeChanged(int percent);

private:
    void setupUi();
    void setupLeftPanel();
    void setupRightPanel();
    void setupSettingsOverlay();
    void setupEditOverlay();

    void loadTrack(int index);
    void playTrack();
    void pauseTrack();
    void stopTrack();
    void refreshPlaylist();
    void updatePlaylistButtons();
    void updatePlaylistActionButton();
    void deleteActivePlaylist();
    void hideDeleteConfirm();
    void showCreatePlaylist();
    void hideCreatePlaylist();
    void showRenamePlaylist();
    void hideRenamePlaylist();
    void updateHeartButton();
    void updatePlayButton();
    void updateTitleText();
    void updateCover();
    QVector<int> playbackOrder() const;
    Track *currentTrack();
    static QString formatTime(qint64 ms);

    enum PlaybackMode { Sequential, Shuffle, RepeatOne };
    PlaybackMode m_playMode = Sequential;
    QPushButton *m_modeButton = nullptr;
    void updateModeButton();

    void savePlaylist() const;
    void loadPlaylist();
    QString storagePath() const;
    QString musicFolder() const;
    void scanMusicFolder();

    QVector<Track>  m_playlist;
    int             m_currentIndex = -1;
    bool            m_settingsOpen = false;

    enum Filter { All, Favorites, Playlist };
    Filter          m_filter = All;
    QString         m_activePlaylist;
    QStringList     m_customPlaylists;
    bool            m_addToPlaylistMode = false;
    QString         m_addTargetPlaylist;
    QVector<QStringList> m_addModeBackup;

    QMediaPlayer   *m_player   = nullptr;
    QAudioOutput   *m_audioOut = nullptr;

    QWidget        *m_outerWidget = nullptr;
    QPushButton    *m_minButton   = nullptr;
    QPushButton    *m_closeButton = nullptr;

    QWidget        *m_leftPanel      = nullptr;
    QWidget        *m_coverWidget    = nullptr;
    QPixmap         m_currentCover;
    QLabel         *m_titleLabel     = nullptr;
    QLabel         *m_artistLabel    = nullptr;
    QSlider        *m_progressSlider = nullptr;
    QLabel         *m_currentTime    = nullptr;
    QLabel         *m_totalTime      = nullptr;
    QPushButton    *m_heartButton    = nullptr;
    QPushButton    *m_prevButton     = nullptr;
    QPushButton    *m_playButton     = nullptr;
    QPushButton    *m_nextButton     = nullptr;
    QPushButton    *m_settingsButton = nullptr;
    QPushButton    *m_editTrackButton = nullptr;

    QWidget        *m_editOverlay = nullptr;
    QLabel         *m_editCoverPreview = nullptr;
    QLineEdit      *m_trackTitleEdit = nullptr;
    QLineEdit      *m_trackArtistEdit = nullptr;
    QByteArray      m_editCoverData;
    int             m_editTrackIndex = -1;

    QWidget        *m_rightPanel     = nullptr;
    QPushButton    *m_allFilterBtn   = nullptr;
    QPushButton    *m_favFilterBtn   = nullptr;

    QWidget        *m_filterButtonsWidget = nullptr;
    QHBoxLayout    *m_filterButtonsLayout = nullptr;

    QLabel         *m_countLabel     = nullptr;
    QListWidget    *m_listWidget     = nullptr;

    QWidget        *m_settingsOverlay = nullptr;
    QPushButton    *m_addTrackButton  = nullptr;
    QPushButton    *m_playlistActionBtn = nullptr;
    QPushButton    *m_cancelEditBtn = nullptr;
    QWidget        *m_playlistRowWidget = nullptr;
    QStackedWidget *m_playlistStack = nullptr;
    QPushButton    *m_deletePlaylistBtn = nullptr;
    QWidget        *m_deleteConfirmWidget = nullptr;
    QLabel         *m_deleteConfirmLabel = nullptr;
    QString         m_pendingDeletePlaylist;
    QWidget        *m_newPlaylistWidget = nullptr;
    QLineEdit      *m_newPlaylistEdit = nullptr;
    bool            m_createPlaylist = false;
    QPushButton    *m_renamePlaylistBtn = nullptr;
    QWidget        *m_renamePlaylistWidget = nullptr;
    QLineEdit      *m_renamePlaylistEdit = nullptr;
    QString         m_renamePlaylistSource;
    bool            m_renamePlaylist = false;
    QShortcut      *m_playPauseShortcut = nullptr;
    QLabel         *m_addToPlaylistLabel = nullptr;

    QSlider        *m_volumeSlider    = nullptr;
    QLabel         *m_volumeLabel     = nullptr;
    QVector<QSlider*> m_eqSliders;

    bool    m_dragging = false;
    QPoint  m_dragOffset;
};

#endif