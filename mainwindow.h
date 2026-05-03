#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QSlider>
#include <QTimer>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QComboBox>
#include <QMap>
#include <QList>
#include <QGraphicsDropShadowEffect>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>

/* ═══════════════════════════════════════════════════
   Binary Tree Node  –  left = dot, right = dash
   ═══════════════════════════════════════════════════ */
struct MorseNode {
    QChar      ch;
    MorseNode *dot;
    MorseNode *dash;
    MorseNode(QChar c = QChar('\0')) : ch(c), dot(nullptr), dash(nullptr) {}
};

/* ═══════════════════════════════════════════════════
   MorseBST – handles all scripts via transliteration
   ═══════════════════════════════════════════════════ */
class MorseBST {
public:
    MorseBST();
    ~MorseBST();

    QString encodeText(const QString &text) const;
    QString decodeText(const QString &morse) const;
    QString encodeChar(QChar c) const;
    QChar   decodeSymbol(const QString &sym) const;

    /* Per-script transliteration maps */
    QMap<QChar, QChar> hindiMap;
    QMap<QChar, QChar> arabicMap;
    QMap<QChar, QChar> greekMap;
    QMap<QChar, QChar> russianMap;
    QMap<QChar, QChar> japaneseMap;
    QMap<QChar, QChar> chineseMap;
    QMap<QChar, QChar> germanMap;
    QMap<QChar, QChar> frenchMap;
    QMap<QChar, QChar> spanishMap;
    QMap<QChar, QChar> portugueseMap;

    QString transliterate(const QString &text, const QMap<QChar,QChar> &map) const;

private:
    MorseNode            *root;
    QMap<QChar, QString>  charToMorse;

    void insert(const QString &code, QChar ch);
    void freeTree(MorseNode *n);
    void initMaps();
};

/* ═══════════════════════════════════════════════════
   Glowing Dot/Dash widget
   ═══════════════════════════════════════════════════ */
class GlowDot : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness)
public:
    explicit GlowDot(bool isDash, QWidget *p = nullptr);
    int  brightness() const   { return m_bright; }
    void setBrightness(int b) { m_bright = b; update(); }
    void flash();
    void dim();
protected:
    void paintEvent(QPaintEvent *) override;
private:
    bool m_isDash;
    int  m_bright;
    QPropertyAnimation *m_anim;
};

/* ═══════════════════════════════════════════════════
   MorseStrip – scrollable animated strip
   ═══════════════════════════════════════════════════ */
class MorseStrip : public QWidget {
    Q_OBJECT
public:
    explicit MorseStrip(QWidget *p = nullptr);
    void loadMorse(const QString &morse);
    void flashAt(int idx);
    void resetAll();
    int  count() const { return m_dots.size(); }
private:
    QHBoxLayout     *m_layout;
    QList<GlowDot*>  m_dots;
};

/* ═══════════════════════════════════════════════════
   Language entry
   ═══════════════════════════════════════════════════ */
struct LangInfo {
    QString name;
    QString flag;
    QString script;
    QString tip;
};

/* ═══════════════════════════════════════════════════
   MainWindow
   ═══════════════════════════════════════════════════ */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *p = nullptr);
    ~MainWindow();

private slots:
    void onTranslate();
    void onSwap();
    void onPlay();
    void onStop();
    void onTick();
    void onSpeedChanged(int v);
    void onClear();
    void onCopy();
    void onLangChanged(int idx);
    void onInputChanged();

private:
    MorseBST      *m_tree;
    bool           m_toMorse;

    /* widgets */
    QTextEdit     *m_input;
    QTextEdit     *m_output;
    QPushButton   *m_btnTranslate;
    QPushButton   *m_btnSwap;
    QPushButton   *m_btnPlay;
    QPushButton   *m_btnStop;
    QPushButton   *m_btnClear;
    QPushButton   *m_btnCopy;
    QComboBox     *m_langBox;
    QLabel        *m_langTip;
    QSlider       *m_slider;
    QLabel        *m_lblSpeed;
    QLabel        *m_lblSpeedVal;
    MorseStrip    *m_strip;
    QScrollArea   *m_scroll;
    QLabel        *m_status;
    QLabel        *m_charCount;
    QLabel        *m_modeLabel;
    QFrame        *m_glowLine;

    /* playback */
    QTimer        *m_timer;
    int            m_playIdx;
    QString        m_currentMorse;

    /* language list */
    QList<LangInfo> m_langs;

    void buildUI();
    void applyTheme();
    void initLangs();
    void setStatus(const QString &msg, const QString &color = "#00ff9f");
    void updateMode();
    int     speedMs() const;
    QString speedName() const;

    /* helper: make a styled section frame */
    QFrame* makeSection(const QString &title, QLayout *content);
};

#endif
